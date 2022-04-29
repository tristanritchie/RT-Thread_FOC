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



void mc_pwm_enable(struct rt_device_pwm *pwm_dev);
void mc_pwm_disable(struct rt_device_pwm *pwm_dev);
void mc_pwm_set(struct rt_device_pwm *pwm_dev, mc_svpwm_t *svm);

void mc_adc_enable(rt_adc_device_t adc1_dev, rt_adc_device_t adc2_dev);
void mc_adc_disable(rt_adc_device_t adc1_dev, rt_adc_device_t adc2_dev);

void mc_current_control(void);
void mc_rotor_alignment(mc_input_signals_t *input, mc_tansform_t *transform, mc_svpwm_t *svm);


static mc_foc_context_t *p_context;

static rt_adc_device_t adc1_dev, adc2_dev;

static struct rt_device_pwm *pwm_dev; /* PWM device handle */

static rt_device_t pulse_encoder_dev;


static mc_svpwm_t svm;
static mc_input_signals_t input;
static mc_tansform_t transform;

static mc_pi_controller_t d_axis_controller;
static mc_pi_controller_t q_axis_controller;

static mc_pi_controller_t speed_controller;


void mc_toggle_pin(rt_base_t pin)
{
    rt_pin_write(pin, !rt_pin_read(pin));
}

float adc_count_to_volt(uint32_t res)
{
    return (3.3/4095)*res;
}

rt_err_t mc_adc_callback(rt_device_t dev,rt_size_t size)
{
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
    if (p_context->state == MC_FOC_ENABLE)
    {
        mc_current_control();
    }
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
    return RT_EOK;
}



void mc_foc_init(void)
{
    LOG_D("foc init start");

    p_context = rt_malloc(sizeof(mc_foc_context_t));

    rt_pin_mode(LED1, PIN_MODE_OUTPUT);
    rt_pin_mode(LED2, PIN_MODE_OUTPUT);
    rt_pin_mode(LED3, PIN_MODE_OUTPUT);
    rt_pin_mode(PIN_E0, PIN_MODE_OUTPUT);


    pulse_encoder_dev = rt_device_find(QE_DEV_NAME);
    if (pulse_encoder_dev == RT_NULL)
    {
        LOG_D("pulse2 not found");
    }

    rt_device_open(pulse_encoder_dev, RT_DEVICE_OFLAG_RDWR);

    /* RT-Thread driver device object initialization */
    /* Register phase A current measurement ADC */
    adc1_dev = (rt_adc_device_t)rt_device_find(ADC1_DEV_NAME);
    if (adc1_dev == RT_NULL)
    {
        LOG_D("adc1 not found");
    }
    /* Register phase B current measurement ADC */
    adc2_dev = (rt_adc_device_t)rt_device_find(ADC2_DEV_NAME);
    if (adc2_dev == RT_NULL)
    {
        LOG_D("adc2 not found");
    }
    /* Register PWM timer device */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        LOG_D("pwm1 not found");
    }


    /* ADC offset calibration */
    //mc_adc_offset_calibration(adc1_dev, 0, &input.a_offset);
    //mc_adc_offset_calibration(adc2_dev, 1, &input.b_offset);


    /* FOC structure initialization */
    /* Initialize PI controller block structure */

    /* Initialize SVM block structure */
    mc_svm_init(&svm);

    mc_adc_enable(adc1_dev, adc2_dev);


   /* Enable device */
   mc_pwm_enable(pwm_dev);

   mc_rotor_alignment(&input, &transform, &svm);

   rt_device_control(pulse_encoder_dev, PULSE_ENCODER_CMD_CLEAR_COUNT, RT_NULL);

    rt_device_set_rx_indicate(&adc1_dev->parent, mc_adc_callback);

    p_context->state = MC_FOC_ENABLE;
}


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
    rt_pwm_enable(pwm_dev, PWM_CH2);
    rt_pwm_enable(pwm_dev, PWM_CH3);
}

void mc_pwm_disable(struct rt_device_pwm *pwm_dev)
{
    rt_pwm_disable(pwm_dev, PWM_CH1);
    rt_pwm_disable(pwm_dev, PWM_CH2);
    rt_pwm_disable(pwm_dev, PWM_CH3);
}

void mc_pwm_set(struct rt_device_pwm *pwm_dev, mc_svpwm_t *svm)
{
    rt_pwm_set(pwm_dev, PWM_CH1, PWM_PERIOD, svm->pwm1);
    rt_pwm_set(pwm_dev, PWM_CH2, PWM_PERIOD, svm->pwm2);
    rt_pwm_set(pwm_dev, PWM_CH3, PWM_PERIOD, svm->pwm3);
}


void mc_current_control(void)
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
    p_context->control_hook++;

    if (p_context->control_hook == MC_SPEED_CNTR_PRESCALER)
    {
        p_context->control_hook = 0;

        speed_controller.in_meas = input.speed;
        mc_pi_control(&speed_controller);
        d_axis_controller.in_ref = speed_controller.out;
    }

#endif
}


void mc_rotor_alignment(mc_input_signals_t *input, mc_tansform_t *transform, mc_svpwm_t *svm)
{
    transform->park.d_axis = 0;
    transform->park.q_axis = ALIGN_CURRENT;

    input->e_angle = M_PI;
    mc_calc_sin_cos(input->e_angle, &transform->sin_angle, &transform->cos_angle);

    /* Inverse Clarke and Park transform */
    mc_inverse_park_transform(transform);
    /* SVPWM */
    mc_svpwm_gen(&transform->clarke, svm);
    /*Update PWM duty cycles*/
    mc_pwm_set(pwm_dev, svm);

    rt_thread_mdelay(ALIGN_DELAY_MS);

    input->e_angle = 3 * M_PI / 2;
    mc_calc_sin_cos(input->e_angle, &transform->sin_angle, &transform->cos_angle);

    /* Inverse Clarke and Park transform */
    mc_inverse_park_transform(transform);
    /* SVPWM */
    mc_svpwm_gen(&transform->clarke, svm);
    /*Update PWM duty cycles*/
    mc_pwm_set(pwm_dev, svm);

    rt_thread_mdelay(ALIGN_DELAY_MS);
}


static int foc(int argc, char **argv)
{
    rt_uint32_t demand;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "enable"))
        {
            p_context->state = MC_FOC_ENABLE;
        }
        else if (!strcmp(argv[1], "disable"))
        {
            p_context->state = MC_FOC_DISABLE;
        }
        else if (!strcmp(argv[1], "reset"))
        {
            p_context->state = MC_FOC_DISABLE;

            // reset here


        }
        else if (!strcmp(argv[1], "print"))
        {
            if (argc == 3)
            {
                if (!strcmp(argv[2], "sp"))
                {
                    // print speed
                }
                if (!strcmp(argv[2], "cur"))
                {
                    // print current
                }
                if (!strcmp(argv[2], "dq"))
                {
                    // print dq currents
                }
                if (!strcmp(argv[2], "ab"))
                {
                    // print alpha/beta currents
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
        else if (!strcmp(argv[1], "speed"))
        {
            if (argc == 3)
            {
                demand = atoi(argv[2]);
                // set demand speed
            }
            else
            {
                rt_kprintf("foc speed <demand>  - set demand speed\n");
            }
        }
        else if (!strcmp(argv[1], "torque"))
        {
            if (argc == 3)
            {
                demand = atoi(argv[2]);
                // set demand torque
            }
            else
            {
                rt_kprintf("foc torque <demand>  - set demand speed\n");
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
        rt_kprintf("foc speed <demand>  - set demand speed\n");
        rt_kprintf("foc torque <demand> - set demand torque\n");
        rt_kprintf("foc enable          - enable output\n");
        rt_kprintf("foc disable         - disable output\n");
        rt_kprintf("foc reset           - reinitialize foc\n");
        rt_kprintf("foc print <param>   - print parameter\n");
        rt_kprintf("                        'sp' for speed\n");
        rt_kprintf("                        'cur' for phase currents\n");
        rt_kprintf("                        'dq' for DQ axis vector\n");
        rt_kprintf("                        'ab' for alpha/beta axis and rotor angle\n");
    }
    return RT_EOK;
}
MSH_CMD_EXPORT(foc, foc function)
