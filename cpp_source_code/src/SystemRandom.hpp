#pragma once

#include <limits>
#include <cmath>

class SystemRandom
{
private:
	int _seedArray[56];
	int _inext;
	int _inextp;

public:
	SystemRandom(int seed) {
		int num = 0;
		int num2 = 161803398 - ((seed == INT_MIN) ? INT_MAX : std::abs(seed));
		_seedArray[55] = num2;
		int num3 = 1;
		for (int i = 1; i < 55; i++)
		{
			if ((num += 21) >= 55)
			{
				num -= 55;
			}
			_seedArray[num] = num3;
			num3 = num2 - num3;
			if (num3 < 0)
			{
				num3 += INT_MAX;
			}
			num2 = _seedArray[num];
		}
		for (int j = 1; j < 5; j++)
		{
			for (int k = 1; k < 56; k++)
			{
				int num4 = k + 30;
				if (num4 >= 55)
				{
					num4 -= 55;
				}
				_seedArray[k] -= _seedArray[1 + num4];
				if (_seedArray[k] < 0)
				{
					_seedArray[k] += INT_MAX;
				}
			}
		}
		_inext = 0;
		_inextp = 21;
		seed = 1;
	};

	int InternalSample() {
		int inext = _inext;
		int inextp = _inextp;
		if (++inext >= 56)
		{
			inext = 1;
		}
		if (++inextp >= 56)
		{
			inextp = 1;
		}
		int num = _seedArray[inext] - _seedArray[inextp];
		if (num == INT_MAX)
		{
			num--;
		}
		if (num < 0)
		{
			num += INT_MAX;
		}
		_seedArray[inext] = num;
		_inext = inext;
		_inextp = inextp;
		return num;
	};

	double NextDouble() {
		return (double)InternalSample() * 4.656612875245797E-10;
	}
};
