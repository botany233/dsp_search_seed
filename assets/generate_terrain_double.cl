__constant double F3 = 1.0 / 3.0;
__constant double G3 = 1.0 / 6.0;
__constant float MATHF_PI = 3.1415927f;
__constant double grad3[12][3] = {
    { 1,  1,  0}, {-1,  1,  0}, { 1, -1,  0}, {-1, -1,  0},
    { 1,  0,  1}, {-1,  0,  1}, { 1,  0, -1}, {-1,  0, -1},
    { 0,  1,  1}, { 0, -1,  1}, { 0,  1, -1}, { 0, -1, -1}
};

double Noise(double xin,double yin,double zin,local const short* perm,local const short* permMod12) {
	double G3 = 1.0 / 6.0;
	double F3 = 1.0 / 3.0;
	double num = (xin + yin + zin) * F3;
	int num2 = floor(xin + num);
	int num3 = floor(yin + num);
	int num4 = floor(zin + num);
	double num5 = (double)(num2 + num3 + num4) * G3;
	double num6 = (double)num2 - num5;
	double num7 = (double)num3 - num5;
	double num8 = (double)num4 - num5;
	double num9 = xin - num6;
	double num10 = yin - num7;
	double num11 = zin - num8;

    int c1 = (num9 >= num10);
    int c2 = (num10 >= num11);
    int c3 = (num9 >= num11);

    int num12 = c1 & c3;
    int num13 = (1 - c1) & c2;
    int num14 = (1 - c3) & (1 - c2);
    int num15 = c1 | c3;
    int num16 = (1 - c1) | c2;
    int num17 = (1 - c3) | (1 - c2);

	double num18 = num9 - (double)num12 + G3;
	double num19 = num10 - (double)num13 + G3;
	double num20 = num11 - (double)num14 + G3;
	double num21 = num9 - (double)num15 + 2.0 * G3;
	double num22 = num10 - (double)num16 + 2.0 * G3;
	double num23 = num11 - (double)num17 + 2.0 * G3;
	double num24 = num9 - 1.0 + 3.0 * G3;
	double num25 = num10 - 1.0 + 3.0 * G3;
	double num26 = num11 - 1.0 + 3.0 * G3;
	int num27 = num2 & 0xFF;
	int num28 = num3 & 0xFF;
	int num29 = num4 & 0xFF;
	int num30 = permMod12[num27 + perm[num28 + perm[num29]]];
	int num31 = permMod12[num27 + num12 + perm[num28 + num13 + perm[num29 + num14]]];
	int num32 = permMod12[num27 + num15 + perm[num28 + num16 + perm[num29 + num17]]];
	int num33 = permMod12[num27 + 1 + perm[num28 + 1 + perm[num29 + 1]]];

	double num34 = 0.6 - num9 * num9 - num10 * num10 - num11 * num11;
	double num35;
	if(num34 < 0.0)
	{
		num35 = 0.0;
	} else
	{
		num34 *= num34;
		num35 = num34 * num34 * (grad3[num30][0]*num9 + grad3[num30][1]*num10 + grad3[num30][2]*num11);
	}
	double num36 = 0.6 - num18 * num18 - num19 * num19 - num20 * num20;
	double num37;
	if(num36 < 0.0)
	{
		num37 = 0.0;
	} else
	{
		num36 *= num36;
		num37 = num36 * num36 * (grad3[num31][0]*num18 + grad3[num31][1]*num19 + grad3[num31][2]*num20);
	}
	double num38 = 0.6 - num21 * num21 - num22 * num22 - num23 * num23;
	double num39;
	if(num38 < 0.0)
	{
		num39 = 0.0;
	} else
	{
		num38 *= num38;
		num39 = num38 * num38 * (grad3[num32][0]*num21 + grad3[num32][1]*num22 + grad3[num32][2]*num23);
	}
	double num40 = 0.6 - num24 * num24 - num25 * num25 - num26 * num26;
	double num41;
	if(num40 < 0.0)
	{
		num41 = 0.0;
	} else
	{
		num40 *= num40;
		num41 = num40 * num40 * (grad3[num33][0]*num24 + grad3[num33][1]*num25 + grad3[num33][2]*num26);
	}

	double total = num35 + num37 + num39 + num41;
	return 32.696434 * total;
}

double RidgedNoise_7arg(double x,double y,double z,int nOctaves,double deltaAmp,double deltaWLen,double initialAmp,local const short* perm,local const short* permMod12)
{
	double num = 0.0;
	double num2 = initialAmp;
	for(int i = 0; i < nOctaves; i++)
	{
		num += fabs(Noise(x,y,z,perm,permMod12) * num2);
		num2 *= deltaAmp;
		x *= deltaWLen;
		y *= deltaWLen;
		z *= deltaWLen;
	}
	return num;
}

double Noise3DFBMInitialAmp_7arg(double x,double y,double z,int nOctaves,double deltaAmp,double deltaWLen,double initialAmp,local const short* perm,local const short* permMod12)
{
	double num = 0.0;
	double num2 = initialAmp;
	for(int i = 0; i < nOctaves; i++)
	{
		num += Noise(x,y,z,perm,permMod12) * num2;
		num2 *= deltaAmp;
		x *= deltaWLen;
		y *= deltaWLen;
		z *= deltaWLen;
	}
	return num;
}

double Noise3DFBM_6arg(double x,double y,double z,int nOctaves,double deltaAmp,double deltaWLen,local const short* perm,local const short* permMod12)
{
	double num = 0.0;
	double num2 = 0.5;
	for(int i = 0; i < nOctaves; i++)
	{
		num += Noise(x,y,z,perm,permMod12) * num2;
		num2 *= deltaAmp;
		x *= deltaWLen;
		y *= deltaWLen;
		z *= deltaWLen;
	}
	return num;
}

double Noise3DFBM_5arg(double x,double y,double z,int nOctaves,double deltaAmp,local const short* perm,local const short* permMod12)
{
	return Noise3DFBM_6arg(x,y,z,nOctaves,deltaAmp,2.0,perm,permMod12);
}

double Noise3DFBM_4arg(double x,double y,double z,int nOctaves,local const short* perm,local const short* permMod12)
{
	return Noise3DFBM_6arg(x,y,z,nOctaves,0.5,2.0,perm,permMod12);
}

double Levelize_3arg(double f,double level,double offset) {
	f = f / level - offset;
	double num = floor(f);
	double num2 = f - num;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
}

double Levelize_2arg(double f,double level) {
	return Levelize_3arg(f,level,0.0);
}

double Levelize_1arg(double f) {
	return Levelize_3arg(f,1.0,0.0);
}

double Levelize2_3arg(double f,double level,double offset) {
	f = f / level - offset;
	double num = floor(f);
	double num2 = f - num;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
}

double Levelize2_2arg(double f,double level) {
	return Levelize2_3arg(f,level,0.0);
}

double Levelize2_1arg(double f) {
	return Levelize2_3arg(f,1.0,0.0);
}

double Levelize3_3arg(double f,double level,double offset) {
	f = f / level - offset;
	double num = floor(f);
	double num2 = f - num;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
}

double Levelize3_2arg(double f,double level) {
	return Levelize3_3arg(f,level,0.0);
}

double Levelize3_1arg(double f) {
	return Levelize3_3arg(f,1.0,0.0);
}

double Levelize4_3arg(double f,double level,double offset) {
	f = f / level - offset;
	double num = floor(f);
	double num2 = f - num;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
}

double Levelize4_2arg(double f,double level) {
	return Levelize4_3arg(f,level,0.0);
}

double Levelize4_1arg(double f) {
	return Levelize4_3arg(f,1.0,0.0);
}

double Remap(double sourceMin,double sourceMax,double targetMin,double targetMax,double x)
{
	return (x - sourceMin) / (sourceMax - sourceMin) * (targetMax - targetMin) + targetMin;
}

double Lerp(double a,double b,double t) {
	return a + (b - a) * t;
}

double lerp(double a,double b,double t) {
	return a + (b - a) * clamp(t,0.0,1.0);
}

double CurveEvaluate(double t)
{
	t /= 0.6;
	if(t >= 1.0)
	{
		return 0.0;
	}
	return pow(1.0 - t,3.0) + pow(1.0 - t,2.0) * 3.0 * t;
}

kernel void GenerateTerrain0(
	float planet_radius,
	global unsigned short* heightData
) {
	local float local_planet_radius;
	int gid = get_global_id(0);
	int lid = get_local_id(0);
	if(lid==0) {
		local_planet_radius = planet_radius;
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	if(gid >= 161604) {
		return;
	}
	heightData[gid] = (unsigned short)(local_planet_radius * 100.0);
}

kernel void GenerateTerrain1(
	global const float* vertices,
	float planet_radius,
	global const short* perm_1,
	global const short* perm_2,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global unsigned short* heightData
	// global float* debugData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local float local_planet_radius;

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
	}
	if(lid == 0) {
		local_planet_radius = planet_radius;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num12 = vertices[index] * local_planet_radius;
	double num13 = vertices[index+1] * local_planet_radius;
	double num14 = vertices[index+2] * local_planet_radius;
	double num17 = Noise3DFBM_4arg(num12 * 0.01,num13 * 0.012,num14 * 0.01,6,localPerm_1,localPermMod12_1) * 3.0 - 0.2;
	double num18 = Noise3DFBM_4arg(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3,localPerm_2,localPermMod12_2) * 3.0 * 0.9 + 0.5;
	double num19 = ((num18 > 0.0) ? (num18 * 0.5) : num18);
	double num20 = num17 + num19;
	double num21 = ((num20 > 0.0) ? (num20 * 0.5) : (num20 * 1.6));
	double num22 = ((num21 > 0.0) ? Levelize3_2arg(num21,0.7) : Levelize2_2arg(num21,0.5));
	heightData[gid] = (unsigned short)((local_planet_radius + num22 + 0.2) * 100.0);
	// debugData[gid] = (float)Noise(num12 * 0.01,num13 * 0.012,num14 * 0.01,localPerm_1,localPermMod12_1);
}

kernel void GenerateTerrain2(
	global const float* vertices,
	global const float* custom,
	global const short* perm_1,
	global const short* perm_2,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local float local_custom[4];

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
	}
	if(lid < 4) {
		local_custom[lid] = custom[lid];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num8 = vertices[index] * local_custom[0];
	double num9 = vertices[index+1] * local_custom[0];
	double num10 = vertices[index+2] * local_custom[0];
	double num14 = Noise3DFBM_6arg(num8 * local_custom[1],num9 * local_custom[2],num10 * local_custom[3],6,0.45,1.8,localPerm_1,localPermMod12_1);
	double num15 = Noise3DFBM_4arg(num8 * local_custom[1] * 2.0,num9 * local_custom[2] * 2.0,num10 * local_custom[3] * 2.0,3,localPerm_2,localPermMod12_2);
	double value = num14 * 3.0 + 3.0 * 0.4;
	double num16 = 0.6 / (fabs(value) + 0.6) - 0.25;
	double num17 = ((num16 < 0.0) ? (num16 * 0.3) : num16);
	heightData[gid] = (unsigned short)((local_custom[0] + num17 + 0.1) * 100.0);
}

kernel void GenerateTerrain3(
	global const float* vertices,
	global const float* custom,
	global const short* perm_1,
	global const short* perm_2,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global unsigned short* heightData
	//global float* debugData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local float local_custom[2];

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
	}
	if(lid < 2) {
		local_custom[lid] = custom[lid];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num6 = vertices[index] * local_custom[0];
	double num7 = vertices[index+1] * local_custom[0];
	double num8 = vertices[index+2] * local_custom[0];
	num6 += sin(num7 * 0.15) * 3.0;
	num7 += sin(num8 * 0.15) * 3.0;
	num8 += sin(num6 * 0.15) * 3.0;
	double num11 = Noise3DFBM_6arg(num6 * 0.007,num7 * 0.007 * 1.1,num8 * 0.007,6,0.5,1.8,localPerm_1,localPermMod12_1);
	double num12 = Noise3DFBM_4arg(num6 * 0.007 * 1.3 + 0.5,num7 * 0.007 * 2.8 + 0.2,num8 * 0.007 * 1.3 + 0.7,3,localPerm_2,localPermMod12_2) * 2.0;
	double num13 = Noise3DFBM_4arg(num6 * 0.007 * 6.0,num7 * 0.007 * 12.0,num8 * 0.007 * 6.0,2,localPerm_2,localPermMod12_2) * 2.0;
	
	num13 = Lerp(num13,num13 * 0.1,local_custom[1]);
	double num14 = Noise3DFBM_4arg(num6 * 0.007 * 0.8,num7 * 0.007 * 0.8,num8 * 0.007 * 0.8,2,localPerm_2,localPermMod12_2) * 2.0;
	double num15 = num11 * 2.0 + 0.92;
	double num16 = num12 * fabs(num14 + 0.5);
	num15 += clamp(num16 - 0.35, 0.0, 1.0);
	if(num15 < 0.0)
	{
		num15 *= 2.0;
	}
	double num17 = Levelize2_1arg(num15);
	if(num17 > 0.0)
	{
		num17 = Levelize2_1arg(num15);
		num17 = Lerp(Levelize4_1arg(num17),num17,local_custom[1]);
	}
	double b = ((num17 <= 0.0) ? (lerp(-1.0,0.0,num17 + 1.0)) : ((num17 <= 1.0) ? (lerp(0.0,0.3,num17) + num13 * 0.1) : ((num17 > 2.0) ? (lerp(1.2,2.0,num17 - 2.0) + num13 * 0.12) : (lerp(0.3,1.2,num17 - 1.0) + num13 * 0.12))));
	double a = ((num17 <= 0.0) ? (lerp(-4.0,0.0,num17 + 1.0)) : ((num17 <= 1.0) ? (lerp(0.0,0.3,num17) + num13 * 0.1) : ((num17 > 2.0) ? (lerp(1.4,2.7,num17 - 2.0) + num13 * 0.12) : (lerp(0.3,1.4,num17 - 1.0) + num13 * 0.12))));
	double num18 = Lerp(a,b,local_custom[1]);
	heightData[gid] = (unsigned short)((local_custom[0] + num18 + 0.2) * 100.0);
	//debugData[gid] = num17;
}

kernel void GenerateTerrain4(
	global const float* vertices,
	global const float* custom,
	global const short* perm_1,
	global const short* perm_2,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local float local_custom[401];

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
	}
	for(int i = lid; i < 401; i += get_local_size(0)) {
		local_custom[i] = custom[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num7 = vertices[index] * local_custom[0];
	double num8 = vertices[index+1] * local_custom[0];
	double num9 = vertices[index+2] * local_custom[0];
	double num12 = Noise3DFBM_6arg(num7 * 0.007,num8 * 0.007,num9 * 0.007,4,0.45,1.8,localPerm_1,localPermMod12_1);
	double num13 = Noise3DFBM_4arg(num7 * 0.007 * 5.0,num8 * 0.007 * 5.0,num9 * 0.007 * 5.0,4,localPerm_2,localPermMod12_2);
	double num14 = num12 * 1.5;
	double num15 = num13 * 0.2;
	double num16 = num14 * 0.08 + num15 * 2.0;
	double num17 = 0.0;
	for(int k = 0; k < 80; k++)
	{
		double num18 = local_custom[k*4+81] - num7;
		double num19 = local_custom[k*4+82] - num8;
		double num20 = local_custom[k*4+83] - num9;
		double num21 = num18 * num18 + num19 * num19 + num20 * num20;
		if(num21 <= local_custom[k*4+84])
		{
			double num22 = num21 / local_custom[k*4+84] + num15 * 1.2;
			if(num22 < 0.0)
			{
				num22 = 0.0;
			}
			double num23 = num22 * num22;
			double num24 = num23 * num22;
			double num25 = -15.0 * num24 + 21.833333333334 * num23 - 7.533333333333 * num22 + 0.7 + num15;
			if(num25 < 0.0)
			{
				num25 = 0.0;
			}
			num25 *= num25;
			num25 *= local_custom[k+1];
			num17 = ((num17 > num25) ? num17 : num25);
		}
	}
	double num10 = num17 + num16 + 0.2;
	heightData[gid] = (unsigned short)((local_custom[0] + num10 + 0.1) * 100.0);
}

kernel void GenerateTerrain5(
	global const float* vertices,
	float planet_radius,
	global const short* perm_1,
	global const short* perm_2,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local float local_planet_radius;

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
	}
	if(lid == 0) {
		local_planet_radius = planet_radius;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num3 = vertices[index] * local_planet_radius;
	double num4 = vertices[index+1] * local_planet_radius;
	double num5 = vertices[index+2] * local_planet_radius;
	double num6 = 0.0;
	double num8 = Levelize_1arg(num3 * 0.007);
	double num9 = Levelize_1arg(num4 * 0.007);
	double num10 = Levelize_1arg(num5 * 0.007);
	num8 += Noise(num3 * 0.05,num4 * 0.05,num5 * 0.05,localPerm_1,localPermMod12_1) * 0.04;
	num9 += Noise(num4 * 0.05,num5 * 0.05,num3 * 0.05,localPerm_1,localPermMod12_1) * 0.04;
	num10 += Noise(num5 * 0.05,num3 * 0.05,num4 * 0.05,localPerm_1,localPermMod12_1) * 0.04;
	double num11 = fabs(Noise(num8,num9,num10,localPerm_2,localPermMod12_2));
	double num12 = (0.16 - num11) * 10.0;
	num12 = ((!(num12 > 0.0)) ? 0.0 : ((num12 > 1.0) ? 1.0 : num12));
	num12 *= num12;
	double num13 = (Noise3DFBM_4arg(num4 * 0.005f,num5 * 0.005f,num3 * 0.005f,4,localPerm_1,localPermMod12_1) + 0.22) * 5.0;
	num13 = ((!(num13 > 0.0)) ? 0.0 : ((num13 > 1.0) ? 1.0 : num13));
	double num14 = fabs(Noise3DFBM_4arg(num8 * 1.5,num9 * 1.5,num10 * 1.5,2,localPerm_2,localPermMod12_2));
	num6 -= num12 * 1.2 * num13;
	if(num6 >= 0.0)
	{
		num6 += num11 * 0.25 + num14 * 0.6;
	}
	num6 -= 0.1;
	double num16 = -0.3 - num6;
	if(num16 > 0.0)
	{
		double num17 = Noise(num3 * 0.16,num4 * 0.16,num5 * 0.16,localPerm_2,localPermMod12_2) - 1.0;
		num16 = ((num16 > 1.0) ? 1.0 : num16);
		num16 = (3.0 - num16 - num16) * num16 * num16;
		num6 = -0.3 - num16 * 3.700000047683716 + num16 * num16 * num16 * num16 * num17 * 0.5;
	}
	heightData[gid] = (unsigned short)((local_planet_radius + num6 + 0.2) * 100.0);
}

kernel void GenerateTerrain6(
	global const float* vertices,
	float planet_radius,
	global const short* perm_1,
	global const short* perm_2,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local float local_planet_radius;

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
	}
	if(lid == 0) {
		local_planet_radius = planet_radius;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num3 = vertices[index] * local_planet_radius;
	double num4 = vertices[index+1] * local_planet_radius;
	double num5 = vertices[index+2] * local_planet_radius;
	double num6 = 0.0;
	double num8 = Levelize_1arg(num3 * 0.007);
	double num9 = Levelize_1arg(num4 * 0.007);
	double num10 = Levelize_1arg(num5 * 0.007);
	num8 += Noise(num3 * 0.05,num4 * 0.05,num5 * 0.05,localPerm_1,localPermMod12_1) * 0.04;
	num9 += Noise(num4 * 0.05,num5 * 0.05,num3 * 0.05,localPerm_1,localPermMod12_1) * 0.04;
	num10 += Noise(num5 * 0.05,num3 * 0.05,num4 * 0.05,localPerm_1,localPermMod12_1) * 0.04;
	double num11 = fabs(Noise(num8,num9,num10,localPerm_2,localPermMod12_2));
	double num12 = (0.16 - num11) * 10.0;
	num12 = ((num12 <= 0.0) ? 0.0 : ((num12 > 1.0) ? 1.0 : num12));
	num12 *= num12;
	double num13 = (Noise3DFBM_4arg(num4 * 0.005,num5 * 0.005,num3 * 0.005,4,localPerm_1,localPermMod12_1) + 0.22) * 5.0;
	num13 = ((!(num13 > 0.0)) ? 0.0 : ((num13 > 1.0) ? 1.0 : num13));
	double num14 = fabs(Noise3DFBM_4arg(num8 * 1.5,num9 * 1.5,num10 * 1.5,2,localPerm_2,localPermMod12_2));
	num6 -= num12 * 1.2 * num13;
	if(num6 >= 0.0)
	{
		num6 += num11 * 0.25 + num14 * 0.6;
	}
	num6 -= 0.1;
	double num15 = -0.3 - num6;
	if(num15 > 0.0)
	{
		num15 = ((num15 > 1.0) ? 1.0 : num15);
		num15 = (3.0 - num15 - num15) * num15 * num15;
		num6 = -0.3 - num15 * 3.700000047683716;
	}
	double f = ((num12 > 0.30000001192092896) ? num12 : 0.30000001192092896);
	f = Levelize_2arg(f,0.7);
	num6 = ((num6 > -0.800000011920929) ? num6 : ((0.0 - f - num11) * 0.8999999761581421));
	num6 = ((num6 > -1.2000000476837158) ? num6 : (-1.2000000476837158));
	heightData[gid] = (unsigned short)((local_planet_radius + num6 + 0.2) * 100.0);
}

kernel void GenerateTerrain7(
	global const float* vertices,
	float planet_radius,
	global const short* perm_1,
	global const short* perm_2,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local float local_planet_radius;

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
	}
	if(lid == 0) {
		local_planet_radius = planet_radius;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num12 = vertices[index] * local_planet_radius;
	double num13 = vertices[index+1] * local_planet_radius;
	double num14 = vertices[index+2] * local_planet_radius;
	double num17 = Noise3DFBM_4arg(num12 * 0.008,num13 * 0.01,num14 * 0.01,6,localPerm_1,localPermMod12_1) * 3.0 - 2.4;
	double num18 = Noise3DFBM_4arg(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3,localPerm_2,localPermMod12_2) * 3.0 * 0.9 + 0.5;
	double num19 = ((num18 > 0.0) ? (num18 * 0.5) : num18);
	double num20 = num17 + num19;
	double num21 = ((num20 > 0.0) ? (num20 * 0.5) : (num20 * 1.6));
	double num22 = ((num21 > 0.0) ? Levelize3_2arg(num21,0.7) : Levelize2_2arg(num21,0.5));
	heightData[gid] = (unsigned short)((local_planet_radius + num22) * 100.0);
}

kernel void GenerateTerrain8(
	global const float* vertices,
	global const float* custom,
	global const short* perm_1,
	global const short* permMod12_1,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPermMod12_1[512];
	local float local_custom[5];

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPermMod12_1[i] = permMod12_1[i];
	}
	if(lid < 5) {
		local_custom[lid] = custom[lid];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num4 = vertices[index] * local_custom[0];
	double num5 = vertices[index+1] * local_custom[0];
	double num6 = vertices[index+2] * local_custom[0];
	float num9 = clamp(Noise3DFBM_6arg(num4 * local_custom[1],num5 * local_custom[2],num6 * local_custom[3],6,0.45,1.8,localPerm_1,localPermMod12_1) + 1.0 + local_custom[4] * 0.01,0.0,2.0);
	float num10 = 0.0;
	if(num9 < 1.0)
	{
		float f = cos(num9 * MATHF_PI) * 1.1f;
		f = sign(f) * pow(f,4.0f);
		f = clamp(f,-1.0f,1.0f);
		num10 = 1.0f - (f + 1.0f) * 0.5f;
	} else
	{
		float f2 = cos((num9 - 1.0f) * MATHF_PI) * 1.1f;
		f2 = sign(f2) * pow(f2,4.0f);
		f2 = clamp(f2,-1.0f,1.0f);
		num10 = 2.0f - (f2 + 1.0f) * 0.5f;
	}
	heightData[gid] = (unsigned short)((local_custom[0] + (double)num10 + 0.1) * 100.0);
}

kernel void GenerateTerrain9(
	global const float* vertices,
	global const float* custom,
	global const short* perm_1,
	global const short* perm_2,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local float local_custom[3];

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPerm_2[i] = perm_2[i];
		localPermMod12_2[i] = permMod12_2[i];
	}
	if(lid < 3) {
		local_custom[lid] = custom[lid];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num12 = vertices[index] * local_custom[0];
	double num13 = vertices[index+1] * local_custom[0];
	double num14 = vertices[index+2] * local_custom[0];
	double num17 = Noise3DFBM_4arg(num12 * 0.01 * 0.75,num13 * 0.012 * 0.5,num14 * 0.01 * 0.75,6,localPerm_1,localPermMod12_1) * 3.0 - 0.2;
	double num18 = Noise3DFBM_4arg(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3,localPerm_2,localPermMod12_2) * 3.0 * 0.9 + 0.5;
	double num19 = ((num18 > 0.0) ? (num18 * 0.5) : num18);
	double num20 = num17 + num19;
	double num21 = ((num20 > 0.0) ? (num20 * 0.5) : (num20 * 1.6));
	double num22 = ((num21 > 0.0) ? Levelize3_2arg(num21,0.7) : Levelize2_2arg(num21,0.5));
	num22 += 0.618;
	num22 = ((num22 > -1.0) ? (num22 * 1.5) : (num22 * 4.0));
	double num23 = Noise3DFBM_4arg(num12 * 0.01 * 2.5,num13 * 0.012 * 8.0,num14 * 0.01 * 2.5,2,localPerm_2,localPermMod12_2) * 0.6 - 0.3;
	double num24 = num21 * 2.5 + num23 + 0.3;
	double val = Levelize_1arg(num21 + 0.7);
	double num25 = Noise3DFBM_4arg(num12 * 0.01 * local_custom[1],num13 * 0.012 * local_custom[1],num14 * 0.01 * local_custom[1],6,localPerm_1,localPermMod12_1) * 3.0 - 0.2;
	double num26 = Noise3DFBM_4arg(num12 * 0.0025,num13 * 0.0025,num14 * 0.0025,3,localPerm_2,localPermMod12_2) * 3.0 * 0.9 + 0.5;
	double num27 = ((num26 > 0.0) ? (num26 * 0.5) : num26);
	double x = (num25 + num27 + 5.0) * 0.13;
	x = pow(x,6.0) * 24.0 - 24.0;
	double num28 = ((num22 >= 0.0 - local_custom[2]) ? 0.0 : pow(fmin(fabs(num22 + local_custom[2]) / 5.0,1.0),1.0));
	double num15 = num22 * (1.0 - num28) + x * num28;
	num15 = ((num15 > 0.0) ? (num15 * 0.5) : num15);
	heightData[gid] = (unsigned short)((local_custom[0] + num15 + 0.2) * 100.0);
}

kernel void GenerateTerrain10(
	global const float* vertices,
	global const float* custom,
	global const short* perm_1,
	global const short* perm_2,
	global const short* perm_3,
	global const short* perm_4,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global const short* permMod12_3,
	global const short* permMod12_4,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPerm_3[512];
	local short localPerm_4[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local short localPermMod12_3[512];
	local short localPermMod12_4[512];
	local float local_custom[61];

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPerm_3[i] = perm_3[i];
		localPerm_4[i] = perm_4[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
		localPermMod12_3[i] = permMod12_3[i];
		localPermMod12_4[i] = permMod12_4[i];
	}
	for(int i = lid; i < 61; i += get_local_size(0)) {
		local_custom[i] = custom[i];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num9 = vertices[index] * local_custom[0];
	double num10 = vertices[index+1] * local_custom[0];
	double num11 = vertices[index+2] * local_custom[0];
	double num12 = Levelize_1arg(num9 * 0.007);
	double num13 = Levelize_1arg(num10 * 0.007);
	double num14 = Levelize_1arg(num11 * 0.007);
	num12 += Noise(num9 * 0.05,num10 * 0.05,num11 * 0.05,localPerm_3,localPermMod12_3) * 0.04;
	num13 += Noise(num10 * 0.05,num11 * 0.05,num9 * 0.05,localPerm_3,localPermMod12_3) * 0.04;
	num14 += Noise(num11 * 0.05,num9 * 0.05,num10 * 0.05,localPerm_3,localPermMod12_3) * 0.04;
	double num15 = fabs(Noise(num12,num13,num14,localPerm_4,localPermMod12_4));
	double num16 = (0.16 - num15) * 10.0;
	num16 = ((num16 <= 0.0) ? 0.0 : ((num16 > 1.0) ? 1.0 : num16));
	num16 *= num16;
	double num17 = (Noise3DFBM_4arg(num10 * 0.005,num11 * 0.005,num9 * 0.005,4,localPerm_3,localPermMod12_3) + 0.22) * 5.0;
	num17 = ((num17 <= 0.0) ? 0.0 : ((num17 > 1.0) ? 1.0 : num17));
	double num18 = fabs(Noise3DFBM_4arg(num12 * 1.5,num13 * 1.5,num14 * 1.5,2,localPerm_4,localPermMod12_4));
	double num21 = Noise3DFBM_4arg(num9 * 0.007 * 5.0,num10 * 0.007 * 5.0,num11 * 0.007 * 5.0,4,localPerm_2,localPermMod12_2);
	double num22 = num21 * 0.2;
	double num23 = 0.0;
	for(int k = 0; k < 10; k++)
	{
		double num24 = local_custom[4*k+1] - num9;
		double num25 = local_custom[4*k+2] - num10;
		double num26 = local_custom[4*k+3] - num11;
		double num27 = local_custom[k+41] * num24 * num24 + num25 * num25 + num26 * num26;
		num27 = Remap(-1.0,1.0,0.2,5.0,num21) * num27;
		if(num27 < (local_custom[4*k+4] * local_custom[4*k+4]))
		{
			double num28 = 1.0f - sqrt((float)(num27 / (double)(local_custom[4*k+4] * local_custom[4*k+4])));
			double num29 = 1.0 - num28;
			double num30 = 1.0 - num29 * num29 * num29 * num29 + num22 * 2.0;
			if(num30 < 0.0)
			{
				num30 = 0.0;
			}
			num23 = fmax(num23,local_custom[k+51] * num30);
		}
	}
	num9 += sin(num10 * 0.15) * 2.0;
	num10 += sin(num11 * 0.15) * 2.0;
	num11 += sin(num9 * 0.15) * 2.0;
	num9 *= 0.007;
	num10 *= 0.007;
	num11 *= 0.007;
	double f = pow(((Noise3DFBM_6arg(num9 * 0.6,num10 * 0.6,num11 * 0.6,4,0.5,1.8,localPerm_1,localPermMod12_1) + 1.0) * 0.5),1.3);
	double x = Noise3DFBM_4arg(num9 * 6.0,num10 * 6.0,num11 * 6.0,5,localPerm_2,localPermMod12_2);
	x = Remap(-1.0,1.0,-0.1,0.15,x);
	double num31 = Noise3DFBM_4arg(num9 * 5.0 * 3.0,num10 * 5.0,num11 * 5.0,1,localPerm_2,localPermMod12_2);
	double num32 = Noise3DFBM_4arg(num9 * 5.0 * 3.0 + num31 * 0.3,num10 * 5.0 + num31 * 0.3,num11 * 5.0 + num31 * 0.3,5,localPerm_2,localPermMod12_2) * 0.1;
	f = (float)Levelize_1arg(Levelize4_1arg(f));
	f = min(1.0,f);
	if(f <= 0.8)
	{
		f = ((f <= 0.4) ? (f + x) : (f + num32));
	}
	double a = f * 2.5 - f * num23;
	double num19 = max(a,x * 2.0);
	double num33 = (2.0 - num19) / 2.0;
	num19 -= num16 * 1.2 * num17 * num33;
	if(num19 >= 0.0)
	{
		num19 += (num15 * 0.25 + num18 * 0.6) * num33;
	}
	heightData[gid] = (unsigned short)((local_custom[0] + num19) * 100.0);
}

kernel void GenerateTerrain11(
	global const float* vertices,
	global const float* custom,
	global const short* perm_1,
	global const short* perm_2,
	global const short* perm_3,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global const short* permMod12_3,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPerm_3[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local short localPermMod12_3[512];
	local float local_custom[5];

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPerm_3[i] = perm_3[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
		localPermMod12_3[i] = permMod12_3[i];
	}
	if(lid < 5) {
		local_custom[lid] = custom[lid];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num10 = vertices[index] * local_custom[0];
	double num11 = vertices[index+1] * local_custom[0];
	double num12 = vertices[index+2] * local_custom[0];
	double num15 = Noise3DFBM_4arg(num10 * 0.007 * 4.0,num11 * 0.007 * 8.0,num12 * 0.007 * 4.0,3,localPerm_2,localPermMod12_2);
	double x = Noise3DFBM_6arg(num10 * 0.007 * 0.6,num11 * 0.007 * 1.5 * 2.5,num12 * 0.007 * 0.6,6,0.45,1.8,localPerm_1,localPermMod12_1) * 0.95 + num15 * 0.05;
	x = Remap(-1.0,1.0,0.0,1.0,x);
	x = pow(x,(double)local_custom[4]);
	x += 1.0;
	x = Levelize2_1arg(x);
	double x2 = Noise3DFBM_5arg(num10 * local_custom[1],num11 * local_custom[2],num12 * local_custom[3],5,0.55,localPerm_3,localPermMod12_3);
	x2 = Remap(-1.0,1.0,0.0,1.0,x2);
	x2 = pow(x2,0.65);
	double num14 = Levelize3_1arg(x2) * x;
	double num13 = (num14 - 0.4) * 0.9;
	num13 = fmax(-0.3,num13);
	heightData[gid] = (unsigned short)((local_custom[0] + num13) * 100.0);
}

kernel void GenerateTerrain12(
	global const float* vertices,
	global const float* custom,
	global const short* perm_1,
	global const short* perm_2,
	global const short* permMod12_1,
	global const short* permMod12_2,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPerm_2[512];
	local short localPermMod12_1[512];
	local short localPermMod12_2[512];
	local float local_custom[3];

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPerm_2[i] = perm_2[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPermMod12_2[i] = permMod12_2[i];
	}
	if(lid < 3) {
		local_custom[lid] = custom[lid];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double temp = vertices[index+1];
	double num6 = fabs(asin(temp)) * 2.0 / MATHF_PI;
	double num11 = vertices[index];
	double num12 = temp * 2.5 * local_custom[2];
	double num13 = vertices[index+2];
	double num14 = Noise3DFBM_5arg(num11 * local_custom[1],num12 * local_custom[1],num13 * local_custom[1],3,0.4,localPerm_2,localPermMod12_2) * 0.2;
	double num9 = RidgedNoise_7arg(num11 * local_custom[1],num12 * local_custom[1] - num14,num13 * local_custom[1],6,0.7,2.0,0.8,localPerm_1,localPermMod12_1);
	double num10 = Noise3DFBMInitialAmp_7arg(num11 * local_custom[1],num12 * local_custom[1] - num14,num13 * local_custom[1],6,0.6,2.0,0.7,localPerm_1,localPermMod12_1);
	num10 *= num9 + num10;
	num10 = 0.2 + 8.0 * num10 * num9;
	double x = num10 + 0.5;
	x = Remap(-8.0,8.0,0.0,1.0,x);
	x = clamp(x,0.0,1.0);
	x += 0.5;
	x = pow(x,1.5);
	x -= CurveEvaluate(num6 * 0.9);
	double num7 = clamp(x * 2.0,0.0,2.0);
	num7 = num7 * 1.1 - 0.2;
	heightData[gid] = (unsigned short)((local_custom[0] + num7) * 100.0);
}

kernel void GenerateTerrain13(
	global const float* vertices,
	global const float* custom,
	global const short* perm_1,
	global const short* permMod12_1,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPermMod12_1[512];
	local float local_custom[5];

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPermMod12_1[i] = permMod12_1[i];
	}
	if(lid < 5) {
		local_custom[lid] = custom[lid];
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(gid >= 161604) {
		return;
	}
	int index = gid * 3;
	double num4 = vertices[index] * local_custom[0];
	double num5 = vertices[index+1] * local_custom[0];
	double num6 = vertices[index+2] * local_custom[0];
	double x = Remap(-1.0,1.0,0.0,1.0,Noise3DFBM_4arg(num4 * local_custom[1],num5 * local_custom[2],num6 * local_custom[3],6,localPerm_1,localPermMod12_1));
	x = pow(x,(double)local_custom[4]) * 3.0625;
	x = Remap(0.0,2.0,0.0,4.0,x);
	if(x < 1.0)
	{
		x = pow(x,2.0);
	}
	x -= 0.2;

	double num8 = fmin(x,4.0);
	if(num8 > 2.0)
	{
		num8 = ((num8 <= 3.0) ? (2.0 - 1.0 * (num8 - 2.0)) : ((num8 <= 3.5) ? 1.0 : (1.0 + 2.0 * (num8 - 3.5))));
	}
	heightData[gid] = (unsigned short)((local_custom[0] + num8 + 0.1) * 100.0);
}
