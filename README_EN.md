[English](./README_EN.md) | [简体中文](./README.md)

# Dyson Sphere Program Seed Searcher & Viewer

## Searcher Features

- Detailed search conditions: Conditions are divided into four levels: galaxy, star, planet, and moon. They can be freely combined for customized filtering of veins, star/planet types, etc.
- High simulation accuracy: As far as I know, this is the only tool that implements planetary vein generation, restoring the vein count in the game almost 100%, instead of the traditional vein group count.
- High search efficiency: The seed search part is written in C++ for higher execution efficiency. The single-thread speed is more than 20 times that of [DspFindSeed](https://github.com/Xinyuell/DspFindSeed) (tested with the starter system Barren Desert condition).
- Built-in multi-threading: No need to open multiple instances.

![image](/assets/language/en_US/show_search.png)

## Viewer Features

- Built-in UI to display seed info and export to files; allows manual or batch import of seed lists. The import format is compatible with [DspFindSeed](https://github.com/Xinyuell/DspFindSeed)'s search results.
- Supports seed sorting: Allows sorting seeds by star type, planet type, vein group/vein count, etc., and displays the corresponding sort value.
- Allows customized sort values: Ready-made interfaces are provided in [Custom Sort](/GUI/seed_viewer/sort_seed/custom_sort.py), making it convenient to write Python code to evaluate seed values for sorting.

![image](/assets/language/en_US/show_check.png)

## Usage Instructions

### General
- Standard Mode / Fast Mode: In Standard Mode, the tool ensures the calculated vein group/vein count is as consistent with the game as possible, representing the highest accuracy of this tool. However, fully generating veins has a huge performance overhead, hence the introduction of Fast Mode. In Fast Mode, the vein groups and veins used are theoretical maximums, and the actual generated count is about `0.8` times this value. Fast Mode has a similar performance overhead for each seed.

### Searcher
- Search Conditions: Searcher conditions are divided into four levels: galaxy, star, planet, and moon. The galaxy level can directly add planet sub-conditions. Checking a condition only controls whether it takes effect itself, and does not chain to its sub-conditions.
- Range Search / Secondary Search: Range Search iterates based on the set seed ID and star count range. Secondary Search iterates based on an imported CSV file. Secondary Search automatically deduplicates the imported seeds.
- Condition Optimization: Besides the Fast/Standard Mode setting, search conditions also significantly affect search speed. For specific reasons and related data, please refer to the in-app tutorial interface.
- Storage Format: Saves as `.csv` files. Each line represents one seed: `Seed ID, Star Count`, compatible with [DspFindSeed](https://github.com/Xinyuell/DspFindSeed).

### Viewer
- Seed List: Maximum import limit is `100,000` seeds, with deduplication.
- Refresh Delay: Completely acquiring info for one seed takes about `3~10` seconds in CPU mode, and usually `<1` second with GPU acceleration enabled. The program caches info for the last `100` clicked seeds. You can quickly click multiple seeds to utilize multi-threading for accelerated generation.
- Seed Info Export: You can select seeds in the list, right-click, and batch export seed info. Each seed corresponds to a `.csv` file.
- Special Conditions: When sorting, high-yield gas giants are considered gas giants, but gas giants are not considered high-yield gas giants.

### Settings
#### Basic Settings
- Max CPU Threads: Can be set up to 128 threads, but the actual number of threads used will not exceed the CPU core count.
- GPU Acceleration: This program relies on OpenCL to accelerate planet terrain generation. Some GPUs may not support this. In this case, please disable this option, and the algorithm will fall back to CPU execution.
#### GPU Settings
- Work Group Size: Recommended to set to the maximum value supported by the GPU to improve performance. Since we cannot directly obtain whether the GPU supports this size, if normal searching/sorting fails, please try decreasing this value.
- GPU Device: Currently, the program only supports using at most one GPU. It is recommended to use the highest-performing GPU to improve performance.
- Max GPU Threads: Manages the maximum number of threads using GPU acceleration simultaneously. The optimal value can be found through performance testing.

## Supported Search Conditions
- Galaxy Level:
  - Minimum count of 14 vein types
  - Minimum yield of 14 vein types
- Star Level:
  - Star Type: Multi-select allowed
  - Minimum Luminosity
  - Maximum Distance: Input 0 for the starter star system
  - Match Count: Number of stars/star systems that need to meet the condition
  - Minimum count of 14 vein types: Unipolar Magnets only appear on Neutron Stars and Black Holes
  - Minimum yield of 14 vein types
- Planet Level:
  - Planet Type: Multi-select allowed. The starter planet is always and exclusively Mediterranean
  - Planet Features: i.e., planet attributes, multi-select allowed. The "Multiple Satellites" attribute only appears on gas planets (Gas Giants, Ice Giants, High-Yield Gas Giants), and satellite attributes will not appear on gas planets.
  - Liquid
  - Fully Enclosed: Global ray receivers can achieve 100% continuous reception without lenses (planet is entirely within the Dyson Sphere radius). A star system can have at most one such planet.
  - Full Reception: Global ray receivers can achieve 100% continuous reception with lenses (planet is inside the ionosphere radius constraint). A star system can have at most two such planets.
  - Match Count: Number of planets that need to meet the condition
  - Minimum count of 14 vein types
  - Minimum yield of 14 vein types
- Bond Condition
  - Contains two independent star/planet-level sub-conditions, designed to search for combinations of two celestial bodies within a specified distance.

## Special Thanks

The following repositories provided varying degrees of help during the development of this project, thanks!

- [dspseedsearch](https://github.com/crazyyao0/dspseedsearch)
- [DspFindSeed](https://github.com/Xinyuell/DspFindSeed)
- [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets)
- [AhabAssistantLimbusCompany](https://github.com/KIYI671/AhabAssistantLimbusCompany)