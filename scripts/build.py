import PyInstaller.__main__
import shutil
import os


PyInstaller.__main__.run(
    [
        "main.spec",
        "--noconfirm",
    ]
)

# 拷贝必要的文件到dist目录
shutil.copy("README.md", os.path.join("dist", "戴森球计划种子搜索查看器", "README.md"))
try:
    shutil.copy("LICENSE", os.path.join("dist", "戴森球计划种子搜索查看器", "LICENSE"))
except FileNotFoundError:
    pass
shutil.copytree("assets", os.path.join("dist", "戴森球计划种子搜索查看器", "assets"), dirs_exist_ok=True)
