#include "json.hpp"

#include "check_seed.hpp"

using namespace nlohmann;
using namespace std;

vector<string> check_batch(int start_seed,int end_seed,int start_star_num,int end_star_num,const json& galaxy_condition,int check_level)
{
	vector<string> result;
	for(int seed = start_seed;seed < end_seed;seed++)
	{
		for(int star_num = start_star_num;star_num<end_star_num;star_num++)
		{
			if(check_seed_level_1(seed,star_num,galaxy_condition,check_level))
				result.push_back(to_string(seed) + ", " + to_string(star_num));
		}
	}
	return result;
}

vector<string> check_precise(vector<int>& seed_vector, vector<int>& star_num_vector,const json& galaxy_condition,int check_level)
{
	vector<string> result;
	for(int i = 0; i<seed_vector.size(); i++)
	{
		int seed = seed_vector[i];
		int star_num = star_num_vector[i];
		if(check_seed_level_1(seed,star_num,galaxy_condition,check_level))
			result.push_back(to_string(seed) + ", " + to_string(star_num));
	}
	return result;
}
