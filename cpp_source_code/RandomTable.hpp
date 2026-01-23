#pragma once
#include <vector>
#include "util.hpp"
#include "VectorLF3.hpp"
#include "SystemRandom.hpp"

class RandomTable
{
public:
	inline static std::vector<VectorLF3> sphericNormal = std::vector<VectorLF3>(65536);
	static double Normal(SystemRandom& rand)
	{
		double num = rand.NextDouble();
		double a = rand.NextDouble() * Math.PI * 2.0;
		return Math.Sqrt(-2.0 * Math.Log(1.0 - num)) * Math.Sin(a);
	}

	static VectorLF3 SphericNormal(int& seed, double scale)
	{
		seed++;
		seed &= 65535;
		return VectorLF3(sphericNormal[seed].x * scale, sphericNormal[seed].y * scale, sphericNormal[seed].z * scale);
	}

	static void GenerateSphericNormal()
	{
		static bool is_init = false;
		if (is_init)
			return;
		is_init = true;
		SystemRandom random = SystemRandom(1001);
		for (int i = 0; i < 65536; i++)
		{
			double num4;
			double num5;
			double num;
			double num2;
			double num3;
			while (true)
			{
				num = random.NextDouble() * 2.0 - 1.0;
				num2 = random.NextDouble() * 2.0 - 1.0;
				num3 = random.NextDouble() * 2.0 - 1.0;
				num4 = Normal(random);
				if (!(num4 > 5.0) && !(num4 < -5.0))
				{
					num5 = num * num + num2 * num2 + num3 * num3;
					if (!(num5 > 1.0) && !(num5 < 1E-06))
					{
						break;
					}
				}
			}
			double num6 = num4 / Math.Sqrt(num5);
			num *= num6;
			num2 *= num6;
			num3 *= num6;
			sphericNormal[i] = VectorLF3(num, num2, num3);
		}
	};
};
