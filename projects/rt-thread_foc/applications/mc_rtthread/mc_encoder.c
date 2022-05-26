/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */

#include "mc_encoder.h"

void mc_read_qe(rt_device_t dev, mc_input_signals_t *input)
{
    struct rt_pulse_encoder_device *pulse_encoder = (struct rt_pulse_encoder_device *)dev;
    input->qe_count = pulse_encoder->ops->get_count(pulse_encoder);

    input->e_angle = (input->qe_count % QE_COUNT_PER_EREV) * QE_COUNT_TO_RAD;
    input->qe_angle_sum += input->e_angle;

    return;
}


void mc_calc_speed(mc_input_signals_t *input)
{
    input->speed = input->qe_angle_sum * MC_SPEED_CNTR_FREQ;
    input->qe_angle_sum = 0;

    return;
}
