# Dyson Sphere Program Seed Searcher & Viewer Guide
In Dyson Sphere Program, each save's galaxy map is uniquely determined by the seed ID, star count, and resource rate. The resource rate only affects the amount in each vein; it does not affect vein locations or vein counts. Because resource distribution varies enormously between maps, the experience can range from a perfect starting system with a gas giant, two moons, and two rare resources to a cyber prison with only 30 unipolar magnets across the galaxy, no Aquatica, and no Sakura Ocean.

This is why seed searchers exist: they restore the galaxy-generation portion of the game's algorithm to find seeds that meet specific conditions. After multiple versions of updates, this seed searcher is currently the only known tool that perfectly reproduces both vein counts and vein amounts. It also includes built-in features for viewing and exporting seed information.

This project is fully open source. Stars are welcome.

Project repository: [dsp_search_seed](https://github.com/botany233/dsp_search_seed)

## Seed Search Tutorial
### UI
Click the magnifying-glass icon on the left to enter the seed search page. This page is mainly divided into three sections: search range settings, search condition settings, and extra settings. The search range includes both the left and right boundaries.
![image](tutorial_search.png)

### Search Condition Types
Each Dyson Sphere Program save is a galaxy. By celestial hierarchy, it can be divided into three levels, galaxy-star-planet, or four levels, galaxy-star-planet-moon.

Similarly, this program divides search conditions into three types: galaxy, star, and planet. A planet condition can add itself as a child condition, corresponding to the four-level celestial hierarchy. The supported search content and notes for the three condition types are listed below:

Galaxy conditions

- Minimum counts for 14 vein types
- Minimum amounts for 14 vein types

Star conditions

- Star type: multiple selection supported
- Minimum luminosity
- Max distance: enter 0 for the initial star system
- Required count: the number of star systems that must satisfy this condition
- Minimum counts for 14 vein types: unipolar magnets only appear around neutron stars and black holes
- Minimum amounts for 14 vein types

Planet conditions

- Planet type: multiple selection supported. Only the initial planet can be Mediterranean.
- Planet traits: also called planet entries; multiple selection supported
- Liquid type
- Dyson Reception: Full Cover, where the whole planet can receive without lenses; Full Receive, where the whole planet can receive with lenses. A galaxy can have at most one Full Cover Planet or two Full Receive Planets.
- Required count: the number of planets that must satisfy this condition
- Minimum counts for 14 vein types
- Minimum amounts for 14 vein types

There is also a special condition type used to check whether the current seed contains enough paired celestial bodies. A Bond Condition has two independent child conditions, which can be star or planet conditions. Each child condition can set a maximum connection count to limit how many celestial bodies from the other group a single celestial body can pair with at most. During search, the searcher first filters two groups of celestial bodies based on the child conditions, then filters all possible pair connections by distance requirements, and finally filters the pair connections by the maximum connection counts of the two celestial body groups. Note that for planet conditions, coordinate information is approximated using the star's coordinates.

### Search Condition Structure
When searching seeds, the searcher starts from the galaxy condition and checks whether the current seed satisfies itself and all of its child conditions. When multiple child conditions exist, the searcher checks them one by one. Child conditions are independent of each other, so the same celestial body can satisfy multiple child conditions. The parent condition is considered satisfied only when all child conditions are satisfied, meaning child conditions are combined with an AND relationship.

The searcher also allows planet conditions to be added directly as child conditions of the galaxy. All possible condition structures are shown below:

```
Galaxy Condition
|-- Star Condition
|   |-- Planet Condition
|   `-- Planet Condition
|       `-- Moon Condition
|-- Planet Condition
|-- Planet Condition
|   `-- Moon Condition
`-- Bond Condition
```

Click the button before a condition name to choose whether to enable the current condition. This does not cascade to its child conditions.

### Saving Search Results
Results are stored as `.csv` files and can be viewed with Excel or WPS. Each row is a seed, including seed ID and star count but not resource rate, and is compatible with [DspFindSeed](https://github.com/Xinyuell/DspFindSeed). The file name can be configured at the bottom. The save location is the program's root directory.

### Range Search / Secondary Search
Range Search mode determines the search range by setting the starting seed ID, ending seed ID, and star count. Secondary Search mode determines the search range by importing a previously saved `.csv` seed list. Secondary Search includes deduplication.

Click the first button in Extra Settings to switch between the two search modes.

### Standard Mode / Quick Mode
In Standard Mode, the calculated vein counts and amounts are kept as consistent with the game as possible, representing this program's highest accuracy. However, fully generating veins has a substantial performance cost, so Quick Mode is introduced. In Quick Mode, the calculated vein counts and amounts are theoretical maximum values. The actual vein count averages 78% of that value, and the actual vein amount averages 72% of that value. Quick Mode has roughly consistent performance cost for each seed, while Standard Mode varies greatly.

Click the second button in Extra Settings to switch between the two search accuracy modes.

## Seed Viewer Tutorial
### UI
Click the eye icon on the left to enter the seed viewer page. This page is mainly divided into five sections: resource rate selection, seed list, celestial information display, celestial information details, and seed sorting.
![image](tutorial_check.png)

### Resource Rate Selection
This setting affects the resource rate used for generating seed information and sorting seeds in the viewer.

### Seed List
The viewer itself was not designed for extremely large seed volumes, so it has a limit of 100,000 seeds. Click any seed to switch the content shown in the celestial information display. Hold the left mouse button and drag to select multiple seeds for batch deletion or seed information export. Press Ctrl + A to select all seeds.

Depending on the number of threads, fully obtaining one seed's information with the CPU takes about 1 to 3 seconds. After enabling GPU acceleration, it usually takes less than 1 second. The program caches information for the 100 most recently clicked seeds.

In addition to seed ID and star count, a sort value is provided to evaluate seed value. When exporting a seed list, the sort value is exported as well.

### Celestial Information Display & Details
Click any celestial body in the celestial information display to view its details. Note that after switching the resource rate, the UI will not refresh automatically; you need to click again.

### Seed Sorting
The viewer includes four common seed sorting methods: Vein Count, Vein Amount, Planet Type, and Star Type. It also supports customization with Python code. See [README.md](../README.md) for the detailed tutorial.

The first button switches between ascending and descending sort order. The second button switches between Quick Mode and Standard Mode. Unlike the searcher, the viewer's Standard Mode is not performance-optimized. Sorting requires fully generating all information for every seed. On CPU, it is about 3,000 times slower than Quick Mode, so use it carefully.

Note that High-Yield Gas Giants are treated as Gas Giants when sorting by planet type.

### Exporting Seed Information
After selecting one or more seeds in the seed list, right-click and choose to export seed information. After confirming the content to export, click the export button below and choose an export folder. Each seed generates one **.csv** file, so when exporting many seeds, it is recommended to create a new folder<s> unless you want your desktop buried in files</s>.
![image](tutorial_export_sample.png)

## Settings Tutorial
### UI
Click the gear icon on the left to enter the settings page. This page is divided into two sections: Basic Settings and GPU Settings.
![image](tutorial_setting.png)

### Basic Settings
Max CPU Threads: controls the maximum number of threads that the searcher and viewer can create at the same time. It can be set up to 128 threads, but the number of threads actually used will not exceed the CPU's logical processor count. The default value is the same as the CPU's logical processor count. Note that with this setting, the UI may freeze during search and sorting. If you care about UI smoothness, reduce this value by 1.

Enable GPU Acceleration: the viewer must generate planet terrain when calculating buildable areas. This process is extremely time-consuming, but GPU acceleration can be used. After enabling it, the delay when refreshing seed information and the time required for sorting in the viewer can be significantly reduced. This program's GPU acceleration depends on OpenCL 3.0, and some ancient GPUs may not support it. Some GPUs do not support double precision, mainly Intel integrated graphics before the Ultra series. In that case, single precision will be used for calculation, but the generated mineral information will differ slightly from double precision. About 10% of seeds will have anomalies on 1 to 2 planets.

### GPU Settings
Work Group Size: this value has a negligible effect on GPU acceleration performance. Different GPUs support different maximum values. If search or sorting does not work properly, try reducing this value.

GPU Device: currently, the application only supports using at most one GPU. It is recommended to use the highest-performance GPU to improve performance.

Max GPU Threads: manages the maximum number of threads that use GPU acceleration at the same time. You can find the best value through performance testing. A value from 4 to 8 is recommended for integrated GPUs. Dedicated GPU performance varies significantly, so try a value from 8 to 16.

GPU Benchmark: tests how different GPU thread counts affect terrain generation speed under the specified CPU thread count. The default test time for each thread is 1 second, and it can be adjusted as needed.

## Searcher Performance
The searcher no longer uses GPU computation. The performance table is shown below. All results are for 64-star galaxies:

<table style="width:100%; border-collapse: collapse;">
  <tr class="table-header">
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Performance Table (seed/s)</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Level 1</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Level 2</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Level 3 - Quick</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Level 3 - Standard</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Level 4 - Standard</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Better Starting System - Standard</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">All Rares Hard Flight - Standard</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">All Rares Magnet - Standard</th>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">CPU(Ultra 7 155H)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">156977</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">55584</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">51760</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">15786</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">76.52</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">145663</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">144623</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">55583</td>
  </tr>
</table>

- Level 1: Blue Giant x3
- Level 2: Aquatica x6 + Gas Giant with at least 3 satellites x2
- Level 3: Oil x250 + Unipolar Magnet veins x250
- Level 4: Iron veins x30000
- Better Starting System: the initial star system has a Gas Giant, one of its satellites is Mediterranean, and another satellite is Desolus with Stalagmite Crystal and Fire Ice
- All Rares Hard Flight: galaxy Unipolar Magnet veins x80 + Stalagmite Crystal veins x400; within max distance 5, an O Type Star system has a Gas Giant, Tidal Locking, a Full Cover Planet, Water and Sulfuric Acid, and all rare resources except Unipolar Magnet
- All Rares Magnet: galaxy Unipolar Magnet veins x120; an O Type Star system has a High-Yield Gas Giant, a Full Receive Planet, Water and Sulfuric Acid, and all rare resources except Unipolar Magnet, while the nearest Unipolar Magnet planet is within 12 light-years

## Viewer Performance
The viewer sorting performance table is shown below. All results are for 64-star galaxies. Seed information export speed is similar to Standard Mode:

<table style="width:100%; border-collapse: collapse;">
  <tr class="table-header">
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Performance Table (seed/s)</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Quick Mode</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">Standard Mode</th>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">CPU(Ultra 7 155H)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">9031</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">3.77</td>
  </tr>
  <tr class="zebra-row">
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">Integrated GPU (Arc 128EU)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">9031</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">6.21</td>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">Dedicated GPU (RX 9070)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">9031</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">28.85</td>
  </tr>
</table>
