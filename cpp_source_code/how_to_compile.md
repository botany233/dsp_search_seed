注：以下内容由ai生成

# 如何编译

本文档介绍如何从源码编译本项目。

## 环境要求

1.  **操作系统**: Windows 10/11 (因为涉及到 `.pyd` 编译和 Win32 API 依赖)
2.  **编译器**: Visual Studio 2022 (安装“使用 C++ 的桌面开发”工作负载、MSVC v143 和 Windows SDK；工程使用 C++20、AVX2)
3.  **Python**: 64 位 Python 3.13.x (项目要求 `>=3.13,<3.14`，需包含开发用头文件和库)
4.  **包管理器**:
    *   `vcpkg` (用于管理 C++ 依赖)

## 编译步骤

1.  **打开项目**: 使用 Visual Studio 打开 `cpp_source_code/test_pybind11.sln`。
2.  **配置依赖**:
    *   先还原 `packages.config` 中的 NuGet 包 `vcpkg.D.vcpkg.vcpkg`；工程会检查其 `.props` 和 `.targets` 是否存在。
    *   项目已启用 vcpkg manifest 模式，`vcpkg.json` 当前仅声明 `opencl`。确认本机 vcpkg 集成可用，并已安装 x64 所需的 OpenCL 头文件和库。
    *   仓库未附带 pybind11 头文件，需将其 `include/pybind11/` 放到 `cpp_source_code/include/pybind11/`，或配置对应包含目录。
3.  **修改 Python 路径 (重要)**:
    *   由于项目配置文件中保留了开发者的本地路径，你需要根据你的环境修改 Include 和 Library 路径。
    *   右键项目 `search_seed` -> **属性 (Properties)**。
    *   选择 **配置 (Configuration)** 为 `Release`，**平台 (Platform)** 为 `x64`。
    *   进入 **配置属性 (Configuration Properties)** -> **VC++ 目录 (VC++ Directories)**。
    *   修改 **包含目录 (Include Directories)**: 将指向 `Python313\include` 的路径改为你本机 Python 的 include 目录。
    *   修改 **库目录 (Library Directories)**: 将指向 `Python313\libs` 的路径改为你本机 Python 的 libs 目录。
    *   检查 **C/C++** -> **常规** -> **附加包含目录**：将 `D:\vcpkg\vcpkg\installed\x64-windows\include` 改为本机实际依赖目录，确认可找到 `CL/opencl.hpp`。
4.  **编译**:
    *   选择构建配置为 `Release` 和 `x64`。 (当前 `Debug | x64` 配置生成 `.exe`，不能用作 Python 扩展)
    *   点击 **生成 (Build)** -> **生成解决方案 (Build Solution)**。
5.  **部署扩展**:
    *   编译成功后，按 VS 构建输出确认 `search_seed.pyd` 的实际路径，默认通常位于 `cpp_source_code/x64/Release/`。
    *   将 `search_seed.pyd` 复制到项目根目录下的 `CApi/` 文件夹中，加载它的 Python 必须同为 3.13、64 位。

## 常见问题

*   **找不到 Python.h**: 请检查 VS 项目属性中的 Include 路径是否正确指向了你的 Python 安装目录。
*   **链接错误**: 请检查 Library 路径是否正确；当前工程同时链接 `python3.lib` 和 `python313.lib`，两者都需存在。
*   **OpenCL 错误**: 编译时检查 OpenCL 头文件和库；运行时使用 GPU 还需安装支持 OpenCL 的显卡驱动。
