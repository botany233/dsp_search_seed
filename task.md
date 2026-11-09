# GUI 结构整理任务

## 目标与执行方式

在保持现有 UI 布局、交互、计算结果、配置兼容性和文件格式不变的前提下，整理 GUI 的职责和目录，使搜索、条件编辑、查看器、排序、导出、设置各自集中。

本文件用于交接给后续智能体逐阶段执行。先阅读仓库 `AGENTS.md`，再核实当前源码；源码可能已变化，不要机械套用本文中的文件位置。默认中文沟通。

- 顺序执行，每次只实施一个阶段；完成该阶段的修改后再集中验证。
- 每阶段记录改动、验证结果、遗留限制，核对差异后才进入下一阶段。
- 本文的阶段划分不是并行执行授权，不要求创建子代理。
- 不自动提交、推送或发布，不覆盖用户已有改动。
- 勾选任务表示验收完成，不能仅因文件已经移动就勾选。
- 若无法运行 GUI，可以完成静态检查，但该阶段的运行验收必须标注“待验证”；依赖这些结果的最终验收不得标为通过。
- 临时文件、日志等信息存放在`logs`下

## 不可改变的约束

### UI 与交互

1. 保持导航顺序、页面名称、全部文案、图标、字体、颜色、主题表现和语言切换行为。
2. 保持窗口默认尺寸、页面结构、控件顺序、宽高约束、边距、间距、stretch、对齐、滚动行为、表头及列宽。
3. 保持按钮启用条件、默认值、输入校验、条件编辑入口、右键菜单、弹窗、进度展示及已有操作语义。
4. 保持布局与 QWidget 父子关系所产生的可见行为。拆页面时检查 QSS 选择器、`objectName`、`window()`、`parent()`、`parent().parent()`、弹窗定位、遮罩范围与对象生命周期；不能认为复制布局代码就一定等价。
5. 不新增用户可见功能、开关或提示，不以“体验优化”为由调整布局或交互。

### 数据与执行

1. 不修改 C++、原生扩展 `.pyd`、类型声明或 OpenCL 内核；如发现必须修改 C++ 才能继续，报告具体原因，等待人工批准。
2. 不修改搜索条件语义、条件转换结果、索引、枚举顺序、位掩码、随机序列或计算精度。
3. 保持范围／导入搜索、标准／快速模式、排序规则、资源倍率及 CPU／GPU 路径的已有语义。
4. 保持配置字段、默认值、路径和旧配置兼容性；不改变自动保存时机。保留现有 Pydantic 模型。
5. 分别保留搜索结果 CSV、查看器列表 CSV、详细导出 CSV 的现有格式，包括列、表头、分隔方式、编码、BOM、追加／覆盖行为及排序；不趁整理统一格式。
6. 继续兼容用前两列表示种子 id、恒星数量的现有导入文件，保留各入口现有的去重、范围校验、容量限制和错误处理行为。
7. 不引入新依赖、框架、通用任务系统、service 基类或多层空壳转发；不批量格式化、改名或清理无关代码。
8. 保留实际目录拼写 `Compoents`，不安排单独纠正拼写的工作。

## 当前调用链与问题定位

开始实施前重新阅读以下链路，确认仍与源码一致：

- 启动与页面：`main.py` → `GUI/MainWindow.py` → 查看器、设置、教程、关于页。
- 搜索：主窗口按钮 → `GUI/search_seed.py` → `GUI/config_to_condition.py` → `CApi/c_util.py` → 原生搜索管理器。
- 条件编辑：`GUI/Widgets/sort_tree.py` → `GUI/Compoents/sort_tree.py` → `GUI/Compoents/Widgets/setting_window.py` → 配置绑定控件 → `cfg.save()`。
- 查看与排序：`GUI/seed_viewer/MainInterface.py` → `Compoents/seed_list.py`、`Compoents/seed_scroll.py`、`sort_seed/sort_seed.py` → CApi 数据生成与排序取值规则。
- 详细导出：`seed_scroll.py` → `export_window.py` → 数据生成 → `save_seed_info.py`。
- 设置与 GPU 测试：`GUI/setting_interface.py` → `GUI/Compoents/message_box.py` → 原生 GPU 接口。

本次重点处理主窗口包含整个搜索页、功能组件散落于通用目录、数据与格式化逻辑放在组件目录、导入关系不明确等结构问题。

## 目标目录

下列目录用于明确职责，不要求创建空文件或无意义拆分。阶段 0 应核实并记录最终采用的映射。

```text
GUI/
├── MainWindow.py                 应用外壳、导航、应用级退出协调
├── dsp_icons.py
├── Widgets/                      真正跨功能复用的展示控件
├── Compoents/                    真正跨功能复用的配置绑定控件
│   └── Widgets/
├── search/
│   ├── interface.py              搜索页布局、事件及页面状态
│   ├── worker.py                 原有搜索线程与搜索执行逻辑
│   ├── config_to_condition.py    原有条件转换
│   ├── time_record.py
│   ├── progress.py               原 UserLayout
│   ├── import_seed_info.py
│   └── conditions/
│       ├── editor.py             原 SortTreeWidget 容器
│       ├── tree.py               条件树与节点操作
│       ├── fields.py             节点内条件表单
│       └── settings.py           详细条件设置页
├── seed_viewer/
│   ├── MainInterface.py
│   ├── seed_list.py              原有种子数据容器
│   ├── Compoents/                列表、星系树、详情等展示组件
│   ├── sort_seed/                保留现有排序规则划分
│   └── export/
│       ├── dialog.py             原详细导出弹窗
│       └── formatter.py          原 save_seed_info.py
├── settings/
│   ├── interface.py              设置页及设置分组
│   ├── items.py                  原设置项控件
│   └── gpu_benchmark.py          GPU 测试弹窗及其专用线程
├── tutorial.py
├── about.py
└── Messenger/                    暂保留既有全局信号契约
```

依赖规则：主窗口可以装配各页面；功能组件留在所属功能内；共享控件不得反向依赖具体页面；纯条件转换、格式化和排序取值逻辑不得依赖页面实例。不要为了目录外观整齐强行合并不同语义的控件。

## 阶段 0：建立基线与迁移清单

- [x] 执行 `git status --short`，记录已有修改；读取适用的 `AGENTS.md` 和实际调用链。
- [x] 搜索 GUI 路径在全仓中的引用，包括 `debug/`、启动入口和打包配置，形成旧路径 → 新路径映射；检查动态导入及资源路径。
- [x] 记录当前启动环境、Python／依赖、原生扩展与 GPU 可用性。仅环境未同步时使用 `uv sync --locked`，不要升级依赖。
- [x] 在同一显示环境、窗口尺寸和缩放下，记录整理前 UI 基线：导航、搜索条件、条件详情、查看器、导出弹窗、设置、GPU 测试、教程与关于页；检查中英文、浅色／深色和窗口缩放。截图等证据放仓库外，不提交运行配置。
- [x] 固定最小样例，例如种子 `0、1、42`，恒星数量 `32、64`，资源倍率 `1倍`；搜索使用小范围 `0..9`、恒星数量固定 `32`。记录条件配置、模式及导出选项，后续保持一致。
- [x] 记录搜索结果、排序值、详情关键字段和各类 CSV 基线。搜索结果中的时间注释单独比较格式，不要求时间值一致；同一模式与自身基线比较。

验收：迁移清单、操作基线和无法验证的项目已明确。基线不可取得时说明限制，不把现有行为凭印象写成预期结果。

### 迁移清单

表内路径均相对 `GUI/`。迁移前以 `86ca607` 的源码为准，开始时 `git status --short` 仅列出用户已有的未跟踪 `task.md`。

| 阶段 | 旧路径 | 拟迁入 | 调用方与依赖 |
| --- | --- | --- | --- |
| 1 | `MainWindow.py` 的搜索页 | `search/interface.py` | `main.py` 仍创建 `MainWindow`。外壳保留导航、主题和退出协调，搜索页保留 `searchLayout`、信号及宿主窗口关系。 |
| 2 | `search_seed.py` | `search/worker.py` | 搜索页创建 `SearchThread`，线程读取配置并调用转换器、`CApi.change_condition_legal` 和 `CheckBatchManager`／`CheckPreciseManager`。 |
| 2 | `config_to_condition.py`、`time_record.py` | `search/config_to_condition.py`、`search/time_record.py` | 调用方为搜索线程。条件转换只依赖配置模型。 |
| 2 | `Compoents/userLayout.py`、`Compoents/import_seed_info.py` | `search/progress.py`、`search/import_seed_info.py` | 搜索页通过 `Compoents/__init__.py` 间接导入。保留 `SearchMessages` 和共享开关。 |
| 2 | `Widgets/sort_tree.py` | `search/conditions/editor.py` | 搜索页通过 `Widgets/__init__.py` 导入。容器接收 `SortTreeMessages.CreateSettingsWindow`，在原堆叠控件内切换详情。 |
| 2 | `Compoents/sort_tree.py` | `search/conditions/tree.py`、`search/conditions/fields.py` | 先整体迁入 `tree.py`。随后将节点表单和 `SettingsTreeLeave` 及其专用辅助类迁入 `fields.py`，树节点管理留在 `tree.py`。检查 `Compoents/__init__.py`、编辑容器和详情页的导入。 |
| 2 | `Compoents/Widgets/setting_window.py` | `search/conditions/settings.py` | 编辑容器创建 `SettingsWindow`。当前详情页为类型标注直接导入 `SettingsTreeLeave`，迁移后指向 `fields.py`。配置绑定输入框仍由共享目录提供。 |
| 3 | `seed_viewer/Compoents/seed_list.py` | `seed_viewer/seed_list.py` | 检查 `MainInterface.py`、`Compoents/__init__.py`、`seed_scroll.py`、`seed_text.py` 和查看器自己的 `message_box.py`。 |
| 3 | `seed_viewer/Compoents/export_window.py`、`save_seed_info.py`（同目录） | `seed_viewer/export/dialog.py`、`seed_viewer/export/formatter.py` | `seed_scroll.py` 的右键导出通过 `self.parent().parent()` 传入宿主。弹窗通过标准模式生成数据并调用格式化器。 |
| 4 | `setting_interface.py` | `settings/interface.py`、`settings/items.py` | 主窗口创建设置页。`SettingInterface`、普通和 GPU 分组留在界面模块，`BaseSettingFrame` 与各设置项类归入 `items.py`。 |
| 4 | `Compoents/message_box.py` | `settings/gpu_benchmark.py` | 设置页通过 `Compoents/__init__.py` 导入测试弹窗。迁移该文件内的专用 `LimitLineEdit`、测试线程和弹窗。设置项还在用共享的同名输入框，分别保留。 |
| 4 | `Compoents/about_interface.py` | `about.py` | 主窗口直接导入，关于页包含版本检查线程。 |

- `seed_viewer/MainInterface.py`、查看器其余展示控件和 `sort_seed/`、`tutorial.py`、`dsp_icons.py`、`Messenger/` 保留原路径。共享 `WaitRing` 仍由主窗口和星系树使用。共享绑定控件仍在 `Compoents/`。阶段 5 处理受影响的包导出和星号导入。
- `main.py` 中的局部导入为静态模块路径。全仓检查未发现通过字符串动态加载待迁移 GUI 模块的调用。现有 `debug/` 脚本通过 `CApi` 运行，部分手动把仓库根加入搜索路径。两份 README 链接的 `custom_sort.py` 路径保留。
- `main.spec` 分析 `main.py`，`hiddenimports` 为空，显式打包 `CApi/search_seed.pyd`。`scripts/build.py` 在打包后复制整个 `assets/`，CI 调用该入口。源码启动以 `main.py` 所在目录为工作目录，冻结版以可执行文件目录为准。图标、语言 JSON、教程及其相对图片、OpenCL 内核依赖现有资源位置。

### 固定环境和样例

- Windows build 26200（25H2），Python 3.13.6 x64，PySide6 6.9.3，PySide6-Fluent-Widgets 1.9.2，Pydantic 2.12.3，PyInstaller 6.16.0。`uv sync --check --locked` 显示环境无需变更。
- 原生扩展加载成功。`CApi/search_seed.pyd` 的 SHA-256 为 `353C128B659E1BA5E31D0F46CA53F39CDEDFB1BDC6B90D9918BBBBD35B739CB9`。OpenCL 枚举到 AMD gfx1201 和 Intel Arc Graphics，固定数据样例使用 CPU（设备 `-1`）。
- 窗口和数据从独立副本运行，沿用 `main.main()` 创建实际窗口。Qt 测试通过控件信号执行，文件对话框由测试提供固定路径。主屏为 `G72`，Windows 显示倍率 125%。中文默认窗口 `1320×660`，应用缩放 `1.0`，Qt 屏幕逻辑分辨率 `2048×1152`，像素倍率 `1.25`。英文默认窗口 `1571×786`，应用缩放 `0.84`，Qt 屏幕逻辑分辨率 `2438×1371`，像素倍率 `1.05`。窗口缩小样例为 `1120×580` 逻辑像素。
- UI 截图用默认参数的完整条件树，包含星区、恒星、行星、卫星。数据搜索使用无子条件的有效星区条件，规范化结果为 `{"type":5,"value":{"satisfy_num":0,"children":[]}}`。搜索以两线程、1 倍资源执行，范围为种子 `0..9`、恒星数固定 `32`。导入搜索及查看器使用种子 `0、1、42` 各配 `32、64` 星，输入另含表头、重复的 `42,64` 和非法的 `-1,32`。
- 排序选矿脉数量中的铁，分别采集标准、快速及升序、降序。详情和详细导出用标准生成路径，命名语言为 `English`。详细导出启用星区、恒星、行星三表，其余选项采用 `CSVExportCondition` 默认值，完整选项及条件转换记录在数据基线 JSON。

### 实测基线

- 截图在仓库外的 `C:/Users/18201/.codex/visualizations/2026/11/09/01a1c043-4134-7870-bcae-e214937bd244/phase0/evidence`，按 `zh_CN` 和 `en_US` 分组。两种语言均采集搜索页及条件详情、查看器恒星详情、详细导出弹窗、设置、GPU 测试、教程、关于，以及缩小后的搜索页。浅色和深色分别记录。导航控件读出的顺序为种子搜索器、种子查看器、使用教程、设置、关于，对应 Seed Searcher、Seed Viewer、Tutorial、Settings、About。
- 可对照的 JSON 与原始 CSV 位于已忽略的 `logs/phase0/data/{zh_CN,en_US}/`。每种语言包含输入 CSV、4 个搜索结果、4 个列表 CSV 和 6 个详细 CSV。`observed.json` 记录条件及其转换值、排序行、6 组详情字段和每个 CSV 的 SHA-256、BOM 与换行。两种语言的条件转换、排序值和详情关键字段逐项一致。
- 搜索范围两种模式各返回 `0..9` 的 10 行，导入两种模式各返回 6 个有效组合。搜索 CSV 以 UTF-8 无 BOM 追加，行格式为 `种子, 恒星数`。标准范围搜索连续执行两次，确认追加了两个区块。时间注释每次单独匹配 `#search seed time YYYY-MM-DD HH:MM:SS`，其余行按顺序比较。
- 取种子 `0`、32 星为例，标准铁矿脉数量为 `14244`，快速为 `17818`。该样例的第一颗恒星为 Iota Sagittae（G 型恒星），戴森球半径 `21900 m`。排序结果的升降序互为反序，列表 CSV 按表格顺序覆盖写入，编码为 UTF-8 无 BOM，表头为 `seed_id, star_num, value`。
- 详细导出按 `<seed_id>_<star_num>.csv` 覆盖写入，每文件依次记录选中的星区、恒星、行星区块，实测为 UTF-8 BOM。三种输出 CSV 均使用 Windows CRLF。后续按同语言、同模式、同选项比较详细 CSV 字节。
- AMD 和 Intel 均通过短原生基准。初次使用默认临时目录时，AMD 的 OpenCL 编译因临时文件重命名权限失败。将测试进程的 `TEMP`／`TMP` 指向 `logs/phase0/gpu-temp` 后，两设备均能启用。另在 AMD 上用应用内弹窗跑完 2 个 CPU 线程、GPU worker `0..1`、每档 1 秒的测试，截图为 `zh_CN/gpu_amd_result.png`，数据在 `navigation-gpu.json`。耗时和吞吐仅记录样例。
- 已复现的整理前异常为种子 `0`、32 星的星区汇总面板触发 `IndexError`。`astro_info.py` 用固定 14 项数组按原生 `star.type_id` 计数，恒星详情和详细 CSV 仍可采集。静态核对还发现详细导出的“创建行星表”总开关当前绑定 `csv.star.enable`。后续迁移将这些行为列为单独问题。
- 待验证范围包括 Windows 原生 Mica／标题栏合成效果、系统文件选择器、应用内切换语言后的重启流程、无 OpenCL 设备分支，以及搜索运行中的暂停／继续／停止和关闭流程。本阶段截图采集 Qt 界面，透明标题栏和导航背景会呈黑色。最终回归按后续各阶段的验收范围执行。

## 阶段 1：提取搜索页面

- [ ] 新增 `GUI/search/interface.py`，提取 `SearchInterface`；将主窗口中的搜索布局、控件创建、条件恢复、搜索模式切换、种子导入和按钮事件迁入。
- [ ] 搜索页拥有自己的种子管理器、搜索线程和页面状态；保留原信号、执行顺序、暂停／继续／停止语义。
- [ ] 主窗口保留主题、字体、窗口属性、导航、SIGINT 和应用级关闭协调。通过明确的页面方法访问退出所需任务，避免依赖页面内部布局和控件。
- [ ] 不同时重写关闭流程，不修改搜索线程实现，不添加通用 controller 或任务管理器。
- [ ] 特别核对搜索页的 `objectName`、父控件、弹窗 `window()`、遮罩范围和布局层级；保留原尺寸、边距、stretch 与控件顺序。

验收：页面切换、条件恢复、两种搜索来源、开始／暂停／继续／停止及运行时关闭行为与基线一致；搜索结果一致；主窗口不再创建和操作搜索页具体控件。

## 阶段 2：归拢搜索和条件编辑文件

- [ ] 将 `search_seed.py`、`config_to_condition.py`、`time_record.py`、`UserLayout`、`ImportSeedInfo` 迁入 `search/` 对应位置，保留类名、函数接口和实现。
- [ ] 将 `GUI/Widgets/sort_tree.py` 迁入 `search/conditions/editor.py`。
- [ ] 先将 `GUI/Compoents/sort_tree.py` 整体迁入 `search/conditions/tree.py`，再把节点内表单和详细设置入口等职责明确的代码提取至 `fields.py`。
- [ ] 将 `GUI/Compoents/Widgets/setting_window.py` 迁入 `search/conditions/settings.py`；共享输入框、复选框、组合框仍保留在共享目录。
- [ ] 同步更新全部调用方及包导出。迁移中保持全局 `SortTreeMessages` 的契约和绑定方式，不顺带改为另一套事件机制。
- [ ] 如拆分引入循环导入，调整职责或仅用于类型标注的导入；不要用广泛延迟导入、`sys.path` 修改或兼容转发层掩盖依赖问题。

验收：星系／恒星／行星／卫星条件的增删、重命名、勾选、矿物详情编辑及重启恢复一致；条件转换输出对固定配置逐项一致；UI 无差异。除迁移所必需的包结构外，不改变条件逻辑。

## 阶段 3：整理查看器的数据与导出职责

- [ ] 将 `seed_viewer/Compoents/seed_list.py` 迁至 `seed_viewer/seed_list.py`，保留容量限制、校验、锁与更新方式。
- [ ] 将 `export_window.py` 迁至 `export/dialog.py`，将 `save_seed_info.py` 迁至 `export/formatter.py`，同步更新导入。
- [ ] 保留 `sort_seed/` 现有分类与取值实现，保持标准／快速路径、线程数计算和排序方向。
- [ ] 保留列表与数据容器的现有同步机制；不迁移到 `QAbstractTableModel`，不调整缓存策略、表格重建或导出执行流程。
- [ ] 检查种子表格打开导出弹窗的父级引用。若迁移确实改变了层级，只做保持相同宿主窗口和生命周期所必需的显式传参。

验收：手动添加、重复项、非法项、CSV 导入、容量限制、批量删除、详情缓存、各排序大类和方向、列表导出、详细导出及取消行为一致；导出文件与基线比较，不能只检查文件存在。

## 阶段 4：归拢设置页和辅助页面

- [ ] 将 `setting_interface.py` 迁至 `settings/interface.py`；将设置项基础控件提取至 `settings/items.py`，保持原初始化和信号连接顺序。
- [ ] 将 GPU 测试专用 `LimitLineEdit`、`GPUBenchmarkThread`、`GPUBenchmarkMessageBox` 迁至 `settings/gpu_benchmark.py`。它与共享目录中的同名输入框语义不同，不直接合并。
- [ ] 将 `Compoents/about_interface.py` 迁至 `GUI/about.py`，保留更新检查线程及行为；教程页保持原样。
- [ ] 检查所有 `objectName`、主题选择器、宽度计算、设置项联动、配置保存与 GPU 接口调用顺序。

验收：普通设置和语言设置、GPU 可用／不可用显示、设备与 worker 设置、GPU 测试开始／结束／关闭、教程加载及关于页行为与基线一致。无 GPU 时标明对应运行路径未验证。

## 阶段 5：显式化依赖并收尾

- [ ] 仅对本次迁移涉及的 GUI 模块和相关 `__init__.py`，将星号导入替换为实际使用的显式导入，移除已无调用方的旧导出。
- [ ] 检查共享目录，确认其中没有条件编辑、GPU 测试、关于页、种子数据容器或详细导出等业务专用实现。
- [ ] 检查旧路径的全仓引用并逐项处理；不保留没有实际外部调用方的旧路径兼容壳。文档中的历史说明与运行引用分别判断。
- [ ] 仅删除本次迁移直接产生的重复代码和确认不可达代码，不扩大为全仓清理。
- [ ] 审查主窗口与各页面的所有权、信号连接次数和退出路径，确认没有因迁移新增对象泄漏、重复回调或线程提前销毁。

验收：启动入口、受影响调试入口及相关打包静态引用均指向有效路径；无新增循环依赖；不存在“新旧两份实现同时维护”的状态。

## 本次暂不实施的行为性调整

以下是审阅发现的后续候选问题，不属于本次结构整理的完成条件。记录具体位置，单独立项，不静默夹带修复：

- 排序线程直接读取 GUI 控件；后续可改为主线程取得参数快照后启动任务。
- `Config.copy()` 当前是浅复制；后续可核实是否需要深复制及对运行中编辑条件的影响。
- 详情生成、详细导出和部分关闭等待使用 `processEvents()` 轮询；后续可分别改为定时取结果或后台任务。
- 全局 Messenger 改为实例信号，重新设计任务成功／失败／取消状态和资源释放策略。
- 重命名 `terminate()` 为停止请求接口，调整同时运行多个任务时的关闭协调。
- 统一配置自动保存机制、抽取 CSV 共用读取逻辑、引入表格 model、调整格式化算法或修复已有 CSV 格式问题。

这些调整可能合理，但会改变执行时序、资源生命周期或可观察行为。本次优先建立清晰目录和职责；若现有问题直接阻碍迁移，先说明阻碍和最小处理方案，不能把行为变化当作纯移动文件。

## 每阶段验证要求

1. 阶段修改完成后，对实际修改／新增的 Python 文件执行纯语法检查。例如使用 `uv run --locked python -m py_compile <文件列表>`；它不等于模块导入验证，产生的缓存不得纳入交付。
2. 检查导入路径和模块初始化副作用。`CApi/__init__.py` 会初始化原生代码，`config` 导入可能读写运行配置，不通过随意 import 生成所谓“纯静态验证”。
3. 在仓库根目录使用 `uv run --locked python main.py`，验证本阶段受影响的操作。复用既有调试入口前先读源码、限制规模，不默认运行全部 `debug/` 或依赖缺失的 `compare_capi.py`。
4. 与阶段 0 在相同条件下比较 UI。比较截图时排除鼠标悬停、光标、时间和实时进度等动态差异；控件位置、尺寸、字体、换行、遮罩或主题差异必须解释并消除。
5. 数据比较固定输入、资源倍率、条件与模式；快速模式只与快速模式基线比较。涉及原生路径的运行限制如实记录。
6. 执行 `git diff --check`、`git diff --stat`、`git status --short` 并阅读实际差异，确认没有运行配置、日志、CSV、截图、缓存、构建产物或无关改动混入。
7. 不为这次整理新增测试框架或独立大规模测试脚本；必要的最小验证优先使用既有入口或临时命令。

## 最终验收与交接记录

- [ ] 中英文、浅色／深色、常用窗口尺寸下，与基线的布局、文案和控件行为一致。
- [ ] 搜索两种来源、标准／快速模式、条件编辑、查看详情、排序、导出、设置及退出流程完成回归。
- [ ] 固定输入的搜索结果、条件转换、排序值、详情字段和导出格式保持一致。
- [ ] 全部适用阶段验证通过；无法验证项单独列出，不以语法检查代替 GUI 或原生功能验证。
- [ ] 最终差异仅包含本次结构整理和必需的引用更新，没有修改 C++、原生二进制、OpenCL、依赖或本机配置。

每阶段完成后在此追加简短记录：

```text
阶段：
状态：完成 / 待验证 / 受阻
实际迁移与职责变化：
与原计划的差异及原因：
语法与静态检查结果：
UI 操作、输入参数、数据及文件对照结果：
未验证范围与环境限制：
下一阶段需要注意的事项：
```

#### 第 0 阶段交接（2026-11-09）

- **状态**　完成基线采集。仍需人工或特定环境验证的项目见本阶段实测记录。
- **改动**　仅补充本文的迁移清单、固定环境、运行证据和现有问题。
- **检查**　核对源码、相对导入、包导出、调试和打包入口。独立副本完成应用启动、截图、CPU 样例及 GPU 测试。复核搜索时间行格式、排序方向、CSV 编码与 BOM、换行和文件摘要。仓库内原有运行配置的 SHA-256 与采集前一致。
- **下一阶段**　从主窗口提取搜索页时，优先按相同窗口参数复查 `searchLayout`、导航切换、条件详情的堆叠关系及搜索任务信号。范围和导入搜索分别按标准／快速样例复跑，使用各自的 CSV 基线。按阶段 1 的要求补测运行时暂停、继续、停止及关闭。
