# 戴森球计划种子搜索查看器使用说明

## 通用
- 矿簇/矿脉：矿簇对应游戏中集中的一片矿脉
- 特殊条件：在种子搜索和查看器排序时，高产气巨可被视为气态巨星，但气态巨星不会被视为高产气巨
- 标准模式/快速模式：标准模式下，会确保计算的矿簇/矿脉数量与游戏中尽可能一致，代表本工具的最高精度。但完整生成矿脉的性能开销极大，因此引入快速模式。在该模式中，使用的矿簇和矿脉为理论生成的最大值，实际生成数量大约为该值的**0.8**倍。快速模式对每个种子的性能开销基本一致

## 设置
### 基础设置
- 线程数：管理搜索器和查看器最多同时调用的线程数量。最高可设置为128线程，但实际的线程数不会超过CPU核心数
- GPU加速：本程序依赖OpenCL加速星球地形生成，部分GPU可能不支持，此时请关闭该选项，算法将回退至CPU执行
### GPU设置
- 工作组大小：建议设置为GPU支持的最大值以提高性能。由于无法直接获取GPU是否支持该大小，在无法正常搜索/排序时请尝试减小该值
- GPU设备：目前程序只支持调用最多一块GPU，推荐使用性能最高的GPU以提高性能

## 搜索器
- 搜索条件：搜索器条件分为星系、恒星、星球三级，星系条件可添加恒星/星球条件作为子条件，恒星条件可添加星球条件作为子条件。条件需满足自身及其所有子条件才被判定为满足，满足星系条件的种子将判定为合格并被记录。条件的勾选仅控制自身是否生效，不连锁至其子条件
- 范围搜素/二次搜素：范围搜索根据设定搜索的种子id和恒星数范围进行遍历，二次搜索根据导入的csv文件进行遍历。二次搜索会自动对导入的种子去重
- 条件优化：对于未设置的条件，搜索时会自动跳过以提高性能，建议不要设置任何多余条件
- 存储格式：为**.csv**文件，每行为一个种子，"**种子id, 恒星数量**"，与[DspFindSeed](https://github.com/Xinyuell/DspFindSeed)兼容
- 标准/快速模式性能：见下表，恒星数均为64

<!-- | 性能表格(seed/s) | 3蓝巨-快速模式 | 3蓝巨-标准模式 | 250油250磁石-快速模式 | 250油250磁石-标准模式 | 20000铁-快速模式 | 20000铁-标准模式 |
| :----: | :----: | :----: | :----: | :----: | :----: | :----: |
| CPU(Ultra 7 155H) | 0 | 0 | 0 | 0 | 0 | 0 |
| 核显(Arc 128EU) | 0 | 0 | 0 | 0 | 0 | 0 |
| 独显(RX 9070) | 0 | 0 | 0 | 0 | 0 | 0 | -->

<table style="width:100%; border-collapse: collapse;">
  <tr class="table-header">
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">性能表格(seed/s)</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">3蓝巨-快速模式</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">3蓝巨-标准模式</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">250油250磁石-快速模式</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">250油250磁石-标准模式</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">30000铁-快速模式</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">30000铁-标准模式</th>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">CPU(Ultra 7 155H)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">37736</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">37736</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">27027</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">345</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">27778</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">1.12</td>
  </tr>
  <tr class="zebra-row">
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">核显(Arc 128EU)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">37736</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">37736</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">27027</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">467</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">27778</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">1.56</td>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">独显(RX 9070)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">37736</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">37736</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">27027</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">1266</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">27778</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">3.85</td>
  </tr>
</table>

![image](show_search.png)

## 查看器
- 种子列表：导入种子上限为10万个，有去重功能
- 刷新延时：CPU模式完整获取一个种子信息大约需要`3~10`秒，启用GPU加速后一般`<1`秒。程序会缓存最近100个点击的种子信息，因此重复浏览一个种子时的刷新延时会显著降低
- 标准/快速模式性能：见下表，恒星数均为64

<table style="width:100%; border-collapse: collapse;">
  <tr class="table-header">
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">性能表格(seed/s)</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">快速模式</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">标准模式</th>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">CPU(Ultra 7 155H)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">8159</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">1.13</td>
  </tr>
  <tr class="zebra-row">
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">核显(Arc 128EU)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">8159</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">1.53</td>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">独显(RX 9070)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">8159</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">3.87</td>
  </tr>
</table>

![image](show_check.png)

## 支持的搜索条件

星系级：

  - 14种矿簇最低数量：一个种子最高26簇单极磁石
  - 14种矿脉最低数量

恒星级：

  - 恒星类型
  - 最低光照
  - 最远距离：输入0表示初始恒星系
  - 符合数量：需要满足该条件的恒星/恒星系数量
  - 14种矿簇最低数量：单极磁石仅在中子星和黑洞出现
  - 14种矿脉最低数量

星球级：

  - 星球类型：有且仅有初始星球为地中海类型
  - 星球特点：即星球词条，其中多卫星词条只出现在气态行星（气态巨星、冰巨星、高产气巨），卫星词条不会出现在气态行星
  - 液体
  - 全包：不使用透镜时全球可接收，一个星系最多只有一个全包星
  - 全接收：使用透镜时全球可接收，一个星系最多只有两个全接收星
  - 符合数量：需要满足该条件的行星数量
  - 14种矿簇最低数量
  - 14种矿脉最低数量

## 搜索条件优化的额外说明
&nbsp;&nbsp;&nbsp;&nbsp;本模块主要通过解释搜索器的种子判定机制来说明标准模式下不同条件的搜索时间为何差异极大，并给出优化搜索条件的相关思路。

&nbsp;&nbsp;&nbsp;&nbsp;判定一个种子是否满足条件可分为星系生成和条件判断两步，其中生成星球矿物（无论是生成上限还是精确生成）占据了大量的性能。但在进行条件判断时，不是所有的星系信息都会被使用，大部分种子都因为矿物外的条件被剔除。因此，将星系生成拆分为多步，同时进行多次条件判断能够极大的提高搜索性能。

&nbsp;&nbsp;&nbsp;&nbsp;目前，星系生成被拆分为生成恒星/星球信息、生成星球矿物上限、生成精确矿物三步，每一次生成后，都会进行一次条件判定，其中第一次判定时因为没有矿物信息将忽略矿簇/矿脉条件。同时，若搜索条件无矿簇/矿脉要求，将只进行第一轮生成和判定。而在快速模式下，不会进行第三轮生成与判定。

&nbsp;&nbsp;&nbsp;&nbsp;标准模式中，第三步为每个星球生成精确矿物极其耗时。除了使用GPU加速外，还可以只生成必要星球的精确矿物。即如果该星球的矿物数量不会影响到判断结果，则不进行生成。

&nbsp;&nbsp;&nbsp;&nbsp;具体的，每个含矿脉/矿簇的条件会生成矿物掩码对星球进行筛选，若该星球中含有掩码对应的矿物，则会被标记。在条件通过后，将不会继续生成其它标记的星球的矿物信息。同时，对于包含多种矿簇/矿脉类型的条件，将会使用动态矿物掩码，已经满足的矿物类型将会从掩码中移除。

&nbsp;&nbsp;&nbsp;&nbsp;其中，星系级矿物条件通常会标记最多的星球，对性能影响也最明显。若在标准模式下的星系条件对常见矿物进行要求并且数值较大（比如要求星系包含3万铁矿脉），则几乎每个星球都会生成精确矿物，性能开销接近无任何优化时。

&nbsp;&nbsp;&nbsp;&nbsp;因此，做出以下建议：

- 星系条件中尽量只包含单极磁石的矿物条件，最多再包含**1~2**种稀有资源
- 减少使用矿物条件判定的恒星/星球数量，可以通过增加其它必要条件来减少需要生成精确矿物的范围
