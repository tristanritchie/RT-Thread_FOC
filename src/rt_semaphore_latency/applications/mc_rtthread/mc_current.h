/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */

#ifndef APPLICATIONS_MC_RTTHREAD_MC_CURRENT_H_
#define APPLICATIONS_MC_RTTHREAD_MC_CURRENT_H_

#include <rtthread.h>
#include <rtdevice.h>

#include "drivers/adc.h"


typedef struct mc_input_signals_t
{
    float ia;
    float ib;
    float ic;
    float e_angle;
} mc_input_signals_t;

#endif /* APPLICATIONS_MC_RTTHREAD_MC_CURRENT_H_ */
