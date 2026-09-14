#! /bin/python3

import subprocess
from pathlib import Path
import shlex

CC = "clang++"

# options: "Debug" "Release" "dev"
BUILD_TYPE = "dev"

FLAGS = "-Wall -Werror -Wextra"

if BUILD_TYPE == "Release":
    FLAGS += " -DNDEBUG -O2 -march=native -mtune=native"
elif BUILD_TYPE == "Debug":
    FLAGS += " -g -Og"
elif BUILD_TYPE == "dev":
    BUILD_TYPE = "Debug"
    FLAGS += " -g -O2 -march=native -mtune=native"

BUILD_DIR = Path("build")

SHADER_COMPILER = "glslc"
SHADER_SRC_DIR = Path("shaders")
SHADER_BUILD_DIR = Path("shaders/dist")

def run(cmd: str) -> None:
    print(cmd)
    result = subprocess.run(
        shlex.split(cmd),
        capture_output=True, 
        text=True,
    )
    if result.stdout != '':
        print(result.stdout)
    if result.stderr != '':
        print(result.stderr)

def main() -> None:
    run(f"cmake -B {BUILD_DIR} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER={CC} -DCMAKE_CXX_FLAGS=\"{FLAGS}\" -DCMAKE_BUILD_TYPE={BUILD_TYPE}")
    run(f"cmake --build {BUILD_DIR} --config {BUILD_TYPE}")

    for shader in SHADER_SRC_DIR.rglob('*'):
        if shader.is_file() and not SHADER_BUILD_DIR in shader.parents:
            output = SHADER_BUILD_DIR / shader.relative_to(SHADER_SRC_DIR)
            output.parent.mkdir(parents=True, exist_ok=True)
            run(f"{SHADER_COMPILER} {shader} -o {output}.spv")

if __name__ == "__main__":
    main()