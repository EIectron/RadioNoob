#include "common.h"

int32_t constrain(int32_t value, int32_t minValue, int32_t maxValue) {
    if (value < minValue) {
        return minValue;
    } else if (value > maxValue) {
        return maxValue;
    } else {
        return value;
    }
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int16_t mapJoystickValues(int16_t val, int16_t lower, int16_t middle, int16_t upper, bool reverse)
{
  val = constrain(val, lower, upper);
	if (val < middle) {
			val = map(val, lower, middle, 1000, 1500);
	} else {
			val = map(val, middle, upper, 1500, 2000);
	}

	if (reverse) {
			val = 2000 - (val - 1000);
	}
	return val;
}

int16_t mapJoystickCRSFValues(int16_t val, int16_t lower, int16_t middle, int16_t upper, int16_t crsf_min, int16_t crsf_mid, int16_t crsf_max, bool reverse)
{
	val = constrain(val, lower, upper);
	if (val < middle) {
			val = map(val, lower, middle, crsf_min, crsf_mid);
	} else {
			val = map(val, middle, upper, crsf_mid, crsf_max);
	}

	if (reverse) {
			val = crsf_max - (val - crsf_min);
	}
	return val;
}