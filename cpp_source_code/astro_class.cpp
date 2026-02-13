#include <cstdint>

#include "astro_class.hpp"
#include "PlanetAlgorithm.hpp"

static uint16_t get_has_veins(const uint16_t *veins_group,const uint16_t *veins_point) {
	uint16_t has_veins = 0;
	for(int i=0;i<14;i++) {
		has_veins |= (veins_group[i] > 0 || veins_point[i] > 0) << i;
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

	float temp[14] = {0};

	for(int i = 0; i < themeProto.VeinSpot.size(); i++)
		veins_group[i] = themeProto.VeinSpot[i];
	for(int i = 0; i < themeProto.VeinCount.size(); i++)
		temp[i] = themeProto.VeinCount[i];

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
	temp[8] = 0.7f;
	++veins_group[9];
	++veins_group[9];
	for(int index = 1; index < 12 && dotNet35Random1.NextDouble() < 0.449999988079071; ++index)
		++veins_group[9];
	temp[9] = 0.7f;
	++veins_group[11];
	for(int index = 1; index < 12 && dotNet35Random1.NextDouble() < 0.5; ++index)
		++veins_group[11];
	temp[11] = 0.7f;
	break;
	case EStarType::NeutronStar:
	p = 4.5f;
	++veins_group[13];
	for(int index = 1; index < 12 && dotNet35Random1.NextDouble() < 0.649999976158142; ++index)
		++veins_group[13];
	temp[13] = 0.7f;
	break;
	case EStarType::BlackHole:
	p = 5.0f;
	++veins_group[13];
	for(int index = 1; index < 12 && dotNet35Random1.NextDouble() < 0.649999976158142; ++index)
		++veins_group[13];
	temp[13] = 0.7f;
	break;
	}

	for(int index1 = 0; index1 < themeProto.RareVeins.size(); ++index1)
	{
		int rareVein = themeProto.RareVeins[index1];
		float num2 = star->index == 0 ? themeProto.RareSettings[index1 * 4] : themeProto.RareSettings[index1 * 4 + 1];
		float rareSetting1 = themeProto.RareSettings[index1 * 4 + 2];
		float rareSetting2 = themeProto.RareSettings[index1 * 4 + 3];
		float num3 = rareSetting2;
		float num4 = 1.0f - Mathf.Pow(1.0f - num2,p);
		float num5 = 1.0f - Mathf.Pow(1.0f - rareSetting2,p);
		float num6 = 1.0f - Mathf.Pow(1.0f - num3,p);
		if(dotNet35Random1.NextDouble() < (double)num4)
		{
			++veins_group[rareVein-1];
			temp[rareVein-1] = num5;
			for(int index2 = 1; index2 < 12 && dotNet35Random1.NextDouble() < (double)rareSetting1; ++index2)
				++veins_group[rareVein-1];
		}
	}
	for(int i=0;i<14;i++)
	{
		if(veins_group[i]>1)
			veins_group[i] += 1;
		veins_point[i] = Mathf.RoundToInt(temp[i]*24.0f) * veins_group[i];
	}
	veins_point[6] = veins_group[6]; //油井单独处理
	for(int i=0;i<14;i++) {
		star->upper_veins_group[i] += veins_group[i];
		star->upper_veins_point[i] += veins_point[i];
		star->galaxy->veins_group[i] += veins_group[i];
		star->galaxy->veins_point[i] += veins_point[i];
	}
	has_veins = get_has_veins(veins_group,veins_point);
}

void PlanetClassSimple::generate_real_veins() {
	std::unique_ptr planet_algorithm = PlanetAlgorithmManager(algoId);
	planet_algorithm->GenerateTerrain(*this);
	planet_algorithm->GenerateVeins(*this,this->star->galaxy->birthPlanetId);
	is_real_veins = true;
	for(int i=0;i<14;i++) {
		star->real_veins_group[i] += veins_group[i];
		star->real_veins_point[i] += veins_point[i];
		star->galaxy->veins_group[i] += veins_group[i];
		star->galaxy->veins_point[i] += veins_point[i];
	}
}
