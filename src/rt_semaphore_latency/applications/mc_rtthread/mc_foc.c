/*
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
#include "drivers/rt_drv_pwm.h"
#include "drivers/adc.h"
#include "stm32f4xx_hal.h"

#include "mc_foc.h"

#define DBG_TAG "mc_foc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>


#define CH1 1
#define CH2 2
#define CH3 3
#define PIN_E0              64
#define LED3                30

void mc_foc_tasks(void*);

void mc_pwm_enable(struct rt_device_pwm *pwm_dev);
void mc_pwm_disable(struct rt_device_pwm *pwm_dev);
void mc_pwm_set(struct rt_device_pwm *pwm_dev, mc_svpwm_t *svm);

void mc_adc_enable();
void mc_adc_disable(rt_adc_device_t adc_dev);

void mc_toggle_pin(rt_base_t pin);

rt_adc_device_t adc1_dev, adc2_dev, adc3_dev;

struct rt_device_pwm *pwm_dev; /* PWM device handle */
mc_svpwm_t svm;
mc_input_signals_t input;
mc_tansform_t transform;

mc_pi_controller_t d_axis_controller;
mc_pi_controller_t q_axis_controller;

rt_sem_t adc_sem;

rt_thread_t foc_thread;
static char foc_thread_stack[1024];


float adc_count_to_volt(uint32_t res)
{
    return (3.3/4095)*res;
}



rt_err_t mc_adc_callback(rt_device_t dev,rt_size_t size)
{
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
    rt_sem_release(adc_sem);
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
    return RT_EOK;
}


void mc_foc_tasks(void *parameter)
{
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
    static rt_err_t result;
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
        result = rt_sem_take(adc_sem, RT_WAITING_FOREVER);
        if(result == RT_EOK)
        {
            HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
        }
    }
}


void mc_foc_init(void)
{
    LOG_D("foc init start");

    rt_pin_mode(LED1, PIN_MODE_OUTPUT);
    rt_pin_mode(LED2, PIN_MODE_OUTPUT);
    rt_pin_mode(LED3, PIN_MODE_OUTPUT);

    rt_pin_mode(PIN_E0, PIN_MODE_OUTPUT);


    adc_sem = rt_sem_create("adcsem", 0, RT_IPC_FLAG_FIFO);

    if (adc_sem == RT_NULL)
    {
        LOG_D("sem create failed");
    }

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

    /* FOC structure initialization */
    /* Initialize SVM structure */
    mc_svm_init(&svm);

    /* Initialize SVM structure */
    mc_adc_enable();


   /* Set the default value of PWM period and pulse width */
   mc_pwm_set(pwm_dev, &svm);
   /* Enable device */
   mc_pwm_enable(pwm_dev);
   rt_device_set_rx_indicate(&adc1_dev->parent, mc_adc_callback);


}

void mc_adc_enable()
{
    HAL_ADCEx_InjectedStart(adc3_dev->parent.user_data);
    HAL_ADCEx_InjectedStart(adc2_dev->parent.user_data);
    HAL_ADCEx_InjectedStart_IT(adc1_dev->parent.user_data);
}

void mc_adc_disable(rt_adc_device_t adc_dev)
{
    HAL_ADCEx_InjectedStop_IT(adc_dev->parent.user_data);

    rt_adc_disable(adc_dev, 0);
    rt_adc_disable(adc_dev, 1);
    rt_adc_disable(adc_dev, 2);
}

void mc_pwm_enable(struct rt_device_pwm *pwm_dev)
{
    rt_pwm_enable(pwm_dev, CH1);
    rt_pwm_enable(pwm_dev, CH2);
    rt_pwm_enable(pwm_dev, CH3);
}

void mc_pwm_disable(struct rt_device_pwm *pwm_dev)
{
    rt_pwm_disable(pwm_dev, CH1);
    rt_pwm_disable(pwm_dev, CH2);
    rt_pwm_disable(pwm_dev, CH3);
}

void mc_pwm_set(struct rt_device_pwm *pwm_dev, mc_svpwm_t *svm)
{
    rt_pwm_set(pwm_dev, CH1, PWM_PERIOD, svm->pwm1);
    rt_pwm_set(pwm_dev, CH2, PWM_PERIOD, svm->pwm2);
    rt_pwm_set(pwm_dev, CH3, PWM_PERIOD, svm->pwm3);
}



