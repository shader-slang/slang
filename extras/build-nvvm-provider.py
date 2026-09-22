#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Build this checkout's NVVM provider with an isolated LLVM 14.0.6 toolchain."""

import argparse
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys


LLVM_COMMIT = "f28c006a5895fc0e329fe15fead81e37457cb1d1"
LLVM_REPOSITORY = "https://github.com/llvm/llvm-project.git"
SLANG_SOURCE = Path(__file__).resolve().parents[1]


def run(command, capture=False):
    """Run one command without shell interpretation, stopping on failure."""
    print("+ " + subprocess.list2cmdline([str(arg) for arg in command]), flush=True)
    result = subprocess.run(command, check=True, text=True, stdout=subprocess.PIPE if capture else None)
    return result.stdout.strip() if capture else None


class HostTools:
    """Keep tool selection and path conversion consistent for Windows-hosted WSL builds."""

    def __init__(self, requested):
        self.wsl = platform.system() == "Linux" and "microsoft" in platform.release().lower()
        native = "windows" if os.name == "nt" or self.wsl else platform.system().lower()
        self.host = native if requested == "native" else requested
        if self.host != native and not (self.wsl and self.host == "linux"):
            raise ValueError("--host does not enable cross-compilation; use the current native host")
        self.windows = self.host == "windows"
        self.cmake = self.require("cmake.exe" if self.windows else "cmake")
        if self.wsl and self.windows:
            self.require("wslpath")

    @staticmethod
    def require(name):
        """Report a missing host tool without falling back to another platform's executable."""
        tool = shutil.which(name)
        if not tool:
            raise ValueError("Required tool is missing: " + name)
        return tool

    def path(self, path):
        """Convert an absolute local path only when passing it to Windows tools from WSL."""
        value = str(Path(path).resolve())
        if self.wsl and self.windows:
            return run(["wslpath", "-w", value], capture=True)
        return value

    def git(self, *args):
        """Run the Git matching the selected build host."""
        name = "git.exe" if self.windows and self.wsl else "git"
        return run([self.require(name), *args], capture=True)


def fetch_llvm(tools, directory):
    """Fetch the immutable LLVM release commit into a script-owned sparse checkout."""
    git_dir = directory / ".git"
    if not git_dir.exists():
        if directory.exists() and any(directory.iterdir()):
            raise ValueError("Managed source directory is not empty: " + str(directory))
        directory.mkdir(parents=True, exist_ok=True)
        tools.git("init", tools.path(directory))
    prefix = ["-C", tools.path(directory)]
    # An interrupted initial fetch has no HEAD and can safely retry. An existing checkout must
    # already be the pinned source; never reset a developer's changed checkout on a rerun.
    try:
        head = tools.git(*prefix, "rev-parse", "--verify", "HEAD")
    except subprocess.CalledProcessError:
        head = None
    if head is None:
        tools.git(*prefix, "sparse-checkout", "set", "llvm", "cmake")
        tools.git(*prefix, "fetch", "--depth=1", LLVM_REPOSITORY, LLVM_COMMIT)
        tools.git(*prefix, "checkout", "--detach", LLVM_COMMIT)
        head = tools.git(*prefix, "rev-parse", "HEAD")
    if head != LLVM_COMMIT or tools.git(*prefix, "status", "--porcelain"):
        raise ValueError("Managed LLVM checkout must be clean and at " + LLVM_COMMIT)
    return directory / "llvm"


def configure(tools, source, binary, generator, definitions):
    """Configure an independent project without importing LLVM targets into Slang's project."""
    run([tools.cmake, "-S", tools.path(source), "-B", tools.path(binary), "-G", generator,
         *["-D" + key + "=" + value for key, value in definitions.items()]])


def build(tools, directory, config, jobs, targets):
    """Build only the selected targets using the chosen configuration and bounded parallelism."""
    run([tools.cmake, "--build", tools.path(directory), "--config", config,
         "--parallel", str(jobs), "--target", *targets])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=SLANG_SOURCE / "build/nvvm-provider",
                        help="isolated work directory (default: build/nvvm-provider)")
    inputs = parser.add_mutually_exclusive_group()
    inputs.add_argument("--llvm-source", type=Path, help="offline LLVM 14.0.6 llvm/ source directory")
    inputs.add_argument("--llvm-dir", type=Path, help="existing isolated LLVM 14.0.6 CMake package")
    parser.add_argument("--config", choices=["Debug", "Release", "RelWithDebInfo"], default="Release")
    parser.add_argument("--jobs", type=int, default=min(os.cpu_count() or 1, 8))
    parser.add_argument("--generator", help="CMake generator (Ninja or Visual Studio 18 2026 by default)")
    parser.add_argument("--host", choices=["native", "windows", "linux"], default="native",
                        help="WSL native uses Windows tools; explicitly choose linux for a Linux build")
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")

    tools = HostTools(args.host)
    generator = args.generator or ("Visual Studio 18 2026" if tools.windows else "Ninja")
    work = args.build_dir.resolve()
    llvm_dir = args.llvm_dir
    if llvm_dir is None:
        source = args.llvm_source or fetch_llvm(tools, work / "llvm-source")
        if not (source / "CMakeLists.txt").is_file():
            raise ValueError("--llvm-source must name LLVM's llvm/ source directory")
        llvm_build = work / "llvm-build"
        configure(tools, source, llvm_build, generator, {
            "CMAKE_BUILD_TYPE": args.config,
            "CMAKE_POSITION_INDEPENDENT_CODE": "ON",
            "BUILD_SHARED_LIBS": "OFF",
            "LLVM_ENABLE_PIC": "ON",
            "LLVM_ENABLE_EH": "OFF",
            "LLVM_ENABLE_RTTI": "OFF",
            "LLVM_ENABLE_ASSERTIONS": "OFF",
            "LLVM_BUILD_LLVM_DYLIB": "OFF",
            "LLVM_LINK_LLVM_DYLIB": "OFF",
            "LLVM_ENABLE_PROJECTS": "",
            "LLVM_ENABLE_RUNTIMES": "",
            "LLVM_TARGETS_TO_BUILD": "",
            "LLVM_INCLUDE_TESTS": "OFF",
            "LLVM_INCLUDE_BENCHMARKS": "OFF",
            "LLVM_INCLUDE_EXAMPLES": "OFF",
            "LLVM_INCLUDE_DOCS": "OFF",
            "LLVM_BUILD_TOOLS": "OFF",
            "LLVM_ENABLE_BINDINGS": "OFF",
            "LLVM_ENABLE_ZLIB": "OFF",
            "LLVM_ENABLE_TERMINFO": "OFF",
            "LLVM_ENABLE_LIBXML2": "OFF",
            "LLVM_ENABLE_LIBEDIT": "OFF",
            "LLVM_ENABLE_LIBPFM": "OFF",
            "LLVM_ENABLE_FFI": "OFF",
            "LLVM_ENABLE_Z3_SOLVER": "OFF",
            "LLVM_ENABLE_CURL": "OFF",
            "LLVM_USE_CRT_RELEASE": "MT",
            "LLVM_USE_CRT_RELWITHDEBINFO": "MT",
            "LLVM_USE_CRT_DEBUG": "MTd",
        })
        build(tools, llvm_build, args.config, args.jobs, ["LLVMCore", "LLVMBitWriter", "LLVMSupport"])
        llvm_dir = llvm_build / "lib/cmake/llvm"
    if not (llvm_dir / "LLVMConfig.cmake").is_file():
        raise ValueError("LLVM CMake package is missing: " + str(llvm_dir))

    provider_build = work / "provider-build"
    output = work / "artifacts" / args.config
    definitions = {
        "CMAKE_BUILD_TYPE": args.config,
        "SLANG_SOURCE_DIR": tools.path(SLANG_SOURCE),
        "SLANG_NVVM_LLVM_DIR": tools.path(llvm_dir),
        # CMake retains LLVM_DIR across reconfiguration. Keep it aligned when the explicit
        # package input changes in an existing provider build directory.
        "LLVM_DIR": tools.path(llvm_dir),
    }
    for kind in ["LIBRARY", "RUNTIME"]:
        definitions["CMAKE_" + kind + "_OUTPUT_DIRECTORY"] = tools.path(output)
        definitions["CMAKE_" + kind + "_OUTPUT_DIRECTORY_" + args.config.upper()] = tools.path(output)
    configure(tools, SLANG_SOURCE / "source/slang-llvm-nvvm", provider_build, generator, definitions)
    build(tools, provider_build, args.config, args.jobs, ["slang-llvm-nvvm"])
    filename = "slang-llvm-nvvm.dll" if tools.windows else (
        "libslang-llvm-nvvm.dylib" if tools.host == "darwin" else "libslang-llvm-nvvm.so")
    artifact = output / filename
    if not artifact.is_file():
        raise ValueError("Expected provider artifact was not produced: " + str(artifact))
    print("Provider: " + str(artifact))
    print("LLVM CMake package: " + str(llvm_dir.resolve()))
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print("error: " + str(error), file=sys.stderr)
        sys.exit(1)
