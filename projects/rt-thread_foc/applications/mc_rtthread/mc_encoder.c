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

void mc_read_qe(rt_device_t pulse_encoder_dev, mc_input_signals_t *input)
{
    rt_uint32_t encoder_count;
    pulse_encoder_dev->read(pulse_encoder_dev, 0, &encoder_count, 0);

    input->qe_count_sum += encoder_count;
    input->e_angle = QE_COUNT_TO_RAD * encoder_count;
}


void mc_calc_speed(mc_input_signals_t *input)
{
    input->speed = (float)(input->qe_count_sum / MC_SPEED_CNTR_PRESCALER) * QE_COUNT_TO_RADS_PER_S;
}
