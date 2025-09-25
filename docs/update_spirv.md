# Updating external spirv

There are three directories under `external` that are related to SPIR-V:
- external/spirv-headers
- external/spirv-tools
- external/spirv-tools-generated

In order to use the latest or custom SPIR-V, they need to be updated.


## Update SPIRV-Tools

On the Slang repo, you need to update to use the latest commit of SPIRV-Tools and SPIRV-Headers.

1. Create a branch for the update.
   ```
   # This doc will use "update_spirv" as a branch name,
   # but you can use a different name.
   git checkout -b update_spirv
   ```

1. Synchronize and update submodules.
   ```
   git submodule sync
   git submodule update --init --recursive
   ```

1. Update the SPIRV-Tools submodule to the latest version.
   ```
   git -C external/spirv-tools fetch
   git -C external/spirv-tools checkout origin/main
   ```

## Build spirv-tools

A directory, `external/spirv-tools/generated`, holds a set of files generated from spirv-tools directory.
You need to build spirv-tools in order to generate them.

```
cd external
cd spirv-tools
python3.exe utils\git-sync-deps # this step may require you to register your ssh public key to gitlab.khronos.org
cmake.exe . -B build
cmake.exe --build build --config Release

# Go back to repository root
cd ../..
```

## Update SPIRV-Tools

1. Update the SPIRV-Headers submodule to what SPIRV-Tools uses
   ```
   git -C external/spirv-headers fetch
   git -C external/spirv-tools/external/spirv-headers log -1 --oneline
   git -C external/spirv-headers checkout [commit hash from the previous command]
   ```
   Alternatively you can get the hash value of spirv-headers with the following command,
   ```
   grep spirv_headers_revision external/spirv-tools/DEPS
   ```

Note that the update of SPIRV-Headers should be done after running `python3.exe utils\git-sync-deps`, because the python script will update `external/spirv-tools/external/spirv-headers` to whichever commit the current SPIRV-Tools depends on.


## Copy the generated files from `spirv-tools/build/` to `spirv-tools-generated/`

Copy the generated header files from `external/spirv-tools/build/` to `external/spirv-tools-generated/`.
```
rm external/spirv-tools-generated/*.h
rm external/spirv-tools-generated/*.inc
cp external/spirv-tools/build/*.h external/spirv-tools-generated/
cp external/spirv-tools/build/*.inc external/spirv-tools-generated/
```


## Build Slang and run slang-test

After SPIRV submodules are updated, you need to build and test.
```
# Make sure to clean up data generated from the previous SPIRV
rm -fr build
```

There are many ways to build Slang executables. Refer to the [document](https://github.com/shader-slang/slang/blob/master/docs/building.md) for more detail.
For a quick reference, you can build with the following commands,
```
cmake.exe --preset vs2022
cmake.exe --build --preset release
```

After building Slang executables, run `slang-test` to see all tests are passing.
```
set SLANG_RUN_SPIRV_VALIDATION=1
build\Release\bin\slang-test.exe -use-test-server -server-count 8
```

It is often the case that some of tests fail, because of the changes on SPIRV-Header.
You need to properly resolve them before proceed.


## Commit and create a Pull Request

After testing is done, you need to stage and commit the updated submodule references and any generated files.
Note that when you want to use new commit IDs of the submodules, you have to stage with git-add command for the directory of the submodule itself.

```
git add external/spirv-headers
git add external/spirv-tools
git add external/spirv-tools-generated

# Add any other changes needed to resolve test failures

git commit -m "Update SPIRV-Tools and SPIRV-Headers to latest versions"
git push origin update_spirv # Use your own branch name as needed
```

Once all changes are pushed to GitHub, you can create a Pull Request on the main Slang repository.
