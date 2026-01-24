注：以下内容由ai生成

# 如何编译

本文档介绍如何从源码编译本项目。

## 环境要求

1.  **操作系统**: Windows 10/11 (因为涉及到 `.pyd` 编译和 Win32 API 依赖)
2.  **编译器**: Visual Studio 2022 (需要安装 "C++ 桌面开发" 工作负载)
3.  **Python**: 建议 Python 3.13 (与项目配置一致，其他版本需修改配置)
4.  **包管理器**:
    *   `vcpkg` (用于管理 C++ 依赖)

## 编译步骤

1.  **打开项目**: 使用 Visual Studio 打开 `cpp_source_code/test_pybind11.sln`。
2.  **配置依赖**:
    *   项目已启用 vcpkg manifest 模式，Visual Studio 应自动识别 `vcpkg.json` 并安装 `glm` 和 `opencl`。
    *   如果未自动安装，请确保 Visual Studio 的 vcpkg 集成已启用。
3.  **修改 Python 路径 (重要)**:
    *   由于项目配置文件中保留了开发者的本地路径，你需要根据你的环境修改 Include 和 Library 路径。
    *   右键项目 `search_seed` -> **属性 (Properties)**。
    *   选择 **配置 (Configuration)** 为 `Release`，**平台 (Platform)** 为 `x64`。
    *   进入 **配置属性 (Configuration Properties)** -> **VC++ 目录 (VC++ Directories)**。
    *   修改 **包含目录 (Include Directories)**: 将指向 `Python313\include` 的路径改为你本机 Python 的 include 目录。
    *   修改 **库目录 (Library Directories)**: 将指向 `Python313\libs` 的路径改为你本机 Python 的 libs 目录。
4.  **编译**:
    *   选择构建配置为 `Release` 和 `x64`。 (Debug 模式产生的 `.pyd` 可能无法被常规 Python 加载，且性能较差)
    *   点击 **生成 (Build)** -> **生成解决方案 (Build Solution)**。
5.  **部署扩展**:
    *   编译成功后，生成的 `search_seed.pyd` 通常位于 `cpp_source_code/x64/Release/` 目录下。
    *   将 `search_seed.pyd` 复制到项目根目录下的 `CApi/` 文件夹中。

## 常见问题

*   **找不到 Python.h**: 请检查 VS 项目属性中的 Include 路径是否正确指向了你的 Python 安装目录。
*   **链接错误**: 请检查 Library 路径是否正确，且 `python3.lib` 或 `python313.lib` 存在。
*   **OpenCL 错误**: 需确保系统安装了显卡驱动，且 vcpkg 正确安装了 opencl SDK。
