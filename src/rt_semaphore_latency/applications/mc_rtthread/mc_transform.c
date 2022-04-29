#include "mc_transform.h"



void mc_clarke_park_transform(mc_input_signals_t *input, mc_tansform_t *output)
{
    output->clarke.alpha = input->ia;
    output->clarke.beta = (input->ia * ONE_BY_SQRT3) + (input->ib * TWO_BY_SQRT3);

    mc_calc_sin_cos(input->e_angle, &output->sin_angle, &output->cos_angle);

    output->park.d_axis =  output->clarke.alpha * output->cos_angle
                        + output->clarke.beta * output->sin_angle;

    output->park.d_axis = -output->clarke.alpha * output->sin_angle
                        + output->clarke.beta * output->cos_angle;
}

/******************************************************************************/
/* Function name: MCLIB_InvParkTransform                                      */
/* Function parameters: input - park transform inputs                         */
/*                      output - clarke transform output                      */
/*                      position - rotor angle                                */
/* Function return: None                                                      */
/* Description: Inverse Park Transformation.                                  */
/******************************************************************************/
void mc_inverse_park_transform(mc_tansform_t *transform)
{
     transform->clarke.alpha =  transform->park.d_axis * transform->cos_angle - transform->park.q_axis * transform->sin_angle;
     transform->clarke.beta  =  transform->park.d_axis * transform->sin_angle + transform->park.q_axis * transform->cos_angle;
}

