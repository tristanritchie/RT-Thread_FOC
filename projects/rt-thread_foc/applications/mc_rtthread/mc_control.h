/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */

typedef struct mc_pi_controller_t
{
    float kp;
    float ki;
    float kc;
    float out_max;
    float out_min;
    float d_sum;
    float in_ref;
    float in_meas;
    float out;
} mc_pi_controller_t;

#ifdef TORQUE_CONTROL_ENABLE
#define D_AXIS_CONTROLLER_INIT (mc_pi_controller_t)  \
{                                       \
    .kp = D_AXIS_CONTROLLER_KP,         \
    .ki = D_AXIS_CONTROLLER_KI,         \
    .kc = D_AXIS_CONTROLLER_KC,         \
    .out_max = D_AXIS_CONTROLLER_MAX,   \
    .out_min = -D_AXIS_CONTROLLER_MAX,   \
}
#define Q_AXIS_CONTROLLER_INIT (mc_pi_controller_t)  \
{                                       \
    .kp = Q_AXIS_CONTROLLER_KP,         \
    .ki = Q_AXIS_CONTROLLER_KI,         \
    .kc = Q_AXIS_CONTROLLER_KC,         \
    .out_max = Q_AXIS_CONTROLLER_MAX,   \
    .out_min = -Q_AXIS_CONTROLLER_MAX,   \
}
#ifdef SPEESPEED_CONTROL_ENABLE
#define SPEED_CONTROLLER_INIT (mc_pi_controller_t)  \
{                                       \
    .kp = SPEED_CONTROLLER_KP,         \
    .ki = SPEED_CONTROLLER_KI,         \
    .kc = SPEED_CONTROLLER_KC,         \
    .out_max = SPEED_CONTROLLER_MAX,   \
    .out_min = -SPEED_CONTROLLER_MAX,   \
}
#endif /* SPEED_CONTROL_ENABLE */
#endif /* TORQUE_CONTROL_ENABLE */


void mc_pi_control(mc_pi_controller_t *const param);
void mc_reset_controller(mc_pi_controller_t *const param);
