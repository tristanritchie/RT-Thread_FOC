/*/96.-+
 *
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include <string.h>
#include <stdlib.h>


#include "drivers/rt_drv_pwm.h"
#include "drivers/adc.h"
#include "stm32f4xx_hal.h"

#include "mc_foc.h"

#define DBG_TAG "mc_foc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

void mc_foc(void);
void mc_rotor_alignment(mc_input_signals_t *input, mc_tansform_t *transform, mc_svpwm_t *svm);
rt_err_t mc_adc_callback(rt_device_t dev,rt_size_t size);
void mc_foc_tasks(void *parameter);

void mc_communicate(void);

void mc_pwm_enable(struct rt_device_pwm *pwm_dev);
void mc_pwm_disable(struct rt_device_pwm *pwm_dev);
void mc_pwm_set(struct rt_device_pwm *pwm_dev, mc_svpwm_t *svm);

void mc_adc_enable(rt_adc_device_t adc1_dev, rt_adc_device_t adc2_dev);
void mc_adc_disable(rt_adc_device_t adc1_dev, rt_adc_device_t adc2_dev);

static mc_foc_context_t *p_context;

/* RT-Thread peripheral device handles */
static struct rt_device_pwm *pwm_dev;
static rt_adc_device_t adc1_dev, adc2_dev;
static rt_device_t pulse_encoder_dev;

/* FOC parameter structures */
static mc_input_signals_t input;
static mc_tansform_t transform;

static mc_svpwm_t svm = SVPWM_INIT;
static mc_pi_controller_t d_axis_controller = D_AXIS_CONTROLLER_INIT;
static mc_pi_controller_t q_axis_controller = Q_AXIS_CONTROLLER_INIT;
#ifdef SPEED_CONTROL_ENABLE
static mc_pi_controller_t speed_controller = SPEED_CONTROLLER_INIT;
#endif

/* FOC thread */
rt_thread_t foc_thread;
static char foc_thread_stack[1024];


int mc_foc_init(void)
{
    int result = RT_EOK;

    /* Create mem pointer */
    p_context = rt_malloc(sizeof(mc_foc_context_t));
    p_context->com_mode = 0;

    /* Debug GPIOs */
    rt_pin_mode(LED1, PIN_MODE_OUTPUT);
    rt_pin_mode(LED2, PIN_MODE_OUTPUT);
    rt_pin_mode(LED3, PIN_MODE_OUTPUT);
    rt_pin_mode(PIN_E0, PIN_MODE_OUTPUT);

    foc_thread = rt_thread_create("foc_thread", mc_foc_tasks, RT_NULL, sizeof(foc_thread_stack), 1, 5);
    if (foc_thread != RT_NULL)
    {
        rt_thread_startup(foc_thread);
    }

    /* RT-Thread driver device object initialization */
    /* Register phase A current measurement ADC */
    adc1_dev = (rt_adc_device_t)rt_device_find(ADC1_DEV_NAME);
    if (adc1_dev == RT_NULL)
    {
        LOG_D("adc1 not found");
        result = -RT_ERROR;
        goto __exit;
    }
    /* Register phase B current measurement ADC */
    adc2_dev = (rt_adc_device_t)rt_device_find(ADC2_DEV_NAME);
    if (adc2_dev == RT_NULL)
    {
        LOG_D("adc2 not found");
        result = -RT_ERROR;
        goto __exit;
    }
    /* Register PWM timer device */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        LOG_D("pwm1 not found");
        result = -RT_ERROR;
        goto __exit;
    }
    /* Register quadrature encoder interface */
    pulse_encoder_dev = rt_device_find(QE_DEV_NAME);
    if (pulse_encoder_dev == RT_NULL)
    {
        LOG_D("pulse2 not found");
        result = -RT_ERROR;
        goto __exit;
    }

    /* Enable encoder operation */
    rt_device_open(pulse_encoder_dev, RT_DEVICE_OFLAG_RDWR);

    /* ADC offset calibration */
    //mc_adc_offset_calibration(adc1_dev, 0, &input.a_offset);
    //mc_adc_offset_calibration(adc2_dev, 1, &input.b_offset);

    /* FOC structure initialization */
    /* Initialize PI controller block structure */

    /* Initialize default PWM period/pulse*/
    /* Enable peripherals*/
    mc_adc_enable(adc1_dev, adc2_dev);
    mc_pwm_enable(pwm_dev);

    /* Forced alignment of rotor */
    mc_rotor_alignment(&input, &transform, &svm);
    /* Reset encoder to initial position 0 */
    rt_device_control(pulse_encoder_dev, PULSE_ENCODER_CMD_CLEAR_COUNT, RT_NULL);

    /* Register ADC callback */
    rt_device_set_rx_indicate(&adc1_dev->parent, mc_adc_callback);
    /* Enable FOC */
    p_context->enable_state = MC_FOC_ENABLE;

 __exit:
     return result;
}


rt_err_t mc_adc_callback(rt_device_t dev,rt_size_t size)
{
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
    if (p_context->enable_state == MC_FOC_ENABLE)
    {
        mc_foc();
    }
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
    return RT_EOK;
}


void mc_foc(void)
{
    /* Read ADC current measurement */
    mc_read_currents(adc1_dev, adc2_dev, &input);

    /* Read encoder */
    mc_read_qe(pulse_encoder_dev, &input);

    /* Clarke and Park transform */
    mc_clarke_park_transform(&input, &transform);

    /* D/Q current PI control */
    q_axis_controller.in_meas = transform.park.q_axis;
    mc_pi_control(&q_axis_controller);
    transform.park.q_axis = q_axis_controller.out;

    d_axis_controller.in_meas = transform.park.d_axis;
    mc_pi_control(&d_axis_controller);
    transform.park.d_axis = d_axis_controller.out;

    /* Inverse Clarke and Park transform */
    mc_inverse_park_transform(&transform);

    /* SVPWM */
    mc_svpwm_gen(&transform.clarke, &svm);

    /*Update PWM duty cycles*/
    //mc_pwm_set(pwm_dev, &svm);

#ifdef SPEED_CONTROL_ENABLE
    p_context->control_sync++;

    if (p_context->control_sync == MC_SPEED_CNTR_PRESCALER)
    {
        p_context->control_sync = 0;

        speed_controller.in_meas = input.speed;
        mc_pi_control(&speed_controller);
        d_axis_controller.in_ref = speed_controller.out;
    }
#endif /* SPEED_CONTROL_ENABLE */

    return;
}


void mc_rotor_alignment(mc_input_signals_t *input, mc_tansform_t *transform, mc_svpwm_t *svm)
{
    transform->park.d_axis = 0;
    transform->park.q_axis = ALIGN_CURRENT;
    /* Set inital rotor angle - first pass */
    input->e_angle = M_PI;
    mc_calc_sin_cos(input->e_angle, &transform->sin_angle, &transform->cos_angle);

    /* Inverse Clarke and Park transform */
    mc_inverse_park_transform(transform);
    /* SVPWM */
    mc_svpwm_gen(&transform->clarke, svm);
    /* Update PWM duty cycles*/
    mc_pwm_set(pwm_dev, svm);
    /* Delay for mechanical alignment */
    rt_thread_mdelay(ALIGN_DELAY_MS);

    /* Set starting rotor angle - second pass */
    input->e_angle = 3 * M_PI / 2;
    mc_calc_sin_cos(input->e_angle, &transform->sin_angle, &transform->cos_angle);

    /* Inverse Clarke and Park transform */
    mc_inverse_park_transform(transform);
    /* SVPWM */
    mc_svpwm_gen(&transform->clarke, svm);
    /* Update PWM duty cycles*/
    mc_pwm_set(pwm_dev, svm);
    /* Delay for mechanical alignment */
    rt_thread_mdelay(ALIGN_DELAY_MS);

    /* Cut power to motor */
    *svm = SVPWM_INIT;
    mc_pwm_set(pwm_dev, svm);

    return;
}


void mc_foc_tasks(void *parameter)
{
    while (1)
    {
        rt_thread_mdelay(COMM_TX_PERIOD_MS);
        rt_pin_write(LED1, !rt_pin_read(LED1));
        mc_communicate();
    }
}

void mc_foc_enable(void)
{
    if (p_context->enable_state != MC_FOC_ENABLE)
    {
        p_context->enable_state = MC_FOC_ENABLE;
        mc_adc_enable(adc1_dev, adc2_dev);
        mc_pwm_enable(pwm_dev);
    }
}

void mc_foc_disable(void)
{
    if (p_context->enable_state != MC_FOC_DISABLE)
    {
        p_context->enable_state = MC_FOC_DISABLE;
        mc_adc_disable(adc1_dev, adc2_dev);
        mc_pwm_disable(pwm_dev);
    }
}

void mc_set_demand(float setpoint)
{
#ifdef SPEED_CONTROL_ENABLE
    mc_impose_limits(&setpoint, -1000, 1000);
    speed_controller.in_ref = setpoint;
    rt_kprintf("Speed demand set: %.3f", setpoint);
#else
#ifdef TORQUE_CONTROL_ENABLE
    mc_impose_limits(&setpoint, -1, 1);
    d_axis_controller.in_ref = setpoint;
    rt_kprintf("Torque demand set: %.3f", setpoint);
#endif /* TORQUE_CONTROL_ENABLE */
#endif /* SPEED_CONTROL_ENABLE */
}

void mc_communicate(void)
{
    if (p_context->com_mode & COM_SPEED_MASK)
    {
        rt_kprintf("%d \n", (rt_int32_t)input.speed);
    }
    if (p_context->com_mode & COM_CURRENT_MASK)
    {
        rt_kprintf("%d \t %d \t %d \n", (rt_int32_t)input.ia, (rt_int32_t)input.ib, (rt_int32_t)input.ic);
    }
    if (p_context->com_mode & COM_DQ_MASK)
    {
        rt_kprintf("%d \t %d \n", (rt_int32_t)transform.park.d_axis, (rt_int32_t)transform.park.q_axis);
    }
    if (p_context->com_mode & COM_ALPHA_BETA_MASK)
    {
        rt_kprintf("%d \t %d \n", (rt_int32_t)transform.clarke.alpha, (rt_int32_t)transform.clarke.beta);
    }
}

/* Virtual COM interface (UART) */
static int foc(int argc, char **argv)
{
    if (argc > 1)
    {
        if (!strcmp(argv[1], "enable"))
        {
            mc_foc_enable();
        }
        else if (!strcmp(argv[1], "disable"))
        {
            mc_foc_disable();
        }
        else if (!strcmp(argv[1], "print"))
        {
            if (argc == 3)
            {
                if (!strcmp(argv[2], "sp"))
                {
                    // print speed
                    p_context->com_mode |= COM_SPEED_MASK;
                }
                if (!strcmp(argv[2], "cur"))
                {
                    // print current
                    p_context->com_mode |= COM_CURRENT_MASK;
                }
                if (!strcmp(argv[2], "dq"))
                {
                    // print dq currents
                    p_context->com_mode |= COM_DQ_MASK;
                }
                if (!strcmp(argv[2], "ab"))
                {
                    // print alpha/beta currents
                    p_context->com_mode |= COM_ALPHA_BETA_MASK;
                }
                if (!strcmp(argv[2], "stop"))
                {
                    // stop print
                    p_context->com_mode = 0;
                }
            }
            else
            {
                rt_kprintf("foc print <param>   - print parameter\n");
                rt_kprintf("                        'sp' for speed\n");
                rt_kprintf("                        'cur' for phase currents\n");
                rt_kprintf("                        'dq' for DQ axis vector\n");
                rt_kprintf("                        'ab' for alpha/beta axis and rotor angle\n");
            }
        }
        else if (!strcmp(argv[1], "s"))
        {
            if (argc == 3)
            {
                // set demand
                float demand = atof(argv[2]);
                mc_set_demand(demand);
            }
            else
            {
                rt_kprintf("foc s <demand>  - set demand speed or torque\n");
            }
        }
        else
        {
            rt_kprintf("Unknown command. Please enter 'foc' for help\n");
        }
    }

    else
    {
        rt_kprintf("Usage: \n");
        rt_kprintf("foc s <demand>      - set demand speed or torque\n\n");
        rt_kprintf("foc enable          - enable output\n");
        rt_kprintf("foc disable         - disable output\n");
        rt_kprintf("foc print <param>   - print parameter\n");
        rt_kprintf("                        'sp' for speed\n");
        rt_kprintf("                        'cur' for phase currents\n");
        rt_kprintf("                        'dq' for DQ axis vector\n");
        rt_kprintf("                        'ab' for alpha/beta axis and rotor angle\n");
    }
    return RT_EOK;
}
MSH_CMD_EXPORT(foc, foc function)


void mc_adc_enable(rt_adc_device_t adc1_dev, rt_adc_device_t adc2_dev)
{
    HAL_ADCEx_InjectedStart(adc2_dev->parent.user_data);
    HAL_ADCEx_InjectedStart_IT(adc1_dev->parent.user_data);
}


void mc_adc_disable(rt_adc_device_t adc1_dev, rt_adc_device_t adc2_dev)
{
    HAL_ADCEx_InjectedStop_IT(adc1_dev->parent.user_data);
    HAL_ADCEx_InjectedStop(adc2_dev->parent.user_data);
}


void mc_pwm_enable(struct rt_device_pwm *pwm_dev)
{
    rt_pwm_enable(pwm_dev, PWM_CH1);
    rt_pwm_enable(pwm_dev, -PWM_CH1);
    rt_pwm_enable(pwm_dev, PWM_CH2);
    rt_pwm_enable(pwm_dev, -PWM_CH2);
    rt_pwm_enable(pwm_dev, PWM_CH3);
    rt_pwm_enable(pwm_dev, -PWM_CH3);
}


void mc_pwm_disable(struct rt_device_pwm *pwm_dev)
{
    rt_pwm_disable(pwm_dev, PWM_CH1);
    rt_pwm_disable(pwm_dev, -PWM_CH1);
    rt_pwm_disable(pwm_dev, PWM_CH2);
    rt_pwm_disable(pwm_dev, -PWM_CH2);
    rt_pwm_disable(pwm_dev, PWM_CH3);
    rt_pwm_disable(pwm_dev, -PWM_CH3);
}


void mc_pwm_set(struct rt_device_pwm *pwm_dev, mc_svpwm_t *svm)
{
    rt_pwm_set(pwm_dev, PWM_CH1, PWM_PERIOD, svm->pwm1);
    rt_pwm_set(pwm_dev, PWM_CH2, PWM_PERIOD, svm->pwm2);
    rt_pwm_set(pwm_dev, PWM_CH3, PWM_PERIOD, svm->pwm3);
}

