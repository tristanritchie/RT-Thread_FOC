/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */


#ifndef APPLICATIONS_MC_RTTHREAD_MC_SVM_H_
#define APPLICATIONS_MC_RTTHREAD_MC_SVM_H_

#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "mc_transform.h"
#include "mc_user_definitions.h"


typedef struct mc_svpwm_t
{
    float period;   /* PWM period */
    float vr1;      /* Inverse Clarke transform phase A */
    float vr2;      /* Inverse Clarke transform phase B */
    float vr3;      /* Inverse Clarke transform phase C */
    float t1;       /* Time applying first space vector */
    float t2;       /* Time applying second space vector */
    float ta;       /* Channel 1 precise ON time (duty cycle) */
    float tb;       /* Channel 2 precise ON time (duty cycle) */
    float tc;       /* Channel 3 precise ON time (duty cycle) */
    uint32_t pwm1;  /* Channel 1 duty cycle */
    uint32_t pwm2;  /* Channel 2 duty cycle */
    uint32_t pwm3;  /* Channel 3 duty cycle */
} mc_svpwm_t;

#define SVPWM_INIT (mc_svpwm_t) \
{                               \
    .pwm1 = PWM_NEUTRAL,        \
    .pwm2 = PWM_NEUTRAL,        \
    .pwm3 = PWM_NEUTRAL,        \
    .period = PWM_PERIOD,       \
}

void mc_svpwm_gen(const mc_clarke_tansform_t *clarke, mc_svpwm_t *svm);

#endif /* APPLICATIONS_MC_RTTHREAD_MC_SVM_H_ */
