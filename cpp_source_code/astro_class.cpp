#include <cstdint>
#include <iostream>

#include "astro_class.hpp"
#include "PlanetAlgorithm.hpp"

using namespace std;

static uint16_t get_has_veins(const uint16_t *veins_point) {
	uint16_t has_veins = 0;
	for(int i=0;i<14;i++) {
		has_veins |= (veins_point[i] > 0) << i;
	}
	return has_veins;
}

void PlanetClassSimple::MyGenerateVeins()
{
	ThemeProto themeProto = LDB.Select(theme);
	DotNet35Random dotNet35Random1(seed);
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	dotNet35Random1.Next();

	uint16_t veins_group[14] = {0};
	float veins_count_percent[14] = {0};
	float veins_amount_percent[14] = {0};

	for(int i = 0; i < themeProto.VeinSpot.size(); i++)
		veins_group[i] = themeProto.VeinSpot[i];
	for(int i = 0; i < themeProto.VeinCount.size(); i++)
		veins_count_percent[i] = themeProto.VeinCount[i];
	for(int i = 0; i < themeProto.VeinOpacity.size(); i++)
		veins_amount_percent[i] = themeProto.VeinOpacity[i];

	float p = 1.0f;
	ESpectrType spectr = star->spectr;
	switch(star->type)
	{
	case EStarType::MainSeqStar:
	switch(spectr)
	{
	case ESpectrType::M:
	p = 2.5f;
	break;
	case ESpectrType::K:
	p = 1.0f;
	break;
	case ESpectrType::G:
	p = 0.7f;
	break;
	case ESpectrType::F:
	p = 0.6f;
	break;
	case ESpectrType::A:
	p = 1.0f;
	break;
	case ESpectrType::B:
	p = 0.4f;
	break;
	case ESpectrType::O:
	p = 1.6f;
	break;
	}
	break;
	case EStarType::GiantStar:
	p = 2.5f;
	break;
	case EStarType::WhiteDwarf:
	p = 3.5f;
	++veins_group[8];
	++veins_group[8];
	for(int index = 1; index < 12 && dotNet35Random1.NextDouble() < 0.449999988079071; ++index)
		++veins_group[8];
	veins_count_percent[8] = 0.7f;
	veins_amount_percent[8] = 1.0f;
	++veins_group[9];
	++veins_group[9];
	for(int index = 1; index < 12 && dotNet35Random1.NextDouble() < 0.449999988079071; ++index)
		++veins_group[9];
	veins_count_percent[9] = 0.7f;
	veins_amount_percent[9] = 1.0f;
	++veins_group[11];
	for(int index = 1; index < 12 && dotNet35Random1.NextDouble() < 0.5; ++index)
		++veins_group[11];
	veins_count_percent[11] = 0.7f;
	veins_amount_percent[11] = 0.3f;
	break;
	case EStarType::NeutronStar:
	p = 4.5f;
	++veins_group[13];
	for(int index = 1; index < 12 && dotNet35Random1.NextDouble() < 0.649999976158142; ++index)
		++veins_group[13];
	veins_count_percent[13] = 0.7f;
	veins_amount_percent[13] = 0.3f;
	break;
	case EStarType::BlackHole:
	p = 5.0f;
	++veins_group[13];
	for(int index = 1; index < 12 && dotNet35Random1.NextDouble() < 0.649999976158142; ++index)
		++veins_group[13];
	veins_count_percent[13] = 0.7f;
	veins_amount_percent[13] = 0.3f;
	break;
	}
	for(int index1 = 0; index1 < themeProto.RareVeins.size(); ++index1)
	{
		int rareVein = themeProto.RareVeins[index1];
		float rareSetting1 = star->index == 0 ? themeProto.RareSettings[index1 * 4] : themeProto.RareSettings[index1 * 4 + 1];
		float rareSetting2 = themeProto.RareSettings[index1 * 4 + 2];
		float rareSetting3 = themeProto.RareSettings[index1 * 4 + 3];
		rareSetting1 = 1.0f - Mathf.Pow(1.0f - rareSetting1,p);
		rareSetting3 = 1.0f - Mathf.Pow(1.0f - rareSetting3,p);
		if(dotNet35Random1.NextDouble() < (double)rareSetting1)
		{
			++veins_group[rareVein-1];
			veins_count_percent[rareVein-1] = rareSetting3;
			veins_amount_percent[rareVein-1] = rareSetting3;
			for(int index2 = 1; index2 < 12 && dotNet35Random1.NextDouble() < (double)rareSetting2; ++index2)
				++veins_group[rareVein-1];
		}
	}

	for(int i=0;i<14;i++)
	{
		if(veins_group[i]>1)
			veins_group[i] += 1;
		veins_point[i] = Mathf.RoundToInt(veins_count_percent[i]*24.0f) * veins_group[i];
	}
	veins_point[6] = veins_group[6]; //油井单独处理

	bool flag = star->galaxy->birthPlanetId == id;
	float amount_rate_galaxy = star->galaxy->resource_multiplier;
	float num8 = star->resourceCoef;
	if(flag)
		num8 *= 2.0f/3.0f;
	else if(star->galaxy->is_rare_resource)
	{
		if(num8 > 1.0f)
			num8 = Mathf.Pow(num8,0.8f);
		num8 *= 0.7f;
	}
	for(int i=0;i<14;i++)
	{
		veins_amount[i] = Mathf.RoundToInt(veins_amount_percent[i]*100000.0f*num8);
		if(veins_amount[i]<20)
			veins_amount[i] = 20;
		veins_amount[i] += (veins_amount[i]<16000)?Mathf.FloorToInt((float)veins_amount[i]*0.9375f):15000;
		veins_amount[i] = Mathf.RoundToInt((float)veins_amount[i] * 1.1f);
		if(i==6)
		{
			float oil_resource_multiplier;
			if(star->galaxy->is_rare_resource)
				oil_resource_multiplier = 0.5f;
			else
				oil_resource_multiplier = 1.0f;
			veins_amount[i] = Mathf.RoundToInt((float)veins_amount[i] * oil_resource_multiplier);
			if(veins_amount[i]<2500)
				veins_amount[i] = 2500;
		}
		else
			veins_amount[i] = Mathf.RoundToInt((float)veins_amount[i] * star->galaxy->resource_multiplier);
		if(veins_amount[i]<1)
			veins_amount[i] = 1;
		veins_amount[i] *= veins_point[i];
	}
	if(amount_rate_galaxy >= 100.0f) {
		for(int i=0;i<14;i++)
		{
			if(i==6) continue;
			veins_amount[i] = veins_point[i] * 1000000000;
		}
	}

	for(int i=0;i<14;i++) {
		star->upper_veins_point[i] += veins_point[i];
		star->upper_veins_amount[i] += veins_amount[i];
		star->galaxy->veins_point[i] += veins_point[i];
		star->galaxy->veins_amount[i] += veins_amount[i];
	}
	has_veins = get_has_veins(veins_point);
}

void PlanetClassSimple::generate_real_veins() {
	std::unique_ptr planet_algorithm = PlanetAlgorithmManager(algoId);
	planet_algorithm->GenerateTerrain(*this);
	planet_algorithm->GenerateVeins(*this,this->star->galaxy->birthPlanetId);
	is_real_veins = true;
	has_veins = get_has_veins(veins_point);
	for(int i=0;i<14;i++) {
		star->real_veins_point[i] += veins_point[i];
		star->real_veins_amount[i] += veins_amount[i];
		star->galaxy->veins_point[i] += veins_point[i];
		star->galaxy->veins_amount[i] += veins_amount[i];
	}
}
