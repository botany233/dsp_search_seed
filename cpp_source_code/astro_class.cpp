// Original 2022 Copyright https://github.com/crazyyao0.
// Modified by https://github.com/botany233 on 2026.2
#include "astro_class.hpp"
#include "PlanetAlgorithm.hpp"

bool PlanetClassSimple::ensure_theme() {
	if(theme != 0)
		return false;
	// 同一恒星的主题按前序结果排除，先补齐前序行星，再生成自身主题。
	while(star->themed_planet_count <= index) {
		star->planets[star->themed_planet_count].generate_theme();
		star->themed_planet_count++;
	}
	return true;
}

void PlanetClassSimple::generate_theme() {
	// 重放行星信息随机序列，保持主题、算法和修正参数的原始随机位置。
	DotNet35Random random(info_seed);
	for(int i=0;i<12;i++) random.NextDouble();
	double rand1 = random.NextDouble();
	random.NextDouble();
	double rand2 = random.NextDouble();
	double rand3 = random.NextDouble();
	double rand4 = random.NextDouble();
	random.Next();
	int tmp_theme[25];
	int tmp_theme_num = 0;

	const std::vector<int>& themeIds = LDB.themeIds;
	int length1 = (int)themeIds.size();
	for(int index1 = 0; index1 < length1; ++index1)
	{
		const ThemeProto& themeProto = LDB.Select(themeIds[index1]);
		bool flag1 = false;
		if(star->index == 0 && this->type == EPlanetType::Ocean)
		{
			if(themeProto.Distribute == EThemeDistribute::Birth)
				flag1 = true;
		} else
		{
			bool flag2 = ((double)themeProto.Temperature * (double)temperatureBias >= -static_cast<double>(0.1f));
			if(((double)Mathf.Abs(themeProto.Temperature) < 0.5) && themeProto.PlanetType == EPlanetType::Desert)
				flag2 = ((double)Mathf.Abs(temperatureBias) < (double)Mathf.Abs(themeProto.Temperature) + static_cast<double>(0.1f));
			if((themeProto.PlanetType == this->type) && flag2)
			{
				if(star->index == 0)
				{
					if(themeProto.Distribute == EThemeDistribute::Default)
						flag1 = true;
				} else if(themeProto.Distribute == EThemeDistribute::Default || themeProto.Distribute == EThemeDistribute::Interstellar)
					flag1 = true;
			}
		}
		if(flag1)
		{
			for(int index2 = 0; index2 < this->index; ++index2)
			{
				if(star->planets[index2].theme == themeProto.ID)
				{
					flag1 = false;
					break;
				}
			}
		}
		if(flag1)
			tmp_theme[tmp_theme_num++] = themeProto.ID;
	}
	if(!tmp_theme_num)
	{
		for(int index3 = 0; index3 < length1; ++index3)
		{
			const ThemeProto& themeProto = LDB.Select(themeIds[index3]);
			bool flag = false;
			if(themeProto.PlanetType == EPlanetType::Desert)
				flag = true;
			if(flag)
			{
				for(int index4 = 0; index4 < this->index; ++index4)
				{
					if(star->planets[index4].theme == themeProto.ID)
					{
						flag = false;
						break;
					}
				}
			}
			if(flag)
				tmp_theme[tmp_theme_num++] = themeProto.ID;
		}
	}
	if(!tmp_theme_num)
	{
		for(int index = 0; index < length1; ++index)
		{
			const ThemeProto& themeProto = LDB.Select(themeIds[index]);
			if(themeProto.PlanetType == EPlanetType::Desert)
				tmp_theme[tmp_theme_num++] = themeProto.ID;
		}
	}
	this->theme = tmp_theme[(int)(rand1 * (double)tmp_theme_num) % tmp_theme_num];
	const ThemeProto& themeProto1 = LDB.Select(this->theme);
	this->algoId = themeProto1.Algos[(int)(rand2 * (double)themeProto1.Algos.size()) % themeProto1.Algos.size()];
	this->mod_x = (double)themeProto1.ModX.x + rand3 * ((double)themeProto1.ModX.y - (double)themeProto1.ModX.x);
	this->mod_y = (double)themeProto1.ModY.x + rand4 * ((double)themeProto1.ModY.y - (double)themeProto1.ModY.x);

	this->type = themeProto1.PlanetType;
	this->waterItemId = themeProto1.WaterItemId;
	if(themeProto1.Distribute == EThemeDistribute::Birth){
		star->galaxy->birthPlanetId = this->id;
	}
	if(star->dysonRadius > this->maxorbitRadius * 52083.333f)
		this->dsp_level = 2;
	else {
		float temp = NORMAL_PLANET_REAL_RADIUS + themeProto1.IonHeight * 0.6f;
		float ion_enhance = std::sqrt(temp*temp-NORMAL_PLANET_REAL_RADIUS*NORMAL_PLANET_REAL_RADIUS)/temp;
		if((1.041667f-ion_enhance)*this->maxorbitRadius <= 0.00002f * star->dysonRadius)
			this->dsp_level = 1;
		else
			this->dsp_level = 0;
	}
	this->type_id = themeProto1.TypeId;
	this->type_mask = 1 << themeProto1.TypeId;
	this->vein_mask = (type == EPlanetType::Gas)? 0 : (planet_veins_mask[themeProto1.ID - 1] | star_veins_mask[star->type_id]);
	if(type == EPlanetType::Gas)
		veins_state |= EVeinsState::Real;
}

void StarClassSimple::ensure_planets() {
	// 宜居数量跨恒星累计，必须按原有恒星顺序推进基础生成。
	while(galaxy->generated_stars <= index)
		galaxy->stars[galaxy->generated_stars].init_planets();
}

static uint16_t get_planet_vein_mask(const PlanetClassSimple& planet) {
	uint16_t vein_mask = 0;
	for(int i = 0; i < 14; ++i)
		if(planet.veins_point[i] > 0)
			vein_mask |= (1 << i);
	return vein_mask;
}

void PlanetClassSimple::generate_upper_points() {
	uint16_t veins_group[14]{};
	float veins_count_percent[14]{};
	// 矿量系数与矿点上界一同生成，供后续矿量计算复用。
	veins_amount_percent.fill(0.0f);

	const ThemeProto& themeProto = LDB.Select(this->theme);
	DotNet35Random dotNet35Random1(this->seed);
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	dotNet35Random1.Next();

	for(int i = 0; i < themeProto.VeinSpot.size(); i++)
		veins_group[i] = themeProto.VeinSpot[i];
	for(int i = 0; i < themeProto.VeinCount.size(); i++)
		veins_count_percent[i] = themeProto.VeinCount[i];
	for(int i = 0; i < themeProto.VeinOpacity.size(); i++)
		veins_amount_percent[i] = themeProto.VeinOpacity[i];

	float p = 1.0f;
	ESpectrType spectr = this->star->spectr;
	switch(this->star->type)
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
		float rareSetting1 = this->star->index == 0 ? themeProto.RareSettings[index1 * 4] : themeProto.RareSettings[index1 * 4 + 1];
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
	for(int i=0;i<14;++i) {
		if(veins_group[i]>1)
			veins_group[i] += 1;
		veins_point[i] = Mathf.RoundToInt(veins_count_percent[i]*24.0f) * veins_group[i];
	}
	veins_point[6] = veins_group[6]; // 石油按油井数量统计。
	for(int i=0;i<14;i++)
		star->upper_veins_point[i] += veins_point[i];
	veins_state |= EVeinsState::UpperPoints;
	vein_mask = get_planet_vein_mask(*this);
}

void PlanetClassSimple::generate_upper_amount() {
	star->galaxy->ensure_star_position();
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
		if(amount_rate_galaxy >= 100.0f && i!=6) {
			veins_amount[i] = (uint64_t)veins_point[i] * 1000000000;
			continue;
		}
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
		} else
			veins_amount[i] = Mathf.RoundToInt((float)veins_amount[i] * star->galaxy->resource_multiplier);
		if(veins_amount[i]<1)
			veins_amount[i] = 1;
		veins_amount[i] *= veins_point[i];
	}
	for(int i=0;i<14;i++)
		star->upper_veins_amount[i] += veins_amount[i];
	veins_state |= EVeinsState::UpperAmounts;
}

bool PlanetClassSimple::ensure_upper_veins(bool amount) {
	if(veins_state & (EVeinsState::UpperAmounts | EVeinsState::Real))
		return false;
	ensure_theme();
	bool is_gen = false;
	if(!(veins_state & EVeinsState::UpperPoints)) {
		generate_upper_points();
		is_gen = true;
	}
	if(amount && !(veins_state & EVeinsState::UpperAmounts)) {
		generate_upper_amount();
		is_gen = true;
	}
	return is_gen;
}

bool PlanetClassSimple::ensure_real_veins() {
	if(veins_state & EVeinsState::Real)
		return false;
	// 出生主题影响出生点和矿脉分支，真实资源生成前必须确定出生星球。
	star->galaxy->stars[0].ensure_planets();
	star->galaxy->stars[0].ensure_themes();
	star->galaxy->ensure_star_position();
	uint16_t upper_vein_point[14]{};
	uint64_t upper_vein_amount[14]{};
	std::copy_n(veins_point.begin(),14,upper_vein_point);
	std::copy_n(veins_amount.begin(),14,upper_vein_amount);
	if(type != EPlanetType::Gas) {
		veins_point.fill(0);
		veins_amount.fill(0);
		generate_planet_veins(*this);
	}
	//同时修正恒星矿物上下限
	for(int i=0;i<14;i++) {
		star->real_veins_point[i] += veins_point[i];
		star->real_veins_amount[i] += veins_amount[i];
	}
	if(veins_state & EVeinsState::UpperPoints) {
		for(int i=0;i<14;i++)
			star->upper_veins_point[i] -= (upper_vein_point[i] - veins_point[i]);
	} else {
		for(int i=0;i<14;i++)
			star->upper_veins_point[i] += veins_point[i];
	}
	if(veins_state & EVeinsState::UpperAmounts) {
		for(int i=0;i<14;i++)
			star->upper_veins_amount[i] -= (upper_vein_amount[i] - veins_amount[i]);
	} else {
		for(int i=0;i<14;i++)
			star->upper_veins_amount[i] += veins_amount[i];
	}
	veins_state |= EVeinsState::Real;
	vein_mask = get_planet_vein_mask(*this);
	star->set_vein_mask();
	return true;
}

void PlanetClassSimple::init(StarClassSimple& parent,int index,int orbitAround,int orbitIndex,int number,bool gasGiant,int info_seed,int gen_seed) {
	PlanetClassSimple& planet = *this;
	StarClassSimple& star = parent;
	planet.singularity = 0;
	planet.waterItemId = planet.dsp_level = 0;
	planet.type_mask = 0;
	planet.theme = planet.algoId = 0;
	planet.orbitRadius = 1.0f;
	planet.mod_x = planet.mod_y = 0.0;
	planet.orbitAroundPlanet = nullptr;
	planet.moons = {};
	planet.veins_state = EVeinsState::NoVeins;
	planet.veins_point.fill(0);
	planet.veins_amount.fill(0);
	DotNet35Random dotNet35Random(info_seed);
	planet.index = index;
	planet.seed = gen_seed;
	planet.info_seed = info_seed;
	planet.orbitAround = orbitAround;
	planet.orbitIndex = orbitIndex;
	planet.number = number;
	planet.id = star.id * 100 + index + 1;
	planet.star = &star;

	if(orbitAround > 0) {
		planet.singularity |= EPlanetSingularity::Satellite;
		// 全部行星槽位已经分配，但只有前序行星可读。
		for(PlanetClassSimple& p: star.planets.first(index))
		{
			if(orbitAround == p.number && p.orbitAround == 0) {
				planet.orbitAroundPlanet = &p;
				p.moons = std::span(&p+1,&planet+1);
				if(orbitIndex > 1)
					p.singularity |= EPlanetSingularity::MultipleSatellites;
				break;
			}
		}
	}
	double num3 = dotNet35Random.NextDouble();
	double num4 = dotNet35Random.NextDouble();
	dotNet35Random.NextDouble();
	dotNet35Random.NextDouble();
	dotNet35Random.NextDouble();
	dotNet35Random.NextDouble();
	dotNet35Random.NextDouble();
	dotNet35Random.NextDouble();
	dotNet35Random.NextDouble();
	dotNet35Random.NextDouble();
	double num13 = dotNet35Random.NextDouble();
	double num14 = dotNet35Random.NextDouble();
	dotNet35Random.NextDouble();
	double num15 = dotNet35Random.NextDouble();
	float a = Mathf.Pow(1.2f,(float)(num3 * (num4 - 0.5) * 0.5));
	float f1;
	if(orbitAround == 0)
	{
		float b = ::orbitRadius[orbitIndex] * star.orbitScaler;
		float num16 = (float)(((double)a - 1.0) / (double)Mathf.Max(1.0f,b) + 1.0);
		f1 = b * num16;
	} else
		f1 = (float)(((1600.0 * (double)orbitIndex + 200.0) * (double)Mathf.Pow(star.orbitScaler,0.3f) * (double)Mathf.Lerp(a,1.0f,0.5f) + (double)GAS_PLANET_REAL_RADIUS) / 40000.0);
	planet.orbitRadius = f1;
	if((num15 < static_cast<double>(0.04f)))
		planet.singularity |= EPlanetSingularity::LaySide;
	float planet_sunDistance = orbitAround == 0 ? planet.orbitRadius : planet.orbitAroundPlanet->orbitRadius;
	planet.maxorbitRadius = orbitAround == 0 ? planet.orbitRadius : planet.orbitRadius + planet_sunDistance;

	if(orbitAround == 0 && orbitIndex <= 4 && !gasGiant)
	{
		if((num15 > static_cast<double>(0.96f)))
			planet.singularity |= EPlanetSingularity::TidalLocked;
		else if((num15 > static_cast<double>(0.93f)))
			planet.singularity |= EPlanetSingularity::TidalLocked2;
		else if((num15 > static_cast<double>(0.9f)))
			planet.singularity |= EPlanetSingularity::TidalLocked4;
	}
	if((num15 > 0.85) && (num15 <= 0.9))
		planet.singularity |= EPlanetSingularity::ClockwiseRotate;
	float habitableRadius = star.habitableRadius;
	float planet_temperatureBias = 0.0f;
	if(gasGiant)
	{
		planet.type = EPlanetType::Gas;
	} else
	{
		float num18 = Mathf.Ceil(star.galaxy->starCount * 0.29f);
		if(((double)num18 < 11.0))
			num18 = 11.0f;
		double num19 = (double)num18 - (double)star.galaxy->habitableCount;
		float num20 = (float)(star.galaxy->starCount - star.index);
		float num21 = 1000.0f;
		float f2 = 1000.0f;
		if(((double)habitableRadius > 0.0) && ((double)planet_sunDistance > 0.0))
		{
			f2 = planet_sunDistance / habitableRadius;
			num21 = Mathf.Abs(Mathf.Log(f2));
		}
		float num22 = Mathf.Clamp(Mathf.Sqrt(habitableRadius),1.0f,2.0f) - 0.04f;
		double num23 = (double)num20;
		float habitableFraction = (float)(num19 / num23);
		float habitableLerp = Mathf.Lerp(habitableFraction,0.35f,0.5f);
		float num24 = Mathf.Clamp(habitableLerp,0.08f,0.8f);
		float planet_habitableBias = num21 * num22;
		planet_temperatureBias = (float)(static_cast<double>(1.2f) / ((double)f2 + static_cast<double>(0.2f)) - 1.0);
		float probabilityBase = Mathf.Clamp01(planet_habitableBias / num24);
		float probabilityExponent = num24 * 10.0f;
		float num25 = Mathf.Pow(probabilityBase,probabilityExponent);
		if((num13 > (double)num25) && star.index > 0 || planet.orbitAround > 0 && planet.orbitIndex == 1 && star.index == 0)
		{
			planet.type = EPlanetType::Ocean;
			++star.galaxy->habitableCount;
		} else if(((double)f2 < static_cast<double>(0.833333f)))
		{
			float num26 = Mathf.Max(0.15f,(float)((double)f2 * 2.5 - static_cast<double>(0.85f)));
			planet.type = (num14 >= (double)num26) ? EPlanetType::Vocano : EPlanetType::Desert;
		} else if(((double)f2 < static_cast<double>(1.2f)))
		{
			planet.type = EPlanetType::Desert;
		} else
		{
			float num27 = (float)(static_cast<double>(0.9f) / (double)f2 - static_cast<double>(0.1f));
			planet.type = (num14 >= (double)num27) ? EPlanetType::Ice : EPlanetType::Desert;
		}
	}
	planet.temperatureBias = planet_temperatureBias;
}

static float RandNormal(float averageValue,float standardDeviation,double r1,double r2) {
	return static_cast<float>(static_cast<double>(averageValue) + static_cast<double>(standardDeviation) * (float)(Math.Sqrt(-2.0 * Math.Log(1.0 - r1)) * Math.Sin(2.0 * Math.PI * r2)));
}

static uint8_t get_star_type_id(const EStarType type, const ESpectrType spectr) {
	switch(type) {
		case EStarType::MainSeqStar:
		if(spectr != ESpectrType::X)
			return (uint8_t)spectr;
		else
			throw std::runtime_error("Unknown star type");
		case EStarType::GiantStar:
		if(spectr != ESpectrType::X)
			return (uint8_t)spectr + 7;
		else
			throw std::runtime_error("Unknown star type");
		case EStarType::WhiteDwarf:
		return 14;
		case EStarType::NeutronStar:
		return 15;
		case EStarType::BlackHole:
		return 16;
		default:
			throw std::runtime_error("Unknown star type");
	}
}

void StarClassSimple::init(GalaxyClassSimple& parent,int index,int seed,EStarType needtype,ESpectrType needSpectr) {
	StarClassSimple& star = *this;
	star.galaxy = &parent;
	star.type = EStarType::MainSeqStar;
	star.spectr = ESpectrType::X;
	star.resourceCoef = 0.0f;
	star.position = VectorLF3::zero();
	star.uPosition = VectorLF3::zero();
	star.luminosity = star.radius = star.habitableRadius = star.orbitScaler = 1.0f;
	star.dysonRadius = 10.0f;
	star.themed_planet_count = 0;
	star.planets_initialized = false;
	star.upper_veins_point.fill(0);
	star.upper_veins_amount.fill(0);
	star.real_veins_point.fill(0);
	star.real_veins_amount.fill(0);
	star.planets = {};
	if(index == 0)
		init_birth(seed);
	else {
		star.index = index;
		float level = (float)star.index / (float)(parent.starCount - 1);
		star.id = index + 1;
		star.seed = seed;
		DotNet35Random dotNet35Random1(seed);
		int seed1 = dotNet35Random1.Next();
		int Seed = dotNet35Random1.Next();
		dotNet35Random1.Next();
		star.planet_num_seed = dotNet35Random1.Next();
		DotNet35Random dotNet35Random2(Seed);
		double r1 = dotNet35Random2.NextDouble();
		double r2 = dotNet35Random2.NextDouble();
		double num2 = dotNet35Random2.NextDouble();
		double rn = dotNet35Random2.NextDouble();
		double rt = dotNet35Random2.NextDouble();
		double num3 = (dotNet35Random2.NextDouble() - 0.5) * 0.2;
		double num4 = dotNet35Random2.NextDouble() * 0.2 + 0.9;
		double y = dotNet35Random2.NextDouble() * 0.4 - 0.2;
		double num5 = Math.Pow(2.0,y);
		float num6 = Mathf.Lerp(-0.98f,0.88f,level);
		float averageValue = ((double)num6 >= 0.0) ? num6 + 0.65f : num6 - 0.65f;
		float standardDeviation = 0.33f;
		if(needtype == EStarType::GiantStar) {
			averageValue = (y > -0.08) ? -1.5f : 1.6f;
			standardDeviation = 0.3f;
		}
		float num7 = RandNormal(averageValue,standardDeviation,r1,r2);
		switch(needSpectr)
		{
		case ESpectrType::M:
		num7 = -3.0f;
		break;
		case ESpectrType::O:
		num7 = 3.0f;
		break;
		}
		float scaledNormal = ((double)num7 <= 0.0) ? num7 * 1.0f : num7 * 2.0f;
		float p1 = (float)((double)Mathf.Clamp(scaledNormal,-2.4f,4.65f) + static_cast<double>(static_cast<float>(num3)) + 1.0);
		switch(needtype)
		{
		case EStarType::WhiteDwarf:
		star.mass = static_cast<float>(1.0 + static_cast<double>(static_cast<float>(r2)) * 5.0);
		break;
		case EStarType::NeutronStar:
		star.mass = static_cast<float>(7.0 + static_cast<double>(static_cast<float>(r1)) * 11.0);
		break;
		case EStarType::BlackHole:
		star.mass = static_cast<float>(18.0 + static_cast<double>(static_cast<float>(r1 * r2)) * 30.0);
		break;
		default:
		star.mass = Mathf.Pow(2.0f,p1);
		break;
		}
		switch(needtype)
		{
		case EStarType::GiantStar:
		star.age = (float)(static_cast<double>(static_cast<float>(num2)) * static_cast<double>(0.04f) + static_cast<double>(0.96f));
		break;
		case EStarType::WhiteDwarf:
		case EStarType::NeutronStar:
		case EStarType::BlackHole:
		star.age = (float)(static_cast<double>(static_cast<float>(num2)) * static_cast<double>(0.4f) + 1.0);
		break;
		default:
		star.age = ((double)star.mass >= 0.5)
			? (((double)star.mass >= 0.8)
				? (float)(static_cast<double>(static_cast<float>(num2)) * static_cast<double>(0.7f) + static_cast<double>(0.2f))
				: (float)(static_cast<double>(static_cast<float>(num2)) * static_cast<double>(0.4f) + static_cast<double>(0.1f)))
			: (float)(static_cast<double>(static_cast<float>(num2)) * static_cast<double>(0.12f) + static_cast<double>(0.02f));
		break;
		}
		float num9 = static_cast<float>((1.0 - (double)Mathf.Pow(Mathf.Clamp01(star.age),20.0f) * 0.5) * star.mass);
		float star_temperature = (float)(Math.Pow((double)num9,0.56 + 0.14 / (Math.Log10((double)num9 + 4.0) / Math.Log10(5.0))) * 4450.0 + 1300.0);
		double num10 = Math.Log10(((double)star_temperature - 1300.0) / 4500.0) / Math.Log10(2.6) - 0.5;
		if((num10 < 0.0))
			num10 *= 4.0;
		if((num10 > 2.0))
			num10 = 2.0;
		else if((num10 < -4.0))
			num10 = -4.0;
		star.spectr = (ESpectrType)Mathf.RoundToInt((float)num10 + 4.0f);
		star.luminosity = Mathf.Pow(num9,0.7f);
		star.radius = (float)(Math.Pow((double)star.mass,0.4) * num5);
		float p2 = (float)num10 + 2.0f;
		star.habitableRadius = Mathf.Pow(1.7f,p2) + 0.25f * 1.0f;
		star.orbitScaler = Mathf.Pow(1.35f,p2);
		if(((double)star.orbitScaler < 1.0))
			star.orbitScaler = Mathf.Lerp(star.orbitScaler,1.0f,0.6f);
		star.SetStarAge(rn,rt);
		float dysonRadius = star.orbitScaler * 0.28f;
		if(((double)dysonRadius * 40000.0 < (double)radius * 1200 * 1.5)) //TODO: 这里需要跟游戏真值对照，radius的舍入顺序
			dysonRadius = (float)((double)radius * 1200 * 1.5 / 40000.0);
		star.dysonRadius = std::round(dysonRadius * 800) * 100;
		star.luminosity = Mathf.Round((float)Math.Pow(star.luminosity,0.33000001311302185) * 1000.0f) / 1000.0f; //必须在SetStarAge之后再修正
		star.distance = 2.0f;
		}
	star.type_id = get_star_type_id(star.type, star.spectr);
	star.type_mask = uint32_t{1} << star.type_id;
}

void StarClassSimple::init_birth(int seed) {
	StarClassSimple& birthStar = *this;
	birthStar.index = 0;
	birthStar.id = 1;
	birthStar.seed = seed;
	birthStar.resourceCoef = 0.6f;
	DotNet35Random dotNet35Random1(seed);
	int seed1 = dotNet35Random1.Next();
	int Seed = dotNet35Random1.Next();
	birthStar.position = VectorLF3::zero();
	birthStar.uPosition = VectorLF3::zero();
	DotNet35Random dotNet35Random2(Seed);
	double r1 = dotNet35Random2.NextDouble();
	double r2 = dotNet35Random2.NextDouble();
	double num1 = dotNet35Random2.NextDouble();
	double rn = dotNet35Random2.NextDouble();
	double rt = dotNet35Random2.NextDouble();
	double num2 = dotNet35Random2.NextDouble() * 0.2 + 0.9;
	double num3 = Math.Pow(2.0,dotNet35Random2.NextDouble() * 0.4 - 0.2);
	float p1 = Mathf.Clamp((RandNormal(0.0f,0.08f,r1,r2)),-0.2f,0.2f);
	birthStar.mass = Mathf.Pow(2.0f,p1);
	birthStar.age = (float)(num1 * 0.4 + 0.3);
	float num4 = static_cast<float>((1.0 - (double)Mathf.Pow(Mathf.Clamp01(birthStar.age),20.0f) * 0.5) * birthStar.mass);
	float star_temperature = (float)(Math.Pow((double)num4,0.56 + 0.14 / (Math.Log10((double)num4 + 4.0) / Math.Log10(5.0))) * 4450.0 + 1300.0);
	double num5 = Math.Log10(((double)star_temperature - 1300.0) / 4500.0) / Math.Log10(2.6) - 0.5;
	if((num5 < 0.0))
		num5 *= 4.0;
	if((num5 > 2.0))
		num5 = 2.0;
	else if((num5 < -4.0))
		num5 = -4.0;
	birthStar.spectr = (ESpectrType)Mathf.RoundToInt((float)num5 + 4.0f);
	birthStar.luminosity = Mathf.Pow(num4,0.7f);
	birthStar.radius = (float)(Math.Pow((double)birthStar.mass,0.4) * num3);
	float p2 = (float)num5 + 2.0f;
	birthStar.habitableRadius = Mathf.Pow(1.7f,p2) + 0.2f;
	birthStar.orbitScaler = Mathf.Pow(1.35f,p2);
	if(((double)birthStar.orbitScaler < 1.0))
		birthStar.orbitScaler = Mathf.Lerp(birthStar.orbitScaler,1.0f,0.6f);
	birthStar.SetStarAge(rn,rt);
	float dysonRadius = birthStar.orbitScaler * 0.28f;
	if(((double)dysonRadius * 40000.0 < (double)radius * 1200 * 1.5))
		dysonRadius = (float)((double)radius * 1200 * 1.5 / 40000.0);
	birthStar.dysonRadius = std::round(dysonRadius * 800) * 100;
	birthStar.luminosity = Mathf.Round((float)Math.Pow(birthStar.luminosity,0.33000001311302185) * 1000.0f) / 1000.0f; //必须在SetStarAge之后再修正
	birthStar.distance = 0.0f;
}

void StarClassSimple::init_planets() {
	if(planets_initialized)
		return;
	// 调用者保证前序恒星已完成行星初始化，宜居数量可直接沿用星系累计值。
	DotNet35Random dotNet35Random1(this->seed);
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	dotNet35Random1.Next();
	DotNet35Random dotNet35Random2(dotNet35Random1.Next());
	double num1 = dotNet35Random2.NextDouble();
	double num2 = dotNet35Random2.NextDouble();
	double num3 = dotNet35Random2.NextDouble();
	double num4 = dotNet35Random2.NextDouble();
	double num5 = dotNet35Random2.NextDouble();
	double num6 = dotNet35Random2.NextDouble() * 0.2 + 0.9;
	double num7 = dotNet35Random2.NextDouble() * 0.2 + 0.9;
	int planet_count = planet_num;
	if(this->type == EStarType::BlackHole)
	{
		int info_seed = dotNet35Random2.Next();
		int gen_seed = dotNet35Random2.Next();
		planets[0].init(*this,0,0,3,1,false,info_seed,gen_seed);
	} else if(this->type == EStarType::NeutronStar)
	{
		int info_seed = dotNet35Random2.Next();
		int gen_seed = dotNet35Random2.Next();
		planets[0].init(*this,0,0,3,1,false,info_seed,gen_seed);
	} else if(this->type == EStarType::WhiteDwarf)
	{
		if(num1 < static_cast<double>(0.7f))
		{
			int info_seed = dotNet35Random2.Next();
			int gen_seed = dotNet35Random2.Next();
			planets[0].init(*this,0,0,3,1,false,info_seed,gen_seed);
		} else
		{
			if(num2 < static_cast<double>(0.3f))
			{
				int info_seed1 = dotNet35Random2.Next();
				int gen_seed1 = dotNet35Random2.Next();
				planets[0].init(*this,0,0,3,1,false,info_seed1,gen_seed1);
				int info_seed2 = dotNet35Random2.Next();
				int gen_seed2 = dotNet35Random2.Next();
				planets[1].init(*this,1,0,4,2,false,info_seed2,gen_seed2);
			} else
			{
				int info_seed3 = dotNet35Random2.Next();
				int gen_seed3 = dotNet35Random2.Next();
				planets[0].init(*this,0,0,4,1,true,info_seed3,gen_seed3);
				int info_seed4 = dotNet35Random2.Next();
				int gen_seed4 = dotNet35Random2.Next();
				planets[1].init(*this,1,1,1,1,false,info_seed4,gen_seed4);
			}
		}
	} else if(this->type == EStarType::GiantStar)
	{
		if(num1 < static_cast<double>(0.3f))
		{
			int info_seed = dotNet35Random2.Next();
			int gen_seed = dotNet35Random2.Next();
			planets[0].init(*this,0,0,num3 > 0.5 ? 3 : 2,1,false,info_seed,gen_seed);
		} else if(num1 < 0.800000011920929)
		{
			if(num2 < 0.25)
			{
				int info_seed5 = dotNet35Random2.Next();
				int gen_seed5 = dotNet35Random2.Next();
				planets[0].init(*this,0,0,num3 > 0.5 ? 3 : 2,1,false,info_seed5,gen_seed5);
				int info_seed6 = dotNet35Random2.Next();
				int gen_seed6 = dotNet35Random2.Next();
				planets[1].init(*this,1,0,num3 > 0.5 ? 4 : 3,2,false,info_seed6,gen_seed6);
			} else
			{
				int info_seed7 = dotNet35Random2.Next();
				int gen_seed7 = dotNet35Random2.Next();
				planets[0].init(*this,0,0,3,1,true,info_seed7,gen_seed7);
				int info_seed8 = dotNet35Random2.Next();
				int gen_seed8 = dotNet35Random2.Next();
				planets[1].init(*this,1,1,1,1,false,info_seed8,gen_seed8);
			}
		} else
		{
			if(num2 < static_cast<double>(0.15f))
			{
				int info_seed9 = dotNet35Random2.Next();
				int gen_seed9 = dotNet35Random2.Next();
				planets[0].init(*this,0,0,num3 > 0.5 ? 3 : 2,1,false,info_seed9,gen_seed9);
				int info_seed10 = dotNet35Random2.Next();
				int gen_seed10 = dotNet35Random2.Next();
				planets[1].init(*this,1,0,num3 > 0.5 ? 4 : 3,2,false,info_seed10,gen_seed10);
				int info_seed11 = dotNet35Random2.Next();
				int gen_seed11 = dotNet35Random2.Next();
				planets[2].init(*this,2,0,num3 > 0.5 ? 5 : 4,3,false,info_seed11,gen_seed11);
			} else if(num2 < 0.75)
			{
				int info_seed12 = dotNet35Random2.Next();
				int gen_seed12 = dotNet35Random2.Next();
				planets[0].init(*this,0,0,num3 > 0.5 ? 3 : 2,1,false,info_seed12,gen_seed12);
				int info_seed13 = dotNet35Random2.Next();
				int gen_seed13 = dotNet35Random2.Next();
				planets[1].init(*this,1,0,4,2,true,info_seed13,gen_seed13);
				int info_seed14 = dotNet35Random2.Next();
				int gen_seed14 = dotNet35Random2.Next();
				planets[2].init(*this,2,2,1,1,false,info_seed14,gen_seed14);
			} else
			{
				int info_seed15 = dotNet35Random2.Next();
				int gen_seed15 = dotNet35Random2.Next();
				planets[0].init(*this,0,0,num3 > 0.5 ? 4 : 3,1,true,info_seed15,gen_seed15);
				int info_seed16 = dotNet35Random2.Next();
				int gen_seed16 = dotNet35Random2.Next();
				planets[1].init(*this,1,1,1,1,false,info_seed16,gen_seed16);
				int info_seed17 = dotNet35Random2.Next();
				int gen_seed17 = dotNet35Random2.Next();
				planets[2].init(*this,2,1,2,2,false,info_seed17,gen_seed17);
			}
		}
	} else
	{
		double pGas[6];
		if(this->index == 0)
		{
			pGas[0] = 0.0;
			pGas[1] = 0.0;
			pGas[2] = 0.0;
		} else if(this->spectr == ESpectrType::M)
		{
			if(planet_count <= 3)
			{
				pGas[0] = 0.2;
				pGas[1] = 0.2;
			} else
			{
				pGas[0] = 0.0;
				pGas[1] = 0.2;
				pGas[2] = 0.3;
			}
		} else if(this->spectr == ESpectrType::K)
		{
			if(planet_count <= 3)
			{
				pGas[0] = 0.18;
				pGas[1] = 0.18;
			} else
			{
				pGas[0] = 0.0;
				pGas[1] = 0.18;
				pGas[2] = 0.28;
				pGas[3] = 0.28;
			}
		} else if(this->spectr == ESpectrType::G)
		{
			if(planet_count <= 3)
			{
				pGas[0] = 0.18;
				pGas[1] = 0.18;
			} else
			{
				pGas[0] = 0.0;
				pGas[1] = 0.2;
				pGas[2] = 0.3;
				pGas[3] = 0.3;
			}
		} else if(this->spectr == ESpectrType::F)
		{
			if(planet_count <= 3)
			{
				pGas[0] = 0.2;
				pGas[1] = 0.2;
			} else
			{
				pGas[0] = 0.0;
				pGas[1] = 0.22;
				pGas[2] = 0.31;
				pGas[3] = 0.31;
			}
		} else if(this->spectr == ESpectrType::A)
		{
			if(planet_count <= 3)
			{
				pGas[0] = 0.2;
				pGas[1] = 0.2;
			} else
			{
				pGas[0] = 0.1;
				pGas[1] = 0.28;
				pGas[2] = 0.3;
				pGas[3] = 0.35;
			}
		} else if(this->spectr == ESpectrType::B)
		{
			if(planet_count <= 3)
			{
				pGas[0] = 0.2;
				pGas[1] = 0.2;
			} else
			{
				pGas[0] = 0.1;
				pGas[1] = 0.22;
				pGas[2] = 0.28;
				pGas[3] = 0.35;
				pGas[4] = 0.35;
			}
		} else if(this->spectr == ESpectrType::O)
		{
			pGas[0] = 0.1;
			pGas[1] = 0.2;
			pGas[2] = 0.25;
			pGas[3] = 0.3;
			pGas[4] = 0.32;
			pGas[5] = 0.35;
		}
		int num8 = 0;
		int num9 = 0;
		int orbitAround = 0;
		int num10 = 1;
		for(int index = 0; index < planet_count; ++index)
		{
			int info_seed = dotNet35Random2.Next();
			int gen_seed = dotNet35Random2.Next();
			double num11 = dotNet35Random2.NextDouble();
			double num12 = dotNet35Random2.NextDouble();
			bool gasGiant = false;
			if(orbitAround == 0)
			{
				++num8;
				if(index < planet_count - 1 && (num11 < pGas[index]))
				{
					gasGiant = true;
					if(num10 < 3)
						num10 = 3;
				}
				for(; this->index != 0 || num10 != 3; ++num10)
				{
					int num13 = planet_count - index;
					int num14 = 9 - num10;
					if(num14 > num13)
					{
						float a = (float)num13 / (float)num14;
						float num15 = num10 <= 3 ? Mathf.Lerp(a,1.0f,0.15f) + 0.01f : Mathf.Lerp(a,1.0f,0.45f) + 0.01f;
						const double orbitRoll = dotNet35Random2.NextDouble(); if((orbitRoll < (double)num15))
							goto label_62;
					} else
						goto label_62;
				}
				gasGiant = true;
			} else
			{
				++num9;
				gasGiant = false;
			}
			label_62:
			planets[index].init(*this,index,orbitAround,orbitAround == 0 ? num10 : num9,orbitAround == 0 ? num8 : num9,gasGiant,info_seed,gen_seed);
			++num10;
			if(gasGiant)
			{
				orbitAround = num8;
				num9 = 0;
			}
			if(num9 >= 1 && (num12 < 0.8))
			{
				orbitAround = 0;
				num9 = 0;
			}
		}
	}
	planets_initialized = true;
	// 整颗恒星完成后才发布前缀，判断器不会访问尚未 init 的行星槽位。
	galaxy->generated_stars = this->index+1;
	galaxy->generated_planets += planet_count;
}

void StarClassSimple::set_vein_mask() {
	vein_mask = 0;
	for(const PlanetClassSimple& planet : planets) {
		vein_mask |= planet.vein_mask;
	}
}

void StarClassSimple::ensure_themes() {
	ensure_planets();
	if(themed_planet_count < planets.size())
		planets.back().ensure_theme();
}

void StarClassSimple::SetStarAge(double rn,double rt) {
	float num1 = (float)(rn * 0.1 + 0.95);
	float num2 = (float)(rt * 0.4 + 0.8);
	if(((double)this->age >= 1.0))
	{
		if(((double)this->mass >= 18.0))
		{
			this->type = EStarType::BlackHole;
			this->spectr = ESpectrType::X;
			this->mass = static_cast<float>(static_cast<double>(this->mass) * (static_cast<double>(2.5f) * num2));
			this->radius *= 1.0f;
			this->luminosity = static_cast<float>(static_cast<double>(this->luminosity) * (static_cast<double>(0.001f) * num1));
			this->habitableRadius = 0.0f;
		} else if(((double)this->mass >= 7.0))
		{
			this->type = EStarType::NeutronStar;
			this->spectr = ESpectrType::X;
			this->mass = static_cast<float>(static_cast<double>(this->mass) * (static_cast<double>(0.2f) * num1));
			this->radius *= 0.15f;
			this->luminosity = static_cast<float>(static_cast<double>(this->luminosity) * (static_cast<double>(0.1f) * num1));
			this->habitableRadius = 0.0f;
			this->orbitScaler = static_cast<float>(static_cast<double>(this->orbitScaler) * (static_cast<double>(1.5f) * num1));
		} else
		{
			this->type = EStarType::WhiteDwarf;
			this->spectr = ESpectrType::X;
			this->mass = static_cast<float>(static_cast<double>(this->mass) * (static_cast<double>(0.2f) * num1));
			this->radius *= 0.2f;
			this->luminosity = static_cast<float>(static_cast<double>(this->luminosity) * (static_cast<double>(0.04f) * num2));
			this->habitableRadius = static_cast<float>(static_cast<double>(this->habitableRadius) * (static_cast<double>(0.15f) * num2));
		}
	} else
	{
		if(((double)this->age < static_cast<double>(0.96f)))
			return;
		float num4 = (float)(Math.Pow(5.0,Math.Abs(Math.Log10((double)this->mass) - 0.7)) * 5.0);
		if(((double)num4 > 10.0))
			num4 = (float)(((double)Mathf.Log(num4 * 0.1f) + 1.0) * 10.0);
		float num5 = (float)(1.0 - (double)Mathf.Pow(this->age,30.0f) * 0.5);
		this->type = EStarType::GiantStar;
		this->mass = num5 * this->mass;
		this->radius = num4 * num2;
		this->luminosity = 1.6f * this->luminosity;
		this->habitableRadius = 9.0f * this->habitableRadius;
		this->orbitScaler = 3.3f * this->orbitScaler;
	}
}

void GalaxyClassSimple::init(const SeedStruct& input) {
	int galaxySeed = input.seed_id;
	int starNum = input.star_num;
	float resource_rate = resource_rates[input.resource_index];
	generated_stars = 0;
	generated_planets = 0;
	birthPlanetId = 0;
	planetCount = 0;
	seed = galaxySeed;
	starCount = starNum;
	resource_multiplier = resource_rate;
	is_infinite_resource = resource_multiplier >= 99.5f;
	is_rare_resource = resource_multiplier <= 0.1001f;
	is_gen_position = false;

	DotNet35Random dotNet35Random(galaxySeed);
	dotNet35Random.Next();
	stars.clear();
	planets.clear();
	stars.resize(starCount);
	habitableCount = 0;
	float num1 = (float)dotNet35Random.NextDouble();
	float num2 = (float)dotNet35Random.NextDouble();
	float num3 = (float)dotNet35Random.NextDouble();
	float num4 = (float)dotNet35Random.NextDouble();

	int num5 = Mathf.CeilToInt((float)(static_cast<double>(0.01f) * (double)starCount + (double)num1 * static_cast<double>(0.3f)));
	int num6 = Mathf.CeilToInt((float)(static_cast<double>(0.01f) * (double)starCount + (double)num2 * static_cast<double>(0.3f)));
	int num7 = Mathf.CeilToInt((float)(static_cast<double>(0.016f) * (double)starCount + (double)num3 * static_cast<double>(0.4f)));
	int num8 = Mathf.CeilToInt((float)(static_cast<double>(0.013f) * (double)starCount + (double)num4 * static_cast<double>(1.4f)));
	int num9 = starCount - num5;
	int num10 = num9 - num6;
	int num11 = num10 - num7;
	int num12 = (num11 - 1) / num8;
	int num13 = num12 / 2;
	for(int index = 0; index < starCount; ++index) {
		int seed = dotNet35Random.Next();
		if(index == 0)
			stars[index].init(*this,index,seed,EStarType::MainSeqStar,ESpectrType::X);
		else {
			ESpectrType needSpectr = ESpectrType::X;
			if(index == 3)
				needSpectr = ESpectrType::M;
			else if(index == num11 - 1)
				needSpectr = ESpectrType::O;
			EStarType needtype = EStarType::MainSeqStar;
			if(index % num12 == num13)
				needtype = EStarType::GiantStar;
			if(index >= num9)
				needtype = EStarType::BlackHole;
			else if(index >= num10)
				needtype = EStarType::NeutronStar;
			else if(index >= num11)
				needtype = EStarType::WhiteDwarf;
			stars[index].init(*this,index,seed,needtype,needSpectr);
		}
	}
}

static int get_star_planet_num(const StarClassSimple& star) {
	if(star.index == 0)
		return 4;
	DotNet35Random planet_random(star.planet_num_seed);
	double num1 = planet_random.NextDouble();
	if(star.type == EStarType::BlackHole || star.type == EStarType::NeutronStar) return 1;
	if(star.type == EStarType::WhiteDwarf) return num1 < static_cast<double>(0.7f) ? 1 : 2;
	if(star.type == EStarType::GiantStar) return num1 < static_cast<double>(0.3f) ? 1 : (num1 < 0.800000011920929 ? 2 : 3);
	switch(star.spectr) {
	case ESpectrType::M: return num1 >= 0.1 ? (num1 >= 0.3 ? (num1 >= 0.8 ? 4 : 3) : 2) : 1;
	case ESpectrType::K: return num1 >= 0.1 ? (num1 >= 0.2 ? (num1 >= 0.7 ? (num1 >= 0.95 ? 5 : 4) : 3) : 2) : 1;
	case ESpectrType::G: return num1 >= 0.4 ? (num1 >= 0.9 ? 5 : 4) : 3;
	case ESpectrType::F: return num1 >= 0.35 ? (num1 >= 0.8 ? 5 : 4) : 3;
	case ESpectrType::A: return num1 >= 0.3 ? (num1 >= 0.75 ? 5 : 4) : 3;
	case ESpectrType::B: return num1 >= 0.3 ? (num1 >= 0.75 ? 6 : 5) : 4;
	case ESpectrType::O: return num1 >= 0.5 ? 6 : 5;
	default: return 1;
	}
}

void GalaxyClassSimple::ready_for_level2_check() {
	for(StarClassSimple& star : stars) {
		star.planet_num = get_star_planet_num(star);
		planetCount += star.planet_num;
	}
	// resize 只建立空行星对象；字段和资源缓存留给行星 init 写入。
	// 先结束旧对象生命周期，扩容时不搬运可能尚未初始化的字段。
	planets.resize(planetCount);
	int offset = 0;
	for(StarClassSimple& star : stars) {
		star.planets = std::span<PlanetClassSimple>(planets.data()+offset,star.planet_num);
		offset += star.planet_num;
	}
}

static bool CheckCollision(int cur_num,double* x1,double* y1,double* z1,double x,double y,double z,double min_dist_square) {
	#ifdef __AVX2__
	const __m256d vx = _mm256_set1_pd(x);
	const __m256d vy = _mm256_set1_pd(y);
	const __m256d vz = _mm256_set1_pd(z);
	const __m256d v_thresh = _mm256_set1_pd(min_dist_square);

	for(int i = (cur_num - 1) & ~3; i >= 0; i -= 4) {
		__m256d dx = _mm256_loadu_pd(x1 + i);
		__m256d dy = _mm256_loadu_pd(y1 + i);
		__m256d dz = _mm256_loadu_pd(z1 + i);

		dx = _mm256_sub_pd(vx,dx);
		dy = _mm256_sub_pd(vy,dy);
		dz = _mm256_sub_pd(vz,dz);

		__m256d dist = _mm256_mul_pd(dx,dx);
		dist = _mm256_add_pd(dist,_mm256_mul_pd(dy,dy));
		dist = _mm256_add_pd(dist,_mm256_mul_pd(dz,dz));

		__m256d cmp = _mm256_cmp_pd(dist,v_thresh,_CMP_LT_OQ);
		if(_mm256_movemask_pd(cmp) != 0)
			return true;
	}
	#else
	for(int i=0;i<cur_num;i++) {
		double dx = x - x1[i];
		double dy = y - y1[i];
		double dz = z - z1[i];
		double dist_square = dx * dx + dy * dy + dz * dz;
		if(dist_square < min_dist_square)
			return true;
	}
	#endif
	return false;
}

void GalaxyClassSimple::ensure_star_position() {
	if(is_gen_position)
		return;
	is_gen_position = true;
	DotNet35Random dotNet35Random1(seed);
	constexpr int iterCount = 4;
	constexpr double minDist = 2.0;
	constexpr double minStepLen = 2.3;
	constexpr double maxStepLen = 3.5;
	constexpr double flatten = 0.18;
	double x1[256] = {0},y1[256] = {0},z1[256] = {0};
	double x2[8],y2[8],z2[8];
	int cur_num = 1;
	int cur_num_2 = 0;
	int maxCount = (starCount - 1) * iterCount + 1;

	double minDistSquare = minDist * minDist;
	DotNet35RandomLong dotNet35Random(dotNet35Random1.Next());
	double num1 = dotNet35Random.NextDouble();

	int num5 = (int)(num1 * 2.0 + 6.0);
	for(int index = 0; index < num5; ++index)
	{
		int num6 = 0;
		while(num6++ < 256)
		{
			double num7 = dotNet35Random.NextDouble() * 2.0 - 1.0;
			double num8 = (dotNet35Random.NextDouble() * 2.0 - 1.0) * flatten;
			double num9 = dotNet35Random.NextDouble() * 2.0 - 1.0;
			double num10 = dotNet35Random.NextDouble();
			double d = num7 * num7 + num8 * num8 + num9 * num9;
			if(d <= 1.0 && d >= 1E-08)
			{
				double num11 = Math.Sqrt(d);
				double num12 = (num10 * (maxStepLen - minStepLen) + minDist) / num11;
				double x = num7 * num12;
				double y = num8 * num12;
				double z = num9 * num12;
				if(!CheckCollision(cur_num,x1,y1,z1,x,y,z,minDistSquare))
				{
					x1[cur_num] = x;
					y1[cur_num] = y;
					z1[cur_num] = z;
					x2[cur_num_2] = x;
					y2[cur_num_2] = y;
					z2[cur_num_2] = z;
					cur_num_2++;
					cur_num++;
					break;
				}
			}
		}
	}
	int num13 = 0;
	while(num13++ < 256)
	{
		for(int index = 0; index < cur_num_2; ++index)
		{
			if(dotNet35Random.NextDouble() <= 0.7)
			{
				int num14 = 0;
				while(num14++ < 256)
				{
					double num15 = dotNet35Random.NextDouble() * 2.0 - 1.0;
					double num16 = (dotNet35Random.NextDouble() * 2.0 - 1.0) * flatten;
					double num17 = dotNet35Random.NextDouble() * 2.0 - 1.0;
					double num18 = dotNet35Random.NextDouble();
					double d = num15 * num15 + num16 * num16 + num17 * num17;
					if(d <= 1.0 && d >= 1E-08)
					{
						double num19 = Math.Sqrt(d);
						double num20 = (num18 * (maxStepLen - minStepLen) + minDist) / num19;
						double x = x2[index] + num15 * num20;
						double y = y2[index] + num16 * num20;
						double z = z2[index] + num17 * num20;
						if(!CheckCollision(cur_num,x1,y1,z1,x,y,z,minDistSquare))
						{
							x2[index] = x;
							y2[index] = y;
							z2[index] = z;
							x1[cur_num] = x;
							y1[cur_num] = y;
							z1[cur_num] = z;
							cur_num++;
							if(cur_num >= maxCount)
								goto finish_label;
							break;
						}
					}
				}
			}
		}
	}
	finish_label:

	for(int i=0;i<starCount;i++) {
		StarClassSimple& star = stars[i];
		star.position = VectorLF3(x1[i*4],y1[i*4],z1[i*4]);
		star.uPosition = star.position * 2400000.0;
		star.distance = (star.position - stars[0].position).magnitude();
		if(i == 0)
			star.resourceCoef = 0.6f;
		else {
			float num1 = (float)star.position.magnitude() / 32.0f;
			if((double)num1 > 1.0)
				num1 = Mathf.Log(Mathf.Log(Mathf.Log(Mathf.Log(Mathf.Log(num1) + 1.0f) + 1.0f) + 1.0f) + 1.0f) + 1.0f;
			star.resourceCoef = Mathf.Pow(7.0f,num1) * 0.6f;
		}
	}
}
