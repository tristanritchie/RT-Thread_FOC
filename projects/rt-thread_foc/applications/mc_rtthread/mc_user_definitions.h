/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-04-02     Tristan       the first version
 */
#ifndef APPLICATIONS_MC_RTTHREAD_MC_USER_DEFINITIONS_H_
#define APPLICATIONS_MC_RTTHREAD_MC_USER_DEFINITIONS_H_

#include <rtthread.h>
#include <rtdevice.h>

#include <math.h>

/* Board GPIO pin numbers */
#define LED1                16
#define LED2                23
#define LED3                30
#define BTN                 45
#define PIN_E0              64


/* Control system configuration */
#define MC_PWM_FREQ             20000
#define MC_TORQUE_CNTR_FREQ     20000
#define MC_SPEED_CNTR_FREQ      1000
#define MC_SPEED_CNTR_PRESCALER MC_TORQUE_CNTR_FREQ / MC_SPEED_CNTR_FREQ

#define TORQUE_CONTROL_ENABLE
//#define SPEED_CONTROL_ENABLE


/* PWM configuration */
#define PWM_PERIOD              50755               /* 25kHz PWM */
#define PWM_NEUTRAL             (PWM_PERIOD/2)      /* 50% duty cycle */

#define PWM_DEV_NAME            "pwm1"      /* RT-Thread PWM device registration name */
#define PWM_CH1                 1           /* Phase A channel */
#define PWM_CH2                 2           /* Phase B channel */
#define PWM_CH3                 3           /* Phase C channel */


/* ADC configuration */
#define ADC1_DEV_NAME           "adc1"      /* RT-Thread phase A ADC device registration name */
#define ADC2_DEV_NAME           "adc2"      /* RT-Thread phase B ADC device registration name */

#define ADC1_CH_RANK            1           /* Phase A ADC injected (priority) conversion channel rank */
#define ADC2_CH_RANK            1           /* Phase A ADC injected (priority) conversion channel rank */

#define ADC_MAX_COUNT           4095        /*  ADC Resolution */
#define ADC_REF_VOLTAGE         3.3         /*  ADC reference voltage */

#define ADC_MAX_CURRENT         1           /* Absolute current measurement range of ADC */
#define ADC_CURRENT_SCALE       ADC_MAX_CURRENT/(ADC_MAX_COUNT)

/* Motor configuration */
#define MOTOR_NUM_POLE_PAIRS    1           /* Number of pole pairs in the motor */
#define MOTOR_QE_PPR            256         /* Number of quadrature encoder pulses per mechanical revolution */

/* Quadrature encoder configuration */
#define QE_DEV_NAME             "pulse2"
#define QE_COUNT_PER_EREV       ((MOTOR_QE_PPR * 4) / MOTOR_NUM_POLE_PAIRS)     /* Encoder edges per electrical revolution */
#define QE_COUNT_TO_RAD         ((2 * M_PI) / QE_COUNT_PER_EREV)                /* Encoder edge count to electrical angle in radians */
#define QE_COUNT_TO_RADS_PER_S  (MC_SPEED_CNTR_FREQ * QE_COUNT_TO_RAD)          /* Encoder edge count to radians per second */


/* Rotor alignment configuration */
#define ALIGN_CURRENT           0.4         /* Rotor alignment current vector magnitude */
#define ALIGN_DELAY_MS          200         /* Delay (ms) for the rotor to align with stationary current vector */

/* Communication parameters */
#define COM_SPEED_MASK          (1 << 0)
#define COM_CURRENT_MASK        (1 << 1)
#define COM_DQ_MASK             (1 << 2)
#define COM_ALPHA_BETA_MASK     (1 << 3)

#define COM_TX_FREQUENCY        1
#define COMM_TX_PRESCALER       MC_TORQUE_CNTR_FREQ / COM_TX_FREQUENCY

typedef struct mc_input_signals_t
{
    float ia;
    float ib;
    float ic;
    float e_angle;
    float speed;
    rt_uint32_t qe_count_sum;
    rt_uint32_t a_offset;
    rt_uint32_t b_offset;
    rt_uint32_t c_offset;
} mc_input_signals_t;


#endif /* APPLICATIONS_MC_RTTHREAD_MC_USER_DEFINITIONS_H_ */
