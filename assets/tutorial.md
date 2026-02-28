# 戴森球计划种子搜索查看器使用说明

## 通用
- 矿簇/矿脉：矿簇对应游戏中集中的一片矿脉
- 特殊条件：在种子搜索和查看器排序时，高产气巨可被视为气态巨星，但气态巨星不会被视为高产气巨
- 标准模式/快速模式：标准模式下，会确保计算的矿簇/矿脉数量与游戏中尽可能一致，代表本工具的最高精度。但完整生成矿脉的性能开销极大，因此引入快速模式。在该模式中，使用的矿簇和矿脉为理论生成的最大值，实际生成数量大约为该值的**0.8**倍。快速模式对每个种子的性能开销基本一致

## 设置
### 基础设置
- 最大CPU线程数：最高可设置为128线程，但实际调用的线程数不会超过CPU核心数
- GPU加速：本程序依赖OpenCL加速星球地形生成，部分GPU可能不支持，此时请关闭该选项，算法将回退至CPU执行
### GPU设置
- 工作组大小：建议设置为GPU支持的最大值以提高性能。由于无法直接获取GPU是否支持该大小，在无法正常搜索/排序时请尝试减小该值
- GPU设备：目前程序只支持调用最多一块GPU，推荐使用性能最高的GPU以提高性能
- 最大GPU线程数：管理最多同时使用GPU加速的线程数，可以通过性能测试找到最优值

## 搜索器
- 搜索条件：搜索器条件分为星系、恒星、行星、卫星四级，其中星系可直接添加行星子条件。条件的勾选仅控制自身是否生效，不连锁至其子条件
- 范围搜素/二次搜素：范围搜索根据设定搜索的种子id和恒星数范围进行遍历，二次搜索根据导入的csv文件进行遍历。二次搜索会自动对导入的种子去重
- 条件优化：除快速模式/标准模式的设置外，搜索条件也会对搜索速度产生显著影响，具体原因和相关数据见后。
- 存储格式：为**.csv**文件，每行为一个种子，"**种子id, 恒星数量**"，与[DspFindSeed](https://github.com/Xinyuell/DspFindSeed)兼容

![image](show_search.png)

## 查看器
- 种子列表：导入种子上限为10万个，有去重功能
- 刷新延时：CPU模式完整获取一个种子信息大约需要**3~10**秒，启用GPU加速后一般**<1**秒。程序会缓存最近100个点击的种子信息，可以快速点击多个种子以利用多线程加速生成
- 种子信息导出：可在种子列表处框选种子后右键批量导出种子信息，每个种子对应一个**.csv**文件

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

行星级/卫星级：

  - 星球类型：有且仅有初始星球为地中海类型
  - 星球特点：即星球词条，卫星必出现卫星词条
  - 液体
  - 全包：不使用透镜时全球可暖锅，一个星系最多只有一个全包星
  - 全接收：使用透镜时全球可暖锅，一个星系最多只有两个全接收星
  - 符合数量：需要满足该条件的行星数量
  - 14种矿簇最低数量
  - 14种矿脉最低数量

## 搜索器优化机制说明
&nbsp;&nbsp;&nbsp;&nbsp;判定一个种子是否满足条件可分为星系生成和条件判断两步，其中星系生成为用时大头。考虑到进行条件判断时，不满足任一条件即可剔除该种子，不是所有的星系信息都会被使用。因此，将星系生成拆分为多步，同时进行多次条件判断能够极大的提高搜索性能。

&nbsp;&nbsp;&nbsp;&nbsp;目前，星系生成被拆分为生成恒星信息、生成星球信息、生成矿物上限、生成精确矿物四个级别。每个级别生成后，都会进行一次条件判定。在快速模式下，不会进行级别4生成精确矿物的部分，通过前三个级别条件判定的种子将直接通过。

&nbsp;&nbsp;&nbsp;&nbsp;前三个级别由于生成速度较快，将一次性生成该种子所有相关信息。而级别4极其耗时，除了使用GPU加速外，还可以只生成必要星球的精确矿物。即如果该星球的矿物数量不会影响到判断结果，则不进行生成。

&nbsp;&nbsp;&nbsp;&nbsp;具体的，若某星球未通过除矿物外的条件判定或具有的矿物类型与条件要求不符（对行星和卫星条件）/无重叠（对恒星和星系条件），则不进行生成。同时，一个条件满足后将跳过对后续星体的检查。

&nbsp;&nbsp;&nbsp;&nbsp;其中，星系级矿物条件通常对性能影响最明显。若在标准模式下的星系条件对常见矿物进行要求并且数值较大（比如要求星系包含3万铁矿脉），则几乎每个星球都会生成精确矿物，性能开销接近无任何优化时。因此，不建议星系条件中包含常见矿物，只推荐单极磁石和**1~2**种稀有资源。

## 搜索器性能

搜索器性能表格如下，均为64星情况：

<table style="width:100%; border-collapse: collapse;">
  <tr class="table-header">
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">性能表格(seed/s)</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">级别1</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">级别2</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">级别3-快速</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">级别3-标准</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">级别4-标准</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">更好的出生点-标准</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">全珍奇硬飞-标准</th>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">CPU(Ultra 7 155H)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">128843</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">40688</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">26824</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">363.7</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">1.10</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">41090</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">114370</td>
  </tr>
  <tr class="zebra-row">
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">核显(Arc 128EU)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">124447</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">40542</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">26948</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">718.5</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">2.32</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">41134</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">111575</td>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">独显(RX 9070)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">123649</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">39702</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">27099</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">1440</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">4.48</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">41646</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">114068</td>
  </tr>
</table>

- 级别1：蓝巨×3
- 级别2：水世界×6 + 具有至少3个卫星的气态巨星×2
- 级别3：油×250 + 磁石矿脉×250
- 级别4：铁矿脉×30000
- 更好的出生点：距离为0的恒星系->存在冰巨星->卫星一为地中海，一为刺笋结晶矿脉×1的贫瘠荒漠
- 全珍奇硬飞：磁石矿脉×80 + 刺笋结晶矿脉×400的星系->距离为5的O型恒星系->存在气态巨星、潮汐锁定、全包星、水和硫酸、除磁石外全珍奇

## 查看器性能

查看器排序性能表格如下，均为64星情况。种子信息导出速度与标准模式近似：

<table style="width:100%; border-collapse: collapse;">
  <tr class="table-header">
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">性能表格(seed/s)</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">快速模式</th>
    <th style="border: 1px solid #ddd; padding: 10px; text-align: center;">标准模式</th>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">CPU(Ultra 7 155H)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">7878</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">1.11</td>
  </tr>
  <tr class="zebra-row">
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">核显(Arc 128EU)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">7878</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">2.09</td>
  </tr>
  <tr>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">独显(RX 9070)</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">7878</td>
    <td style="border: 1px solid #ddd; padding: 10px; text-align: center;">5.29</td>
  </tr>
</table>
