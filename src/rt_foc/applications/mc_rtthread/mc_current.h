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

#include "mc_user_definitions.h"
#include "drivers/adc.h"

#include "board.h"
#include "stm32f4xx_hal_adc_ex.h"




void mc_adc_offset_calibration(rt_adc_device_t adc_dev, rt_uint32_t channel, rt_uint32_t *offset);

void mc_read_currents(rt_adc_device_t adc1_dev, rt_adc_device_t adc2_dev, mc_input_signals_t *input);


#endif /* APPLICATIONS_MC_RTTHREAD_MC_CURRENT_H_ */
