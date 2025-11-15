import PyInstaller.__main__
import shutil
import os
import argparse

# 读取版本号
parser = argparse.ArgumentParser(description="Build AALC")
parser.add_argument("--version", default="dev", help="AALC Version")
args = parser.parse_args()
version = args.version

with open("config/config.py", "r+", encoding="utf-8") as f:
    raw = f.read()
    content = raw.replace("PLACEHOLDER_VERSION", version, 1)
    f.seek(0)
    f.write(content)
    f.truncate()

PyInstaller.__main__.run(
    [
        "main.spec",
        "--noconfirm",
    ]
)

with open("config/config.py", "r+", encoding="utf-8") as f:
    f.seek(0)
    f.write(raw)
    f.truncate()

# 拷贝必要的文件到dist目录
shutil.copy("README.md", os.path.join("dist", "戴森球计划种子搜索查看器", "README.md"))
try:
    shutil.copy("LICENSE", os.path.join("dist", "戴森球计划种子搜索查看器", "LICENSE"))
except FileNotFoundError:
    pass
shutil.copytree("assets", os.path.join("dist", "戴森球计划种子搜索查看器", "assets"), dirs_exist_ok=True)
