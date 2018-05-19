#ifndef MAX9744_HPP_
#define MAX9744_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include "sdkconfig.h"

#include <stdlib.h>
#include <stdint.h>

#if defined(CONFIG_MAX9744_ESPIDF_ENABLED)

void max9744_init();

int max9744_set_filterless_modulation();
int max9744_set_classic_pwm_modulation();

int max9744_increase_volume();
int max9744_decrease_volume();

int max9744_set_volume(uint8_t volume);

#endif 

#ifdef __cplusplus
}
#endif

#endif
