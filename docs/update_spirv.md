# Updating external spirv

There are three directories under `external` that are related to SPIR-V:
- external/spirv-headers
- external/spirv-tools
- external/spirv-tools-generated

In order to use the latest or custom SPIR-V, they need to be updated.

## Update URLs to the repo

Open `.gitmodules` and update the following sections to have a desired `url` set.
```
[submodule "external/spirv-tools"]
	path = external/spirv-tools
	url = https://github.com/shader-slang/SPIRV-Tools.git
[submodule "external/spirv-headers"]
	path = external/spirv-headers
	url = https://github.com/KhronosGroup/SPIRV-Headers.git
```

Run the following command to apply the change.
```
git submodule sync
git submodule update --init --recursive
```

If you need to use a specific branch from the repos, you need to manually checkout the branch.
```
cd external/spirv-tools
git checkout branch_name_to_use
cd ..
git add spirv-tools

cd external/spirv-headers
git checkout branch_name_to_use
cd ..
git add spirv-headers
```

## Build spirv-tools

A directory, `external/spirv-tools/generated`, holds a set of files generated from spirv-tools directory.
You need to build spirv-tools in order to generate them.

```
cd external/spirv-tools
python3.exe utils/git-sync-deps # this step may require you to register your ssh public key to gitlab.khronos.org
mkdir build
cd build
cmake.exe ..
cmake.exe --build . --config Release
```

## Copy the generated files from spirv-tools

Copy some of generated files from `external/spirv-tools/build/` to `external/spirv-tools-generated`.
The following files are ones you need to copy at the momment, but the list will change in the future.
```
DebugInfo.h
NonSemanticShaderDebugInfo100.h
OpenCLDebugInfo100.h
build-version.inc
core.insts-unified1.inc
debuginfo.insts.inc
enum_string_mapping.inc
extension_enum.inc
generators.inc
glsl.std.450.insts.inc
nonsemantic.clspvreflection.insts.inc
nonsemantic.shader.debuginfo.100.insts.inc
nonsemantic.vkspreflection.insts.inc
opencl.debuginfo.100.insts.inc
opencl.std.insts.inc
operand.kinds-unified1.inc
options-pinned.h
spv-amd-gcn-shader.insts.inc
spv-amd-shader-ballot.insts.inc
spv-amd-shader-explicit-vertex-parameter.insts.inc
spv-amd-shader-trinary-minmax.insts.inc
```

