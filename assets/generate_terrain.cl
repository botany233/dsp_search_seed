__constant float F3 = 1.0f / 3.0f;
__constant float G3 = 1.0f / 6.0f;
__constant float MATHF_PI = 3.1415927f;
__constant float grad3[12][3] = {
	{ 1,  1,  0}, {-1,  1,  0}, { 1, -1,  0}, {-1, -1,  0},
	{ 1,  0,  1}, {-1,  0,  1}, { 1,  0, -1}, {-1,  0, -1},
	{ 0,  1,  1}, { 0, -1,  1}, { 0,  1, -1}, { 0, -1, -1}
};

float Noise(float xin,float yin,float zin,local const short* perm,local const short* permMod12) {
	float num = (xin + yin + zin) * F3;
	int num2 = floor(xin + num);
	int num3 = floor(yin + num);
	int num4 = floor(zin + num);
	float num5 = (float)(num2 + num3 + num4) * G3;
	float num6 = (float)num2 - num5;
	float num7 = (float)num3 - num5;
	float num8 = (float)num4 - num5;
	float num9 = xin - num6;
	float num10 = yin - num7;
	float num11 = zin - num8;

    int c1 = (num9 >= num10);
    int c2 = (num10 >= num11);
    int c3 = (num9 >= num11);

    int num12 = c1 & c3;
    int num13 = (1 - c1) & c2;
    int num14 = (1 - c3) & (1 - c2);
    int num15 = c1 | c3;
    int num16 = (1 - c1) | c2;
    int num17 = (1 - c3) | (1 - c2);

	float num18 = num9 - (float)num12 + G3;
	float num19 = num10 - (float)num13 + G3;
	float num20 = num11 - (float)num14 + G3;
	float num21 = num9 - (float)num15 + 2.0 * G3;
	float num22 = num10 - (float)num16 + 2.0 * G3;
	float num23 = num11 - (float)num17 + 2.0 * G3;
	float num24 = num9 - 1.0 + 3.0 * G3;
	float num25 = num10 - 1.0 + 3.0 * G3;
	float num26 = num11 - 1.0 + 3.0 * G3;
	int num27 = num2 & 0xFF;
	int num28 = num3 & 0xFF;
	int num29 = num4 & 0xFF;
	int num30 = permMod12[num27 + perm[num28 + perm[num29]]];
	int num31 = permMod12[num27 + num12 + perm[num28 + num13 + perm[num29 + num14]]];
	int num32 = permMod12[num27 + num15 + perm[num28 + num16 + perm[num29 + num17]]];
	int num33 = permMod12[num27 + 1 + perm[num28 + 1 + perm[num29 + 1]]];

	float num34 = 0.6f - num9 * num9 - num10 * num10 - num11 * num11;
	float num35;
	if(num34 < 0.0f)
	{
		num35 = 0.0f;
	} else
	{
		num34 *= num34;
		num35 = num34 * num34 * (grad3[num30][0]*num9 + grad3[num30][1]*num10 + grad3[num30][2]*num11);
	}
	float num36 = 0.6f - num18 * num18 - num19 * num19 - num20 * num20;
	float num37;
	if(num36 < 0.0f)
	{
		num37 = 0.0f;
	} else
	{
		num36 *= num36;
		num37 = num36 * num36 * (grad3[num31][0]*num18 + grad3[num31][1]*num19 + grad3[num31][2]*num20);
	}
	float num38 = 0.6f - num21 * num21 - num22 * num22 - num23 * num23;
	float num39;
	if(num38 < 0.0f)
	{
		num39 = 0.0f;
	} else
	{
		num38 *= num38;
		num39 = num38 * num38 * (grad3[num32][0]*num21 + grad3[num32][1]*num22 + grad3[num32][2]*num23);
	}
	float num40 = 0.6f - num24 * num24 - num25 * num25 - num26 * num26;
	float num41;
	if(num40 < 0.0f)
	{
		num41 = 0.0f;
	} else
	{
		num40 *= num40;
		num41 = num40 * num40 * (grad3[num33][0]*num24 + grad3[num33][1]*num25 + grad3[num33][2]*num26);
	}
	float total = num35 + num37 + num39 + num41;
	return 32.696434f * total;
}

float Noise3DFBM_6arg(float x,float y,float z,int nOctaves,float deltaAmp,float deltaWLen,local const short* perm,local const short* permMod12)
{
	float num = 0.0f;
	float num2 = 0.5f;
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

float Noise3DFBM_5arg(float x,float y,float z,int nOctaves,float deltaAmp,local const short* perm,local const short* permMod12)
{
	return Noise3DFBM_6arg(x,y,z,nOctaves,deltaAmp,2.0f,perm,permMod12);
}

float Noise3DFBM_4arg(float x,float y,float z,int nOctaves,local const short* perm,local const short* permMod12)
{
	return Noise3DFBM_6arg(x,y,z,nOctaves,0.5f,2.0f,perm,permMod12);
}

float RidgedNoise_7arg(float x,float y,float z,int nOctaves,float deltaAmp,float deltaWLen,float initialAmp,local const short* perm,local const short* permMod12)
{
	float num = 0.0f;
	float num2 = initialAmp;
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

float Noise3DFBMInitialAmp_7arg(float x,float y,float z,int nOctaves,float deltaAmp,float deltaWLen,float initialAmp,local const short* perm,local const short* permMod12)
{
	float num = 0.0f;
	float num2 = initialAmp;
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

float Levelize_3arg(float f,float level,float offset) {
	f = f / level - offset;
	float num = floor(f);
	float num2 = f - num;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
}

float Levelize_2arg(float f,float level) {
	return Levelize_3arg(f,level,0.0f);
}

float Levelize_1arg(float f) {
	return Levelize_3arg(f,1.0f,0.0f);
}

float Levelize2_3arg(float f,float level,float offset) {
	f = f / level - offset;
	float num = floor(f);
	float num2 = f - num;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
}

float Levelize2_2arg(float f,float level) {
	return Levelize2_3arg(f,level,0.0f);
}

float Levelize2_1arg(float f) {
	return Levelize2_3arg(f,1.0f,0.0f);
}

float Levelize3_3arg(float f,float level,float offset) {
	f = f / level - offset;
	float num = floor(f);
	float num2 = f - num;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
}

float Levelize3_2arg(float f,float level) {
	return Levelize3_3arg(f,level,0.0f);
}

float Levelize3_1arg(float f) {
	return Levelize3_3arg(f,1.0f,0.0f);
}

float Levelize4_3arg(float f,float level,float offset) {
	f = f / level - offset;
	float num = floor(f);
	float num2 = f - num;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	num2 = (3.0f - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
}

float Levelize4_2arg(float f,float level) {
	return Levelize4_3arg(f,level,0.0f);
}

float Levelize4_1arg(float f) {
	return Levelize4_3arg(f,1.0f,0.0f);
}

float Remap(float sourceMin,float sourceMax,float targetMin,float targetMax,float x)
{
	return (x - sourceMin) / (sourceMax - sourceMin) * (targetMax - targetMin) + targetMin;
}

float Lerp(float a,float b,float t) {
	return a + (b - a) * t;
}

float lerp(float a,float b,float t) {
	return a + (b - a) * clamp(t,0.0f,1.0f);
}

float CurveEvaluate(float t)
{
	t /= 0.6f;
	if(t >= 1.0f)
	{
		return 0.0f;
	}
	return pow(1.0f - t,3.0f) + pow(1.0f - t,2.0f) * 3.0f * t;
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
	heightData[gid] = (unsigned short)(local_planet_radius * 100.0f);
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
	float num8 = vertices[index] * local_custom[0];
	float num9 = vertices[index+1] * local_custom[0];
	float num10 = vertices[index+2] * local_custom[0];
	float num14 = Noise3DFBM_6arg(num8 * local_custom[1],num9 * local_custom[2],num10 * local_custom[3],6,0.45f,1.8f,localPerm_1,localPermMod12_1);
	float num15 = Noise3DFBM_4arg(num8 * local_custom[1] * 2.0f,num9 * local_custom[2] * 2.0f,num10 * local_custom[3] * 2.0f,3,localPerm_2,localPermMod12_2);
	float value = num14 * 3.0f + 3.0f * 0.4f;
	float num16 = 0.6f / (fabs(value) + 0.6f) - 0.25f;
	float num17 = ((num16 < 0.0f) ? (num16 * 0.3f) : num16);
	heightData[gid] = (unsigned short)((local_custom[0] + num17 + 0.1f) * 100.0f);
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
	float num7 = vertices[index] * local_custom[0];
	float num8 = vertices[index+1] * local_custom[0];
	float num9 = vertices[index+2] * local_custom[0];
	float num12 = Noise3DFBM_6arg(num7 * 0.007f,num8 * 0.007f,num9 * 0.007f,4,0.45f,1.8f,localPerm_1,localPermMod12_1);
	float num13 = Noise3DFBM_4arg(num7 * 0.007f * 5.0f,num8 * 0.007f * 5.0f,num9 * 0.007f * 5.0f,4,localPerm_2,localPermMod12_2);
	float num14 = num12 * 1.5f;
	float num15 = num13 * 0.2f;
	float num16 = num14 * 0.08f + num15 * 2.0f;
	float num17 = 0.0f;
	for(int k = 0; k < 80; k++)
	{
		float num18 = local_custom[k*4+81] - num7;
		float num19 = local_custom[k*4+82] - num8;
		float num20 = local_custom[k*4+83] - num9;
		float num21 = num18 * num18 + num19 * num19 + num20 * num20;
		if(num21 <= local_custom[k*4+84])
		{
			float num22 = num21 / local_custom[k*4+84] + num15 * 1.2f;
			if(num22 < 0.0f)
			{
				num22 = 0.0f;
			}
			float num23 = num22 * num22;
			float num24 = num23 * num22;
			float num25 = -15.0f * num24 + 21.833333333334f * num23 - 7.533333333333f * num22 + 0.7f + num15;
			if(num25 < 0.0f)
			{
				num25 = 0.0f;
			}
			num25 *= num25;
			num25 *= local_custom[k+1];
			num17 = ((num17 > num25) ? num17 : num25);
		}
	}
	float num10 = num17 + num16 + 0.2f;
	heightData[gid] = (unsigned short)((local_custom[0] + num10 + 0.1f) * 100.0f);
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
	float num3 = vertices[index] * local_planet_radius;
	float num4 = vertices[index+1] * local_planet_radius;
	float num5 = vertices[index+2] * local_planet_radius;
	float num6 = 0.0f;
	float num8 = Levelize_1arg(num3 * 0.007f);
	float num9 = Levelize_1arg(num4 * 0.007f);
	float num10 = Levelize_1arg(num5 * 0.007f);
	num8 += Noise(num3 * 0.05f,num4 * 0.05f,num5 * 0.05f,localPerm_1,localPermMod12_1) * 0.04f;
	num9 += Noise(num4 * 0.05f,num5 * 0.05f,num3 * 0.05f,localPerm_1,localPermMod12_1) * 0.04f;
	num10 += Noise(num5 * 0.05f,num3 * 0.05f,num4 * 0.05f,localPerm_1,localPermMod12_1) * 0.04f;
	float num11 = fabs(Noise(num8,num9,num10,localPerm_2,localPermMod12_2));
	float num12 = (0.16f - num11) * 10.0f;
	num12 = ((!(num12 > 0.0f)) ? 0.0f : ((num12 > 1.0f) ? 1.0f : num12));
	num12 *= num12;
	float num13 = (Noise3DFBM_4arg(num4 * 0.005f,num5 * 0.005f,num3 * 0.005f,4,localPerm_1,localPermMod12_1) + 0.22f) * 5.0f;
	num13 = ((!(num13 > 0.0f)) ? 0.0f : ((num13 > 1.0f) ? 1.0f : num13));
	float num14 = fabs(Noise3DFBM_4arg(num8 * 1.5f,num9 * 1.5f,num10 * 1.5f,2,localPerm_2,localPermMod12_2));
	num6 -= num12 * 1.2f * num13;
	if(num6 >= 0.0f)
	{
		num6 += num11 * 0.25f + num14 * 0.6f;
	}
	num6 -= 0.1f;
	float num16 = -0.3f - num6;
	if(num16 > 0.0f)
	{
		float num17 = Noise(num3 * 0.16f,num4 * 0.16f,num5 * 0.16f,localPerm_2,localPermMod12_2) - 1.0f;
		num16 = ((num16 > 1.0f) ? 1.0f : num16);
		num16 = (3.0f - num16 - num16) * num16 * num16;
		num6 = -0.3f - num16 * 3.700000047683716f + num16 * num16 * num16 * num16 * num17 * 0.5f;
	}
	heightData[gid] = (unsigned short)((local_planet_radius + num6 + 0.2f) * 100.0f);
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
	float num3 = vertices[index] * local_planet_radius;
	float num4 = vertices[index+1] * local_planet_radius;
	float num5 = vertices[index+2] * local_planet_radius;
	float num6 = 0.0f;
	float num8 = Levelize_1arg(num3 * 0.007f);
	float num9 = Levelize_1arg(num4 * 0.007f);
	float num10 = Levelize_1arg(num5 * 0.007f);
	num8 += Noise(num3 * 0.05f,num4 * 0.05f,num5 * 0.05f,localPerm_1,localPermMod12_1) * 0.04f;
	num9 += Noise(num4 * 0.05f,num5 * 0.05f,num3 * 0.05f,localPerm_1,localPermMod12_1) * 0.04f;
	num10 += Noise(num5 * 0.05f,num3 * 0.05f,num4 * 0.05f,localPerm_1,localPermMod12_1) * 0.04f;
	float num11 = fabs(Noise(num8,num9,num10,localPerm_2,localPermMod12_2));
	float num12 = (0.16f - num11) * 10.0f;
	num12 = ((num12 <= 0.0f) ? 0.0f : ((num12 > 1.0f) ? 1.0f : num12));
	num12 *= num12;
	float num13 = (Noise3DFBM_4arg(num4 * 0.005f,num5 * 0.005f,num3 * 0.005f,4,localPerm_1,localPermMod12_1) + 0.22f) * 5.0f;
	num13 = ((!(num13 > 0.0f)) ? 0.0f : ((num13 > 1.0f) ? 1.0f : num13));
	float num14 = fabs(Noise3DFBM_4arg(num8 * 1.5f,num9 * 1.5f,num10 * 1.5f,2,localPerm_2,localPermMod12_2));
	num6 -= num12 * 1.2f * num13;
	if(num6 >= 0.0f)
	{
		num6 += num11 * 0.25f + num14 * 0.6f;
	}
	num6 -= 0.1f;
	float num15 = -0.3f - num6;
	if(num15 > 0.0f)
	{
		num15 = ((num15 > 1.0f) ? 1.0f : num15);
		num15 = (3.0f - num15 - num15) * num15 * num15;
		num6 = -0.3f - num15 * 3.7f;
	}
	float f = ((num12 > 0.3f) ? num12 : 0.3f);
	f = Levelize_2arg(f,0.7f);
	num6 = ((num6 > -0.8f) ? num6 : ((0.0f - f - num11) * 0.9f));
	num6 = ((num6 > -1.2f) ? num6 : (-1.2f));
	heightData[gid] = (unsigned short)((local_planet_radius + num6 + 0.2f) * 100.0f);
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
	float num12 = vertices[index] * local_planet_radius;
	float num13 = vertices[index+1] * local_planet_radius;
	float num14 = vertices[index+2] * local_planet_radius;
	float num17 = Noise3DFBM_4arg(num12 * 0.008f,num13 * 0.01f,num14 * 0.01f,6,localPerm_1,localPermMod12_1) * 3.0f - 2.4f;
	float num18 = Noise3DFBM_4arg(num12 * 0.0025f,num13 * 0.0025f,num14 * 0.0025f,3,localPerm_2,localPermMod12_2) * 3.0f * 0.9f + 0.5f;
	float num19 = ((num18 > 0.0f) ? (num18 * 0.5f) : num18);
	float num20 = num17 + num19;
	float num21 = ((num20 > 0.0f) ? (num20 * 0.5f) : (num20 * 1.6f));
	float num22 = ((num21 > 0.0f) ? Levelize3_2arg(num21,0.7f) : Levelize2_2arg(num21,0.5f));
	heightData[gid] = (unsigned short)((local_planet_radius + num22) * 100.0f);
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
	float num4 = vertices[index] * local_custom[0];
	float num5 = vertices[index+1] * local_custom[0];
	float num6 = vertices[index+2] * local_custom[0];
	float num9 = clamp(Noise3DFBM_6arg(num4 * local_custom[1],num5 * local_custom[2],num6 * local_custom[3],6,0.45f,1.8f,localPerm_1,localPermMod12_1) + 1.0f + local_custom[4] * 0.01f,0.0f,2.0f);
	float num10 = 0.0f;
	if(num9 < 1.0f)
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
	heightData[gid] = (unsigned short)((local_custom[0] + num10 + 0.1f) * 100.0f);
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
	float num12 = vertices[index] * local_custom[0];
	float num13 = vertices[index+1] * local_custom[0];
	float num14 = vertices[index+2] * local_custom[0];
	float num15 = 0.0f;
	float num17 = Noise3DFBM_4arg(num12 * 0.01f * 0.75f,num13 * 0.012f * 0.5f,num14 * 0.01f * 0.75f,6,localPerm_1,localPermMod12_1) * 3.0f - 0.2f;
	float num18 = Noise3DFBM_4arg(num12 * 0.0025f,num13 * 0.0025f,num14 * 0.0025f,3,localPerm_2,localPermMod12_2) * 3.0f * 0.9f + 0.5f;
	float num19 = ((num18 > 0.0f) ? (num18 * 0.5f) : num18);
	float num20 = num17 + num19;
	float num21 = ((num20 > 0.0f) ? (num20 * 0.5f) : (num20 * 1.6f));
	float num22 = ((num21 > 0.0f) ? Levelize3_2arg(num21,0.7f) : Levelize2_2arg(num21,0.5f));
	num22 += 0.618f;
	num22 = ((num22 > -1.0f) ? (num22 * 1.5f) : (num22 * 4.0f));
	float num23 = Noise3DFBM_4arg(num12 * 0.01f * 2.5f,num13 * 0.012f * 8.0f,num14 * 0.01f * 2.5f,2,localPerm_2,localPermMod12_2) * 0.6f - 0.3f;
	float num24 = num21 * 2.5f + num23 + 0.3f;
	float val = Levelize_1arg(num21 + 0.7f);
	float num25 = Noise3DFBM_4arg(num12 * 0.01f * local_custom[1],num13 * 0.012f * local_custom[1],num14 * 0.01f * local_custom[1],6,localPerm_1,localPermMod12_1) * 3.0f - 0.2f;
	float num26 = Noise3DFBM_4arg(num12 * 0.0025f,num13 * 0.0025f,num14 * 0.0025f,3,localPerm_2,localPermMod12_2) * 3.0f * 0.9f + 0.5f;
	float num27 = ((num26 > 0.0f) ? (num26 * 0.5f) : num26);
	float x = (num25 + num27 + 5.0f) * 0.13f;
	x = pow(x,6.0f) * 24.0f - 24.0f;
	float num28 = ((num22 >= 0.0f - local_custom[2]) ? 0.0f : pow(fmin(fabs(num22 + local_custom[2]) / 5.0f,1.0f),1.0f));
	num15 = num22 * (1.0f - num28) + x * num28;
	num15 = ((num15 > 0.0f) ? (num15 * 0.5f) : num15);
	heightData[gid] = (unsigned short)((local_custom[0] + num15 + 0.2f) * 100.0f);
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
	float num9 = vertices[index] * local_custom[0];
	float num10 = vertices[index+1] * local_custom[0];
	float num11 = vertices[index+2] * local_custom[0];
	float num12 = Levelize_1arg(num9 * 0.007f);
	float num13 = Levelize_1arg(num10 * 0.007f);
	float num14 = Levelize_1arg(num11 * 0.007f);
	num12 += Noise(num9 * 0.05f,num10 * 0.05f,num11 * 0.05f,localPerm_3,localPermMod12_3) * 0.04f;
	num13 += Noise(num10 * 0.05f,num11 * 0.05f,num9 * 0.05f,localPerm_3,localPermMod12_3) * 0.04f;
	num14 += Noise(num11 * 0.05f,num9 * 0.05f,num10 * 0.05f,localPerm_3,localPermMod12_3) * 0.04f;
	float num15 = fabs(Noise(num12,num13,num14,localPerm_4,localPermMod12_4));
	float num16 = (0.16f - num15) * 10.0f;
	num16 = ((num16 <= 0.0f) ? 0.0f : ((num16 > 1.0f) ? 1.0f : num16));
	num16 *= num16;
	float num17 = (Noise3DFBM_4arg(num10 * 0.005f,num11 * 0.005f,num9 * 0.005f,4,localPerm_3,localPermMod12_3) + 0.22f) * 5.0f;
	num17 = ((num17 <= 0.0f) ? 0.0f : ((num17 > 1.0f) ? 1.0f : num17));
	float num18 = fabs(Noise3DFBM_4arg(num12 * 1.5f,num13 * 1.5f,num14 * 1.5f,2,localPerm_4,localPermMod12_4));
	float num21 = Noise3DFBM_4arg(num9 * 0.007f * 5.0f,num10 * 0.007f * 5.0f,num11 * 0.007f * 5.0f,4,localPerm_2,localPermMod12_2);
	float num22 = num21 * 0.2f;
	float num23 = 0.0f;
	for(int k = 0; k < 10; k++)
	{
		float num24 = local_custom[4*k+1] - num9;
		float num25 = local_custom[4*k+2] - num10;
		float num26 = local_custom[4*k+3] - num11;
		float num27 = local_custom[k+41] * num24 * num24 + num25 * num25 + num26 * num26;
		num27 = Remap(-1.0f,1.0f,0.2f,5.0f,num21) * num27;
		if(num27 < (local_custom[4*k+4] * local_custom[4*k+4]))
		{
			float num28 = 1.0f - sqrt(num27 / (local_custom[4*k+4] * local_custom[4*k+4]));
			float num29 = 1.0f - num28;
			float num30 = 1.0f - num29 * num29 * num29 * num29 + num22 * 2.0f;
			if(num30 < 0.0f)
			{
				num30 = 0.0f;
			}
			num23 = fmax(num23,local_custom[k+51] * num30);
		}
	}
	num9 += sin(num10 * 0.15f) * 2.0f;
	num10 += sin(num11 * 0.15f) * 2.0f;
	num11 += sin(num9 * 0.15f) * 2.0f;
	num9 *= 0.007f;
	num10 *= 0.007f;
	num11 *= 0.007f;
	float f = pow(((Noise3DFBM_6arg(num9 * 0.6f,num10 * 0.6f,num11 * 0.6f,4,0.5f,1.8f,localPerm_1,localPermMod12_1) + 1.0f) * 0.5f),1.3f);
	float x = Noise3DFBM_4arg(num9 * 6.0f,num10 * 6.0f,num11 * 6.0f,5,localPerm_2,localPermMod12_2);
	x = Remap(-1.0f,1.0f,-0.1f,0.15f,x);
	float num31 = Noise3DFBM_4arg(num9 * 5.0f * 3.0f,num10 * 5.0f,num11 * 5.0f,1,localPerm_2,localPermMod12_2);
	float num32 = Noise3DFBM_4arg(num9 * 5.0f * 3.0f + num31 * 0.3f,num10 * 5.0f + num31 * 0.3f,num11 * 5.0f + num31 * 0.3f,5,localPerm_2,localPermMod12_2) * 0.1f;
	f = Levelize_1arg(Levelize4_1arg(f));
	f = min(1.0f,f);
	if(f <= 0.8f)
	{
		f = ((f <= 0.4f) ? (f + x) : (f + num32));
	}
	float a = f * 2.5f - f * num23;
	float num19 = max(a,x * 2.0f);
	float num33 = (2.0f - num19) / 2.0f;
	num19 -= num16 * 1.2f * num17 * num33;
	if(num19 >= 0.0f)
	{
		num19 += (num15 * 0.25f + num18 * 0.6f) * num33;
	}
	heightData[gid] = (unsigned short)((local_custom[0] + num19) * 100.0f);
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
	float num10 = vertices[index] * local_custom[0];
	float num11 = vertices[index+1] * local_custom[0];
	float num12 = vertices[index+2] * local_custom[0];
	float num15 = Noise3DFBM_4arg(num10 * 0.007f * 4.0f,num11 * 0.007f * 8.0f,num12 * 0.007f * 4.0f,3,localPerm_2,localPermMod12_2);
	float x = Noise3DFBM_6arg(num10 * 0.007f * 0.6f,num11 * 0.007f * 1.5f * 2.5f,num12 * 0.007f * 0.6f,6,0.45f,1.8f,localPerm_1,localPermMod12_1) * 0.95f + num15 * 0.05f;
	x = Remap(-1.0f,1.0f,0.0f,1.0f,x);
	x = pow(x,local_custom[4]);
	x += 1.0f;
	x = Levelize2_1arg(x);
	float x2 = Noise3DFBM_5arg(num10 * local_custom[1],num11 * local_custom[2],num12 * local_custom[3],5,0.55f,localPerm_3,localPermMod12_3);
	x2 = Remap(-1.0f,1.0f,0.0f,1.0f,x2);
	x2 = pow(x2,0.65f);
	float num14 = Levelize3_1arg(x2) * x;
	float num13 = (num14 - 0.4f) * 0.9f;
	num13 = fmax(-0.3f,num13);
	heightData[gid] = (unsigned short)((local_custom[0] + num13) * 100.0f);
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
	float temp = vertices[index+1];
	float num6 = fabs(asin(temp)) * 2.0f / MATHF_PI;
	float num11 = vertices[index];
	float num12 = temp * 2.5f * local_custom[2];
	float num13 = vertices[index+2];
	float num14 = Noise3DFBM_5arg(num11 * local_custom[1],num12 * local_custom[1],num13 * local_custom[1],3,0.4f,localPerm_2,localPermMod12_2) * 0.2f;
	float num9 = RidgedNoise_7arg(num11 * local_custom[1],num12 * local_custom[1] - num14,num13 * local_custom[1],6,0.7f,2.0f,0.8f,localPerm_1,localPermMod12_1);
	float num10 = Noise3DFBMInitialAmp_7arg(num11 * local_custom[1],num12 * local_custom[1] - num14,num13 * local_custom[1],6,0.6f,2.0f,0.7f,localPerm_1,localPermMod12_1);
	num10 *= num9 + num10;
	num10 = 0.2f + 8.0f * num10 * num9;
	float x = num10 + 0.5f;
	x = Remap(-8.0f,8.0f,0.0f,1.0f,x);
	x = clamp(x,0.0f,1.0f);
	x += 0.5f;
	x = pow(x,1.5f);
	x -= CurveEvaluate(num6 * 0.9f);
	float num7 = clamp(x * 2.0f,0.0f,2.0f);
	num7 = num7 * 1.1f - 0.2f;
	heightData[gid] = (unsigned short)((local_custom[0] + num7) * 100.0f);
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
	float num4 = vertices[index] * local_custom[0];
	float num5 = vertices[index+1] * local_custom[0];
	float num6 = vertices[index+2] * local_custom[0];
	float x = Remap(-1.0f,1.0f,0.0f,1.0f,Noise3DFBM_4arg(num4 * local_custom[1],num5 * local_custom[2],num6 * local_custom[3],6,localPerm_1,localPermMod12_1));
	x = pow(x,local_custom[4]) * 3.0625f;
	x = Remap(0.0f,2.0f,0.0f,4.0f,x);
	if(x < 1.0f)
	{
		x = pow(x,2.0f);
	}
	x -= 0.2f;
	
	//float num8[4];
	//num8[0] = fmin(x,4.0f);
	//num8[1] = 2.0f - 1.0f * (num8[0] - 2.0f);
	//num8[2] = 1.0f;
	//num8[3] = 1.0f + 2.0f * (num8[0] - 3.5f);

	//int choose = (int)(num8[0] > 2.0f) + (int)(num8[0] > 3.0f) + (int)(num8[0] > 3.5f);
	//heightData[gid] = (unsigned short)((local_custom[0] + num8[choose] + 0.1f) * 100.0f);

	float num8 = fmin(x,4.0f);
	if(num8 > 2.0f)
	{
		num8 = ((num8 <= 3.0f) ? (2.0f - 1.0f * (num8 - 2.0f)) : ((num8 <= 3.5f) ? 1.0f : (1.0f + 2.0f * (num8 - 3.5f))));
	}
	heightData[gid] = (unsigned short)((local_custom[0] + num8 + 0.1f) * 100.0f);
}
