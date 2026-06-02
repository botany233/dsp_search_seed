# Dyson Sphere Program Seed Searcher & Viewer Tutorial

In Dyson Sphere Program, each save's galaxy map is uniquely determined by the seed ID, star count, and resource multiplier. The resource multiplier only affects resource amounts, not vein positions or vein counts.

This tool restores the galaxy generation algorithm and searches for seeds that meet specific conditions. It also includes seed viewing, sorting, and seed information export features.

Project repository: [dsp_search_seed](https://github.com/botany233/dsp_search_seed)

## Seed Search Tutorial

### UI Overview

Click the magnifier icon on the left to open the seed search page. This page contains search range settings, search condition settings, and extra settings.

![image](tutorial_search.png)

### Search Condition Types

Every save is a galaxy. Its celestial hierarchy can be represented as galaxy-star-planet, or galaxy-star-planet-moon.

The program provides galaxy, star, and planet conditions. A planet condition can add another planet condition as a moon condition. The available condition types are:

Galaxy condition

- Minimum count for 14 vein types
- Minimum amount for 14 vein types

Star condition

- Star type: multi-select
- Minimum luminosity
- Maximum distance: enter 0 for the starter star system
- Required count: how many star systems must satisfy this condition
- Minimum count for 14 vein types. Unipolar Magnets only appear around Neutron Stars and Black Holes
- Minimum amount for 14 vein types

Planet condition

- Planet type: multi-select. The starter planet is always Mediterranean
- Planet traits: multi-select
- Liquid type
- Dyson reception: Fully Covered means global ray receivers can work continuously without lenses. Full Receiver means global ray receivers can work continuously with lenses. A star system can have at most one Fully Covered planet or two Full Receiver planets
- Required count: how many planets must satisfy this condition
- Minimum count for 14 vein types
- Minimum amount for 14 vein types

### Search Condition Structure

When searching seeds, the searcher starts from the galaxy condition and checks whether the current seed satisfies the condition and all of its child conditions. Multiple child conditions are independent from each other, and the same celestial body can satisfy multiple child conditions. A parent condition passes only when all of its child conditions pass.

Possible structures are:

```
Galaxy Condition
┣ Star Condition
┃ ┣ Planet Condition
┃ ┗ Planet Condition
┃   ┗ Moon Condition
┣ Planet Condition
┗ Planet Condition
  ┗ Moon Condition
```

Click the checkbox before a condition name to enable or disable that condition. This does not automatically enable or disable its child conditions.

### Saving Search Results

Search results are saved as `.csv` files in the program root directory. Each row contains a seed ID and star count. The format is compatible with [DspFindSeed](https://github.com/Xinyuell/DspFindSeed).

### Range Search / Precise Search

Range Search iterates through the configured seed ID range and star count range. Precise Search imports a previously saved `.csv` seed list and searches only those seeds. Precise Search deduplicates imported seeds.

Use the first switch in the extra settings area to switch between the two search modes.

### Standard Mode / Fast Mode

Standard Mode tries to keep calculated vein counts and amounts as close to the game as possible. Full vein generation is expensive, so Fast Mode uses theoretical maximum values instead. In Fast Mode, actual vein counts average about 78% of the theoretical value, and actual vein amounts average about 72%.

Use the second switch in the extra settings area to switch between the two accuracy modes.

## Seed Viewer Tutorial

### UI Overview

Click the eye icon on the left to open the seed viewer page. This page contains resource multiplier selection, seed list, celestial tree, detail panel, and sorting controls.

![image](tutorial_check.png)

### Resource Multiplier

This setting affects seed information generation and seed sorting in the viewer.

### Seed List

The viewer supports up to 100,000 imported seeds. Click a seed to load and display its celestial information. Hold the left mouse button and drag to select multiple seeds for batch deletion or seed information export.

Getting full information for one seed takes about 3 to 10 seconds on CPU, and usually less than 1 second with GPU acceleration enabled. The program caches the most recent 100 clicked seeds.

### Celestial Tree And Details

Click any celestial body in the tree to view details. After changing the resource multiplier, click the seed again to refresh the displayed information.

### Seed Sorting

The viewer supports common sort types including vein count, vein amount, planet type, and star type. Custom Python-based sorting is also supported.

The first switch changes ascending or descending order. The second switch changes Fast Mode or Standard Mode. Viewer Standard Mode generates full information for every seed and can be much slower than Fast Mode.

When sorting by planet type, High-Yield Gas Giants are treated as Gas Giants.

### Seed Information Export

Select one or more seeds in the seed list, right-click, and choose export. Select the content to export, click Export, and then choose a destination folder. Each seed creates one `.csv` file.

![image](tutorial_export_sample.png)

## Settings Tutorial

### UI Overview

Click the gear icon on the left to open settings. The page contains Basic Settings and GPU Settings.

![image](tutorial_setting.png)

### Basic Settings

Max CPU Threads controls how many worker threads the searcher and viewer can create. The maximum configurable value is 128, but actual usage will not exceed the CPU logical processor count. If the UI freezes during search or sorting, set this value lower than the CPU thread count.

Enable GPU Acceleration uses OpenCL to accelerate planet terrain generation. Some old GPUs may not support it. Some GPUs do not support double-precision calculation; in that case, part of the calculation falls back to CPU or uses single precision with small differences.

### GPU Settings

Work Group Size can slightly improve GPU performance. The maximum supported value depends on the GPU. If search or sorting fails, try a smaller value.

GPU Device selects the GPU used by the app. The app currently supports at most one GPU.

Max GPU Threads controls how many threads can use GPU acceleration at the same time. The benchmark can help find a suitable value, usually between 2 and 6.

GPU Benchmark tests terrain generation speed under different GPU thread counts for the configured CPU thread count.

## Search Performance Notes

To improve search speed, galaxy generation is split into multiple levels. After each level, the program checks conditions that can already be evaluated. If a condition depends on information that has not been generated yet, it is deferred.

Fast Mode stops before precise vein generation. Standard Mode performs precise generation, which is much more expensive. If a planet cannot affect the final condition result, precise generation for that planet is skipped.

Galaxy-level vein conditions often have the largest performance impact. Avoid requiring large amounts of common resources at galaxy level unless necessary. It is usually better to filter by Unipolar Magnets or one or two rare resources.
