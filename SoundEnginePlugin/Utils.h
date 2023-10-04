#pragma once

#include <cmath>

class Utils
{
public:
	float sgn(float input)
	{
		return (input > 0) - (input < 0);
	}

	float SaturateFunc(float Input, float Param)
	{
		float result = sgn(Input) * ((1 - exp(-fabs(Param * Input))) / (1 - exp(-Param)));
		return result;
	}
};
