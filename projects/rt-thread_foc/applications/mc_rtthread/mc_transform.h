/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-21     Tristan       the first version
 */

#ifndef APPLICATIONS_MC_RTTHREAD_MC_TRANSFORM_H_
#define APPLICATIONS_MC_RTTHREAD_MC_TRANSFORM_H_

#include "mc_user_definitions.h"
#include "mc_current.h"
#include "mc_encoder.h"

#define     SQRT3                                  ((float)1.732)
#define     SQRT3_BY2                              (float)(0.866025403788)

#define     ONE_BY_SQRT3                           (float)(0.5773502691)
#define     TWO_BY_SQRT3                           (float)(1.1547005384)




typedef struct mc_park_tansform_t
{
    float q_axis;
    float d_axis;
} mc_park_tansform_t;

typedef struct mc_clarke_tansform_t
{
    float alpha;
    float beta;
} mc_clarke_tansform_t;

typedef struct mc_tansform_t
{
    mc_clarke_tansform_t    clarke;
    mc_park_tansform_t      park;
    float cos_angle;
    float sin_angle;
} mc_tansform_t;


void mc_calc_sin_cos(float const rotor_angle, float* sin_angle, float* cos_angle);

void mc_clarke_park_transform(mc_input_signals_t *input, mc_tansform_t *output);
void mc_inverse_park_transform(mc_tansform_t *transform);
void mc_impose_limits(float * const input, const float limit_lower, const float limit_upper);
void mc_wrap_angle(float * const angle);




#endif /* APPLICATIONS_MC_RTTHREAD_MC_TRANSFORM_H_ */
