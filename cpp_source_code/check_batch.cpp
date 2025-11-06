#include "json.hpp"

#include "check_seed.hpp"

using namespace nlohmann;
using namespace std;

vector<string> check_batch(int start_seed,int end_seed,int start_star_num,int end_star_num,json& galaxy_condition,json& galaxy_condition_simple,bool check_no_veins)
{
	vector<string> result;
	for(int seed = start_seed;seed < end_seed;seed++)
	{
		for(int star_num = start_star_num;star_num<end_star_num;star_num++)
		{
			if(check_seed(seed,star_num,galaxy_condition,galaxy_condition_simple,check_no_veins))
				result.push_back(to_string(seed) + ", " + to_string(star_num));
		}
	}
	return result;
}

vector<string> check_precise(vector<int>& seed_vector, vector<int>& star_num_vector,json& galaxy_condition,json& galaxy_condition_simple,bool check_no_veins)
{
	vector<string> result;
	for(int i = 0; i<seed_vector.size(); i++)
	{
		int seed = seed_vector[i];
		int star_num = star_num_vector[i];
		if(check_seed(seed,star_num,galaxy_condition,galaxy_condition_simple,check_no_veins))
			result.push_back(to_string(seed) + ", " + to_string(star_num));
	}
	return result;
}