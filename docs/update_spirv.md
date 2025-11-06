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

## Update SPIRV-Headers

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

## CI Validation

The Slang CI system includes an automated check that verifies the generated files in `external/spirv-tools-generated/` are up-to-date whenever changes are made to `external/spirv-tools` or `external/spirv-headers`.

### What the CI Check Does

When you create a Pull Request that modifies the SPIRV submodules, the CI will:

1. Detect changes to `external/spirv-tools` or `external/spirv-headers`
2. Verify that the `spirv-headers` commit matches what `spirv-tools/DEPS` expects
3. Automatically regenerate the files that should be in `external/spirv-tools-generated/`
4. Compare the regenerated files with the committed files
5. **Fail the CI** if there are any discrepancies or mismatches

This ensures that:
- The `spirv-headers` version is compatible with `spirv-tools`
- All generated files are correctly synchronized with the SPIRV-Tools version

### When Does the CI Check Run?

The check only runs when:
- A pull request modifies `external/spirv-tools/**`
- A pull request modifies `external/spirv-headers/**`
- A pull request modifies `external/spirv-tools-generated/**`
- The workflow or check script itself is modified

This path filtering prevents unnecessary builds and keeps CI fast.

### What Files Are Validated?

The check verifies that the following generated files are present and up-to-date:
- `*.inc` files (e.g., `build-version.inc`, `core_tables_body.inc`, etc.)
- `*.h` files (e.g., `DebugInfo.h`, `OpenCLDebugInfo100.h`, etc.)

The `README.md` file in `external/spirv-tools-generated/` is excluded from validation.

### If the CI Check Fails

If you see a failure from the "Check SPIRV Generated Files" job, it means the generated files are out of sync. The CI output will show:

- Which files are missing
- Which files have differences
- Which files are orphaned (should be removed)

To fix the issue, follow the instructions in the CI output, or re-run the steps in the [Copy the generated files](#copy-the-generated-files-from-spirv-toolsbuild-to-spirv-tools-generated) section above.

#### Common Failure Scenarios

**Scenario 1: Missing Files**
```
ERROR: Missing file in spirv-tools-generated: new_file.inc
```
This means a new file is now generated by SPIRV-Tools but hasn't been added to the repository.

**Fix:** Follow the copy steps in the error message to add the new file.

---

**Scenario 2: Outdated Files**
```
ERROR: File differs: build-version.inc
```
This means the file exists but its content doesn't match what SPIRV-Tools would generate.

**Fix:** Regenerate and replace the file following the instructions in the CI output.

---

**Scenario 3: Orphaned Files**
```
ERROR: Orphaned file in spirv-tools-generated: old_file.inc
```
This means a file exists in the repository but is no longer generated by SPIRV-Tools.

**Fix:** Remove the file with `git rm external/spirv-tools-generated/old_file.inc`

---

**Scenario 4: spirv-headers Commit Mismatch**
```
ERROR: spirv-headers commit mismatch!
ERROR:   Expected (from spirv-tools/DEPS): 6bb105b6c4b3a246e1e6bb96366fe14c6dbfde83
ERROR:   Actual (submodule):                1234567890abcdef1234567890abcdef12345678
```
This means the `spirv-headers` submodule commit doesn't match what `spirv-tools` expects in its DEPS file.

**Fix:** Update `spirv-headers` to the expected commit:
```bash
git -C external/spirv-headers fetch
git -C external/spirv-headers checkout 6bb105b6c4b3a246e1e6bb96366fe14c6dbfde83
git add external/spirv-headers
```

---

**Scenario 5: Submodule Not Updated**

If you updated generated files but forgot to update the submodule reference:
```bash
git add external/spirv-tools
git add external/spirv-tools-generated
```

### Testing Locally

You can run the same check locally before pushing to catch issues early:

```bash
bash extras/check-spirv-generated.sh
```

This script will verify that your generated files match what would be produced by building SPIRV-Tools with the current submodule version.
