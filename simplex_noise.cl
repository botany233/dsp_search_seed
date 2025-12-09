__constant float F3 = 1.0f / 3.0f;
__constant float G3 = 1.0f / 6.0f;
__constant float3 grad3[12] = {
    (float3)( 1,  1,  0), (float3)(-1,  1,  0), (float3)( 1, -1,  0), (float3)(-1, -1,  0),
    (float3)( 1,  0,  1), (float3)(-1,  0,  1), (float3)( 1,  0, -1), (float3)(-1,  0, -1),
    (float3)( 0,  1,  1), (float3)( 0, -1,  1), (float3)( 0,  1, -1), (float3)( 0, -1, -1)
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
	
	//向量化计算
	//float4 x_vec = (float4)(num9, num18, num21, num24);
	//float4 y_vec = (float4)(num10, num19, num22, num25);
	//float4 z_vec = (float4)(num11, num20, num23, num26);

	//float4 t_vec = x_vec * x_vec + y_vec * y_vec + z_vec * z_vec;
	//float4 att_vec = (float4)(0.6f) - t_vec;

	//float4 grad_x = (float4)(grad3[num30].x, grad3[num31].x, grad3[num32].x, grad3[num33].x);
	//float4 grad_y = (float4)(grad3[num30].y, grad3[num31].y, grad3[num32].y, grad3[num33].y);
	//float4 grad_z = (float4)(grad3[num30].z, grad3[num31].z, grad3[num32].z, grad3[num33].z);

	//float4 dot_vec = grad_x * x_vec + grad_y * y_vec + grad_z * z_vec;

	//float4 att_sq = att_vec * att_vec;
	//float4 att_quad = att_sq * att_sq;

	//float4 result_vec = att_quad * dot_vec;

	//float4 mask = select((float4)(0.0f), result_vec, att_vec < 0.0f);
	//float total_ = mask.x + mask.y + mask.z + mask.w;

	//标量计算以验证向量化结果
	float3 vec1 = (float3)(num9,num10,num11);
	float num34 = 0.6f - dot(vec1, vec1);
	float num35;
	if(num34 < 0.0f)
	{
		num35 = 0.0f;
	} else
	{
		num34 *= num34;
		num35 = num34 * num34 * dot(grad3[num30],vec1);
	}
	float3 vec2 = (float3)(num18,num19,num20);
	float num36 = 0.6f - dot(vec2, vec2);
	float num37;
	if(num36 < 0.0f)
	{
		num37 = 0.0f;
	} else
	{
		num36 *= num36;
		num37 = num36 * num36 * dot(grad3[num31],vec2);
	}
	float3 vec3 = (float3)(num21,num22,num23);
	float num38 = 0.6f - dot(vec3, vec3);
	float num39;
	if(num38 < 0.0f)
	{
		num39 = 0.0f;
	} else
	{
		num38 *= num38;
		num39 = num38 * num38 * dot(grad3[num32],vec3);
	}
	float3 vec4 = (float3)(num24,num25,num26);
	float num40 = 0.6f - dot(vec4, vec4);
	float num41;
	if(num40 < 0.0f)
	{
		num41 = 0.0f;
	} else
	{
		num40 *= num40;
		num41 = num40 * num40 * dot(grad3[num33],vec4);
	}
	float total = num35 + num37 + num39 + num41;

	//float4 diff = fabs(att_vec - (float4)(num34, num36, num38, num40));
	//if(max(max(diff.x, diff.y), max(diff.z, diff.w)) < 0.1f) {
	//	printf("%f %f %f %f | %f %f %f %f\n", att_vec.x, att_vec.y, att_vec.z, att_vec.w, num40, num38, num36, num34);
	//}

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

float Noise3DFBM_4arg(float x,float y,float z,int nOctaves,local const short* perm,local const short* permMod12)
{
	return Noise3DFBM_6arg(x,y,z,nOctaves,0.5f,2.0f,perm,permMod12);
}

float Levelize(float f,float level,float offset) {
	f = f / level - offset;
	float num = floor(f);
	float num2 = f - num;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
}

float Levelize2_3arg(float f,float level,float offset) {
	f = f / level - offset;
	float num = floor(f);
	float num2 = f - num;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
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
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
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

float Levelize4(float f,float level,float offset) {
	f = f / level - offset;
	float num = floor(f);
	float num2 = f - num;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	num2 = (3.0 - num2 - num2) * num2 * num2;
	f = num + num2;
	f = (f + offset) * level;
	return f;
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

kernel void GenerateTerrain1(
	global const float* vertices,
	float planet_radius,
	global const short* perm_1,
	global const short* permMod12_1,
	global const short* perm_2,
	global const short* permMod12_2,
	global unsigned short* heightData
) {
	int gid = get_global_id(0);
	int lid = get_local_id(0);

	local short localPerm_1[512];
	local short localPermMod12_1[512];
	local short localPerm_2[512];
	local short localPermMod12_2[512];
	local float local_planet_radius;

	// Load permutation tables into local memory
	for(int i = lid; i < 512; i += get_local_size(0)) {
		localPerm_1[i] = perm_1[i];
		localPermMod12_1[i] = permMod12_1[i];
		localPerm_2[i] = perm_2[i];
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
	float num17 = Noise3DFBM_4arg(num12 * 0.01f,num13 * 0.012f,num14 * 0.01f,6,localPerm_1,localPermMod12_1) * 3.0f - 0.2f;
	float num18 = Noise3DFBM_4arg(num12 * 0.0025f,num13 * 0.0025f,num14 * 0.0025f,3,localPerm_2,localPermMod12_2) * 3.0f * 0.9f + 0.5f;
	float num19 = ((num18 > 0.0f) ? (num18 * 0.5f) : num18);
	float num20 = num17 + num19;
	float num21 = ((num20 > 0.0f) ? (num20 * 0.5f) : (num20 * 1.6f));
	float num22 = ((num21 > 0.0f) ? Levelize3_2arg(num21,0.7f) : Levelize2_2arg(num21,0.5f));
	heightData[gid] = (unsigned short)((local_planet_radius + num22 + 0.2f) * 100.0f);
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
		localPermMod12_1[i] = permMod12_1[i];
		localPerm_2[i] = perm_2[i];
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
