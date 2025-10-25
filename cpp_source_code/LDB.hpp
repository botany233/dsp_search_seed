// Original 2022 Copyright https://github.com/crazyyao0.
// Modified by https://github.com/botany233 on 2025.10
#include "defines.hpp"
#include "util.hpp"

#pragma warning(disable:4305)

struct ThemeProto
{
    std::string      Name;
    int              ID;
    std::string      DisplayName;
    EPlanetType      PlanetType;
    float            Temperature;
    EThemeDistribute Distribute;
    std::vector<int>            Algos;
    Vector2          ModX;
    Vector2          ModY;
    std::vector<int>            VeinSpot;
    std::vector<float>          VeinCount;
    std::vector<float>          VeinOpacity;
    std::vector<int>            RareVeins;
    std::vector<float>          RareSettings;
    std::vector<int>            GasItems;
    std::vector<float>          GasSpeeds;
    bool             UseHeightForBuild;
    float            Wind;
    float            IonHeight;
    float            WaterHeight;
    int              WaterItemId;
};

struct LDB_CLASS
{
    std::vector<ThemeProto> themes;
    std::vector<int> themeIds;
    LDB_CLASS()
    {
        themeIds = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
        themes.push_back({ "Ocean 1", 1, "地中海", EPlanetType::Ocean, 0, EThemeDistribute::Birth, {1}, {0, 0}, {0, 0}, {7, 5, 0, 0, 8, 11, 18}, {0.7, 0.6, 0, 0, 1, 1, 1}, {0.6, 0.5, 0, 0, 0.7, 1, 1}, {11}, {0, 1, 0.3, 0.3}, {}, {}, false, 1, 60, 0, 1000 });
        themes.push_back({ "Gas 1", 2, "气态巨星", EPlanetType::Gas, 2, EThemeDistribute::Default, {0}, {0, 0}, {0, 0}, {}, {}, {}, {}, {}, {1120, 1121}, {0.96, 0.04}, false, 0, 0, 0, 0 });
        themes.push_back({ "Gas 2", 3, "气态巨星", EPlanetType::Gas, 1, EThemeDistribute::Default, {0}, {0, 0}, {0, 0}, {}, {}, {}, {}, {}, {1120, 1121}, {0.96, 0.04}, false, 0, 0, 0, 0 });
        themes.push_back({ "Gas 3", 4, "冰巨星", EPlanetType::Gas, -1, EThemeDistribute::Default, {0}, {0, 0}, {0, 0}, {}, {}, {}, {}, {}, {1011, 1120}, {0.7, 0.3}, false, 0, 0, 0, 0 });
        themes.push_back({ "Gas 4", 5, "冰巨星", EPlanetType::Gas, -2, EThemeDistribute::Default, {0}, {0, 0}, {0, 0}, {}, {}, {}, {}, {}, {1011, 1120}, {0.7, 0.3}, false, 0, 0, 0, 0 });
        themes.push_back({ "Desert 1", 6, "干旱荒漠", EPlanetType::Desert, 2, EThemeDistribute::Default, {2}, {1, 1}, {0, 0}, {3, 10, 0, 6, 10, 1, 0}, {0.5, 1, 0, 1, 1, 0.3, 0}, {0.6, 0.6, 0, 1, 1, 0.3, 0}, {9}, {0, 0.18, 0.2, 0.3}, {}, {}, false, 1.5, 70, 0, 0 });
        themes.push_back({ "Desert 2", 7, "灰烬冻土", EPlanetType::Desert, -1, EThemeDistribute::Default, {2}, {0, 0}, {1, 1}, {7, 2, 7, 3, 8, 1, 0}, {1, 0.5, 1, 1, 0.7, 0.3, 0}, {0.6, 0.6, 1, 1, 0.5, 0.3, 0}, {8, 10}, {0.3, 0.5, 0.7, 0.5, 0, 0.3, 0.2, 0.6}, {}, {}, false, 0.4, 50, 0, 0 });
        themes.push_back({ "Ocean 2", 8, "海洋丛林", EPlanetType::Ocean, 0, EThemeDistribute::Interstellar, {1}, {0, 0}, {0, 0}, {7, 2, 12, 0, 4, 10, 22}, {0.6, 0.3, 0.9, 0, 0.8, 1, 1}, {0.6, 0.6, 0.6, 0, 0.5, 1, 1}, {11, 13}, {0, 1, 0.3, 1, 0, 0.5, 0.2, 1}, {}, {}, false, 1, 60, 0, 1000 });
        themes.push_back({ "Lava 1", 9, "熔岩", EPlanetType::Vocano, 5, EThemeDistribute::Default, {5}, {0, 0}, {0, 0}, {15, 15, 2, 9, 4, 2, 0}, {1, 1, 0.6, 1, 0.6, 0.3, 0}, {1, 1, 0.6, 1, 0.5, 0.3, 0}, {9, 10, 12}, {0, 0.2, 0.6, 0.7, 0, 0.2, 0.6, 0.7, 0, 0.1, 0.2, 0.8}, {}, {}, false, 0.7, 60, -2.5, -1 });
        themes.push_back({ "Ice 1", 10, "冰原冻土", EPlanetType::Ice, -5, EThemeDistribute::Default, {3}, {0, 0}, {1, 1}, {5, 1, 3, 10, 2, 1, 0}, {0.6, 0.2, 0.8, 1, 0.8, 0.2, 0}, {1, 0.5, 1, 1, 1, 0.3, 0}, {8, 10, 12}, {0.3, 1, 0.8, 1, 0, 0.2, 0.6, 0.4, 0, 0.1, 0.2, 0.4}, {}, {}, false, 0.7, 55, 0, 1000 });
        themes.push_back({ "Desert 3", 11, "贫瘠荒漠", EPlanetType::Desert, -2, EThemeDistribute::Default, {4}, {0, 0}, {1, 1}, {3, 3, 3, 6, 12, 0, 0}, {0.5, 0.5, 0.5, 1, 1.2, 0, 0}, {0.6, 0.6, 0.9, 0.9, 1.5, 0, 0}, {8, 9, 12}, {0.25, 0.5, 0.6, 0.6, 0, 0.2, 0.6, 0.7, 0, 0.1, 0.2, 0.5}, {}, {}, false, 0, 0, 0, 0 });
        themes.push_back({ "Desert 4", 12, "戈壁", EPlanetType::Desert, 1, EThemeDistribute::Default, {3}, {0, 0}, {1, 1}, {2, 7, 8, 0, 7, 3, 0}, {0.4, 1, 1, 0, 1, 0.7, 0}, {0.8, 1, 1, 0, 1, 0.7, 0}, {9, 10, 12}, {0, 0.25, 0.6, 0.6, 0, 0.25, 0.6, 0.6, 0, 0.1, 0.2, 0.5}, {}, {}, false, 0.8, 60, 0, 0 });
        themes.push_back({ "Volcanic 1", 13, "火山灰", EPlanetType::Vocano, 4, EThemeDistribute::Interstellar, {3}, {0, 0}, {1, 1}, {10, 10, 2, 7, 4, 1, 0}, {1, 1, 0.6, 1, 0.6, 0.3, 0}, {1, 1, 0.6, 1, 0.5, 0.3, 0}, {}, {}, {}, {}, false, 0.8, 70, 0, 1116 });
        themes.push_back({ "Ocean 3", 14, "红石", EPlanetType::Ocean, 0, EThemeDistribute::Interstellar, {1}, {0, 0}, {0, 0}, {4, 6, 0, 0, 10, 8, 12}, {0.7, 0.7, 0, 0, 1, 1, 1}, {0.5, 0.6, 0, 0, 0.8, 1, 1}, {9, 11, 13}, {0, 0.4, 0.3, 0.5, 0, 1, 0.3, 0.8, 0, 0.5, 0.2, 0.8}, {}, {}, false, 1, 60, 0, 1000 });
        themes.push_back({ "Ocean 4", 15, "草原", EPlanetType::Ocean, 0, EThemeDistribute::Interstellar, {6}, {0, 0}, {1, 1}, {7, 4, 7, 1, 2, 7, 18}, {0.7, 0.6, 0.7, 0.4, 0.5, 1, 1}, {0.6, 0.5, 0.6, 0.5, 0.7, 1, 1.2}, {11, 13}, {0, 1, 0.3, 1, 0, 0.5, 0.2, 1}, {}, {}, false, 1.1, 60, 0, 1000 });
        themes.push_back({ "Ocean 5", 16, "水世界", EPlanetType::Ocean, 0, EThemeDistribute::Interstellar, {7}, {0, 0}, {1, 1}, {0, 0, 0, 0, 0, 2, 10}, {0, 0, 0, 0, 0, 0.5, 5}, {0, 0, 0, 0, 0, 0.8, 2}, {13}, {1, 1, 1, 0.9}, {}, {}, false, 1.1, 60, 0, 1000 });
        themes.push_back({ "Desert 5", 17, "黑石盐滩", EPlanetType::Desert, 1, EThemeDistribute::Default, {2}, {0, 0}, {1, 1}, {2, 8, 9, 1, 3, 1, 0}, {1, 0.8, 0.8, 1, 0.7, 0.3, 0}, {0.6, 0.6, 1, 1, 0.5, 0.3, 0}, {9, 12}, {0, 0.7, 0.7, 0.5, 0, 0.1, 0.2, 0.7}, {}, {}, false, 1.1, 60, 0, 0 });
        themes.push_back({ "Ocean 6", 18, "樱林海", EPlanetType::Ocean, 0, EThemeDistribute::Interstellar, {1}, {0, 0}, {0, 0}, {5, 6, 8, 0, 4, 8, 22}, {0.6, 0.5, 0.8, 0, 0.8, 1, 1}, {0.6, 0.6, 0.6, 0, 0.5, 1, 1}, {11, 13}, {0, 1, 0.3, 1, 0, 0.5, 0.2, 1}, {}, {}, false, 1, 60, 0, 1000 });
        themes.push_back({ "Desert 6", 19, "飓风石林", EPlanetType::Desert, 1, EThemeDistribute::Interstellar, {8}, {1.5, 1.5}, {-5, -5}, {2, 8, 2, 7, 4, 1, 0}, {1, 1, 1, 1, 1, 0.5, 0}, {0.6, 0.8, 1, 0.8, 0.6, 0.3, 0}, {9, 10, 12}, {0, 0.25, 0.6, 0.6, 0, 0.25, 0.6, 0.6, 0, 0.4, 0.3, 0.9}, {}, {}, false, 1.6, 70, 0, 0 });
        themes.push_back({ "Desert 7", 20, "猩红冰湖", EPlanetType::Desert, -2, EThemeDistribute::Default, {9}, {6, 6}, {8, 8}, {5, 11, 1, 8, 3, 1, 0}, {0.8, 1, 0.5, 1, 0.7, 0.3, 0}, {1, 1, 1, 1, 0.5, 0.3, 0}, {8, 9, 12}, {0.25, 1, 0.6, 0.7, 0, 0.2, 0.6, 0.9, 0, 0.3, 0.4, 1}, {}, {}, false, 0.7, 55, 0, -1 });
        themes.push_back({ "Gas 5", 21, "高产气巨", EPlanetType::Gas, 1.0, EThemeDistribute::Interstellar, {0}, {0, 0}, {0, 0}, {}, {}, {}, {}, {}, {1120, 1121}, {0.84, 0.16}, false, 0, 0, 0, 0 });
        themes.push_back({ "Desert 8", 22, "热带草原", EPlanetType::Ocean, 0, EThemeDistribute::Interstellar, {10}, {1, 1}, {1, 1}, {7, 4, 7, 2, 3, 6, 14}, {0.7, 0.6, 1, 0.8, 0.7, 1, 1}, {0.7, 0.6, 0.8, 0.7, 1, 1.2, 1}, {11, 13}, {0, 1, 0.5, 1, 0, 0.6, 0.25, 1}, {}, {}, false, 0.7, 55, 0, -1 });
        themes.push_back({ "Desert 9", 23, "橙晶荒漠", EPlanetType::Desert, 0.08, EThemeDistribute::Interstellar, {11}, {1.5, 1.5}, {1, 1}, {13, 2, 0, 2, 0, 2, 0}, {1, 0.5, 0, 0.7, 0, 0.6, 0}, {1.2, 0.8, 0, 1, 0, 0.5, 0}, {11, 12}, {0, 0.7, 0.2, 0.6, 0, 1, 1, 0.84}, {}, {}, false, 0.7, 55, 0, -1 });
        themes.push_back({ "Desert 10", 24, "极寒冻土", EPlanetType::Ice, -4, EThemeDistribute::Default, {12}, {1, 1}, {1, 1}, {9, 2, 2, 6, 2, 1, 0}, {0.8, 0.5, 0.8, 1, 0.7, 0.3, 0}, {0.8, 0.8, 1.2, 1, 1, 0.3, 0}, {8, 9, 12}, {0.3, 1, 0.8, 1, 0, 1, 0.7, 1, 0, 0.4, 0.5, 0.7}, {}, {}, false, 0.7, 55, 0, -1 });
        themes.push_back({ "Desert 11", 25, "潘多拉沼泽", EPlanetType::Ocean, 0, EThemeDistribute::Interstellar, {13}, {1, 1}, {3, 3}, {8, 3, 8, 1, 3, 9, 20}, {0.7, 0.6, 1, 1, 0.6, 1, 1}, {0.7, 0.5, 1, 1, 0.7, 1.2, 1}, {10, 11, 13}, {0, 0.5, 0.3, 1, 0, 1, 0.3, 1, 0, 0.5, 0.2, 1}, {}, {}, false, 0.7, 55, 0, -1 });
    }

    ThemeProto& Select(int index)
    {
        return themes[index - 1];
    }

} LDB;
