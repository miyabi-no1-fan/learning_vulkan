#! /bin/python3

import subprocess
from pathlib import Path
import shlex

CC = "clang++"

# options: "Debug" "Release"
BUILD_TYPE = "Debug"

FLAGS = "-Wall -Werror -Wextra"

if BUILD_TYPE == "Release":
    FLAGS += " -DNDEBUG -O2 -march=native -mtune=native"
elif BUILD_TYPE == "Debug":
    FLAGS += " -g -O2"

BUILD_DIR = Path("build")

SHADER_COMPILER = "glslc"
SHADER_SRC_DIR = Path("shaders")
SHADER_BUILD_DIR = Path("shaders/dist")

def run(cmd: str) -> None:
    print(cmd)
    subprocess.run(shlex.split(cmd))

def main() -> None:
    run(f"cmake -B {BUILD_DIR} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER={CC} -DCMAKE_CXX_FLAGS=\"{FLAGS}\" -DCMAKE_BUILD_TYPE={BUILD_TYPE}")
    run(f"cmake --build {BUILD_DIR} --config {BUILD_TYPE}")

    for shader in SHADER_SRC_DIR.rglob('*'):
        if shader.is_file() and not SHADER_BUILD_DIR in shader.parents:
            output = SHADER_BUILD_DIR / shader.relative_to(SHADER_SRC_DIR)
            output.parent.mkdir(parents=True, exist_ok=True)
            run(f"{SHADER_COMPILER} {shader} -o {output}.spv")

    print("")

if __name__ == "__main__":
    main()