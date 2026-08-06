# Running VK-GL-CTS with Slang locally

The nightly workflow [`.github/workflows/nightly-slang-vkglcts-test.yml`](../.github/workflows/nightly-slang-vkglcts-test.yml)
runs the Vulkan Conformance Test Suite (VK-GL-CTS) with Slang substituted for glslang as the
GLSL-to-SPIR-V compiler. This document explains how that works and how to reproduce a nightly
failure on a local machine.

Throughout this document, the running example is a real regression: the nightly reports
`dEQP-VK.descriptor_indexing.storage_image_minNonUniform` as failing. That case is line 3 of
`slang-passing-tests.txt`, so it is a test that used to pass with Slang — the steps below narrow it
down from "the nightly is red" to a single `slangc` command.

## How the Slang integration works

The tests come from the Slang fork of the CTS, <https://github.com/shader-slang/VK-GL-CTS>. That
fork adds two things to upstream VK-GL-CTS:

- `external/vulkancts/framework/vulkan/vkShaderToSpirV_slang.cpp` — the Slang glue.
- A hook at the top of `vk::compileGlslToSpirV()` in
  `external/vulkancts/framework/vulkan/vkShaderToSpirV.cpp` that redirects shader compilation into
  that glue instead of glslang.

The hook is compiled in only when `ENABLE_SLANG_COMPILATION` is defined and the host is Windows, so
**the Slang-enabled CTS is a Windows-only configuration**.

At run time the glue does the following for every shader the CTS wants to compile:

1. Writes the GLSL source that the test generated to a file named `test.slang.<ext>` in the working
   directory, where `<ext>` is `.vert`, `.frag`, `.geom`, or `.comp` depending on the stage. This
   file is overwritten for every shader, so at any moment it holds the most recently compiled one —
   which, after a crash or a hang, is the shader that broke.
2. Loads `slang-compiler.dll` from the current directory (or from `%SLANG_DLL_PATH_OVERRIDE%`).
3. Spawns `test-server.exe` once and sends it a JSON-RPC `tool` request per shader, equivalent to
   this command line:

   ```
   slangc test.slang.<ext> -target spirv -stage <stage> -entry main -allow-glsl -matrix-layout-row-major
   ```

   The server returns SPIR-V assembly, which the glue assembles and hands back to the CTS. Running
   the compiler out-of-process is what keeps a Slang crash or assertion from taking down the whole
   `deqp-vk` run.

4. Falls back to compiling in-process through `slang-compiler.dll` when server mode is disabled (see
   the environment variables below).

Environment variables the glue reads:

| Variable                          | Effect                                                                                                                    |
| --------------------------------- | ------------------------------------------------------------------------------------------------------------------------- |
| `DISABLE_CTS_SLANG=1`             | Bypass Slang entirely and compile with glslang, i.e. the upstream CTS behavior. Use this to get a baseline for a failure. |
| `DISABLE_CTS_SLANG_SERVER_MODE=1` | Compile in-process via `slang-compiler.dll` instead of spawning `test-server.exe`. Useful for attaching a debugger.       |
| `SLANG_DLL_PATH_OVERRIDE`         | Directory to load `slang-compiler.dll` and `test-server.exe` from, instead of the working directory.                      |

The nightly sets `DISABLE_CTS_SLANG: 0`, so Slang is used.

## Prerequisites

- Windows with a working Vulkan driver and a GPU. The tests actually execute on the device; there is
  no CPU fallback.
- A Release build of Slang.
- `gh.exe` (or a browser) to download the prebuilt CTS release.
- Under WSL, use the Windows binaries — `cmake.exe`, `git.exe`, `gh.exe` — and run `deqp-vk.exe`
  from a Windows shell (PowerShell or `cmd`), not from inside WSL. The CTS binary needs the Windows
  Vulkan loader and the Windows-side working directory semantics that the Slang glue relies on.

## Step 1: Build Slang the way the nightly does

The nightly configures with tests enabled, because it needs `test-server.exe`:

```bash
cmake.exe --preset default --fresh \
  -DSLANG_SLANG_LLVM_FLAVOR=DISABLE \
  -DSLANG_ENABLE_CUDA=0 \
  -DSLANG_ENABLE_EXAMPLES=0 \
  -DSLANG_ENABLE_TESTS=1
cmake.exe --workflow --preset release
```

SLANG_ENABLE_TESTS needs to be enabled to use test-server.exe for concurrent execution.
What matters is a **Release** build (the prebuilt `deqp-vk.exe` is Release, so a Debug
`slang-compiler.dll` will not be ABI-compatible with it in general) that produces these four files
in `build/Release/bin`:

- `slang-compiler.dll`
- `slang-glslang.dll`
- `slang-glsl-module.dll`
- `test-server.exe`

## Step 2: Get the test lists

You do not need to build the CTS, and you do not need the full fork checkout — the run itself comes
from a prebuilt release binary (Step 3). All that is needed from the repository are the two
test-list files, so do what the workflow does and take a sparse, shallow checkout of just those:

```bash
# From the repository root. external/vk-gl-cts/ is already in .gitignore.
git.exe clone --depth 1 --filter=blob:none --sparse \
  https://github.com/shader-slang/VK-GL-CTS.git external/vk-gl-cts
git.exe -C external/vk-gl-cts sparse-checkout set --no-cone \
  test-lists/slang-passing-tests.txt test-lists/slang-waiver-tests.xml
```

That leaves you with:

- `external/vk-gl-cts/test-lists/slang-passing-tests.txt` — the case list the nightly runs. These are
  the tests known to pass with Slang, so any failure here is a regression. Our example case is in it:

  ```bash
  grep -n storage_image_minNonUniform external/vk-gl-cts/test-lists/slang-passing-tests.txt
  # 3:dEQP-VK.descriptor_indexing.storage_image_minNonUniform
  ```

- `external/vk-gl-cts/test-lists/slang-waiver-tests.xml` — tests waived on specific devices/drivers,
  which the runner reports but does not fail on.

Both files track the fork's `main` branch, not the release, so re-run the clone (or a `git pull`) to
pick up list changes. If you later need the CTS or glue sources to read, debug, or build, widen the
sparse checkout — see [the appendix](#appendix-building-deqp-vk-from-source).

## Step 3: Lay out the prebuilt CTS

The nightly downloads a prebuilt, Slang-enabled `deqp-vk.exe` from the fork's GitHub releases. Check
the workflow file for the exact version it currently pins — at the time of writing that is
`VK-GL-CTS_WithSlang-0.0.9-win64.zip`. Reproduce the same layout locally:

```bash
# From the repository root. Adjust the version to match the workflow.
CTS_ZIP=VK-GL-CTS_WithSlang-0.0.9-win64.zip
CTS_DIR=external/vk-gl-cts-bin

gh.exe release download --repo shader-slang/VK-GL-CTS --pattern "$CTS_ZIP" --dir external
unzip -d "$CTS_DIR" "external/$CTS_ZIP"
```

Then copy in your locally built Slang binaries and the test lists, exactly as the `vkcts setup` step
does. The DLLs and `test-server.exe` must sit next to `deqp-vk.exe`, because the glue loads them
from the working directory:

```bash
cp build/Release/bin/slang-compiler.dll   "$CTS_DIR"/
cp build/Release/bin/slang-glslang.dll    "$CTS_DIR"/
cp build/Release/bin/slang-glsl-module.dll "$CTS_DIR"/
cp build/Release/bin/test-server.exe      "$CTS_DIR"/

cp external/vk-gl-cts/test-lists/slang-passing-tests.txt "$CTS_DIR"/
cp external/vk-gl-cts/test-lists/slang-waiver-tests.xml  "$CTS_DIR"/
```

Both `external/vk-gl-cts/` and `external/vk-gl-cts-bin/` are in `.gitignore`. If you unpack the CTS
somewhere else, add that path to `.git/info/exclude` so it does not show up in `git status`.

## Step 4: Run

All commands below run from the unpacked CTS directory, which must also be the working directory —
`deqp-vk.exe` resolves the Slang DLLs, `test-server.exe`, and the temporary `test.slang.*` file
relative to it.

Sanity check that the device comes up (this is the nightly's `dump device info` step):

```
.\deqp-vk.exe --deqp-case=dEQP-VK.info.device
type TestResults.qpa
```

Run the full nightly case list:

```
.\deqp-vk.exe --deqp-archive-dir=. ^
              --deqp-caselist-file=slang-passing-tests.txt ^
              --deqp-waiver-file=slang-waiver-tests.xml
```

`deqp-vk.exe` exits non-zero if any non-waived case fails, and writes the details to
`TestResults.qpa`. The full list takes hours; for triage, narrow it down.

Run just the one failing case. This is the copy-and-pasteable command for our example — run it from
the unpacked CTS directory in PowerShell or `cmd`:

```
.\deqp-vk.exe --deqp-archive-dir=. --deqp-case=dEQP-VK.descriptor_indexing.storage_image_minNonUniform --deqp-log-filename=minNonUniform.qpa --deqp-log-shader-sources=enable --deqp-log-decompiled-spirv=enable --deqp-shadercache=disable --deqp-terminate-on-fail=enable
```

The extra flags are what make the result useful for triage: the shader cache is off so Slang is
really re-invoked, the GLSL going in and the SPIR-V coming out are both recorded in
`minNonUniform.qpa`, and the run stops at the failure so the temporary `test.slang.*` file is left
holding the shader that broke.

Wildcards work too, which is handy for running the whole neighborhood of a failure:

```
.\deqp-vk.exe --deqp-archive-dir=. --deqp-case="dEQP-VK.descriptor_indexing.*minNonUniform"
```

For a handful of failures from one nightly, put the case names one per line in a file and pass it
via `--deqp-caselist-file` instead.

Useful flags when triaging:

| Flag                                 | Why                                                                           |
| ------------------------------------ | ----------------------------------------------------------------------------- |
| `--deqp-log-filename=<file>`         | Write results somewhere other than `TestResults.qpa`.                         |
| `--deqp-log-shader-sources=enable`   | Record the GLSL source of every shader in the log — the input Slang sees.     |
| `--deqp-log-decompiled-spirv=enable` | Record the disassembled SPIR-V, i.e. what Slang produced.                     |
| `--deqp-terminate-on-fail=enable`    | Stop at the first failure, leaving `test.slang.*` holding the failing shader. |
| `--deqp-shadercache=disable`         | Bypass the shader cache so every run really re-invokes Slang.                 |
| `--deqp-spirv-validation=enable`     | Validate the SPIR-V that Slang produced.                                      |

## Step 5: Triage a failure

Read the `.qpa` log first — `minNonUniform.qpa` for the command above; the nightly dumps the last
1000 lines of its `TestResults.qpa` on failure. It holds the per-case verdict and, with the logging
flags above, the shader source and the compiler's diagnostic output (the glue prints Slang
diagnostics with a `SLANG:` prefix). The verdict tells you which kind of failure this is: a compile
error or `Failed to compile` from the glue means Slang rejected the GLSL, while a `Fail` with an
image or buffer comparison mismatch means Slang produced SPIR-V that is accepted but wrong.

Before digging into Slang, make two comparisons:

- Re-run the same case with `DISABLE_CTS_SLANG=1` to compile with glslang instead. If
  `storage_image_minNonUniform` still fails, the bug is not in Slang — it is a driver, environment,
  or CTS issue.

  ```
  set DISABLE_CTS_SLANG=1
  .\deqp-vk.exe --deqp-archive-dir=. --deqp-case=dEQP-VK.descriptor_indexing.storage_image_minNonUniform
  set DISABLE_CTS_SLANG=
  ```

- Set `DISABLE_CTS_SLANG_SERVER_MODE=1` to compile in-process through `slang-compiler.dll` instead of
  spawning `test-server.exe`. Slang then runs inside `deqp-vk.exe`, so you can attach a debugger and
  break on the failing compile. The tradeoff is that a Slang crash takes down the whole run.

Then reduce the failure to a plain `slangc` invocation. The run above left `test.slang.<ext>` in the
CTS directory holding the GLSL of the shader that failed — `test.slang.comp` for
`storage_image_minNonUniform`, which is a compute-stage case (`vktDescriptorSetsIndexingTests.cpp`
gives the storage-image variants `VK_SHADER_STAGE_COMPUTE_BIT`, while the other `minNonUniform`
cases are vertex+fragment). You can also copy the source out of the `.qpa` log. Compile it with
exactly the options the glue passes:

```
slangc.exe test.slang.comp -target spirv -stage compute -entry main -allow-glsl -matrix-layout-row-major -o out.spv
```

Stage names map as `.vert` → `vertex`, `.frag` → `fragment`, `.geom` → `geometry`, `.comp` →
`compute`; those four stages are the only ones the glue routes to Slang.

At this point you have a standalone repro and it is an ordinary Slang bug. Set
`SLANG_RUN_SPIRV_VALIDATION=1` to have `slangc` validate its own output, use `-target spirv-asm` to
read the generated SPIR-V — for `minNonUniform`-style cases, check where `NonUniform` decorations
land on the image access — and see [docs/debugging.md](debugging.md) for `-dump-ir` and
`insttrace.py`. Reduce the GLSL by hand until you have a small case, and add it as a regression test
under `tests/`.

## Appendix: building deqp-vk from source

**Skip this entire section if you downloaded the release package in Step 3.** That zip already
contains a Slang-enabled `deqp-vk.exe` built from this same source, it is exactly what the nightly
runs, and it is what you should reproduce a nightly failure against — building the CTS yourself
changes nothing about the reproduction and costs a long dependency fetch and build.

Build the CTS from source only when the failure is on the CTS side rather than the Slang side: you
want to step through the glue in `vkShaderToSpirV_slang.cpp`, add logging to it, or try a change to
the fork before proposing it.

This needs the full checkout, so widen the sparse checkout from Step 2 first (or clone without
`--sparse`):

```bash
git.exe -C external/vk-gl-cts sparse-checkout disable
```

Then follow the fork's README:

```bash
cd external/vk-gl-cts
python.exe external/fetch_sources.py
cmake.exe -S . -B build
cmake.exe --build build --target deqp-vk --config Release
```

`fetch_sources.py` pulls in the external dependencies (glslang, SPIRV-Tools, and friends); it needs
network access and takes a while. The resulting binary is
`build/external/vulkancts/modules/vulkan/Release/deqp-vk.exe`. Build with `--config Debug` instead if
you want to debug the glue, but then pair it with a Debug Slang build so the two agree on the CRT.

To run it, copy `slang-compiler.dll`, `slang-glslang.dll`, `slang-glsl-module.dll`, and
`test-server.exe` next to `deqp-vk.exe` exactly as in Step 3, and point `--deqp-archive-dir` at the
checkout's `data` directory so the tests can find their resource files:

```
.\deqp-vk.exe --deqp-archive-dir=<path to external\vk-gl-cts\data> --deqp-case=dEQP-VK.descriptor_indexing.storage_image_minNonUniform
```

Note that the CTS build does not link Slang in; `external/slang/CMakeLists.txt` in the fork is
effectively empty and the glue loads `slang-compiler.dll` at run time. So a rebuilt `deqp-vk.exe`
picks up whichever Slang binaries you drop beside it, the same as the released one.

## Updating the pinned CTS version

The workflow hardcodes the release asset name (`VK-GL-CTS_WithSlang-<version>-win64.zip`) in several
places: the `release-downloader` step, the `Expand-Archive` call, and every path in the `vkcts setup`,
`dump device info`, and `vkcts run` steps. When the fork publishes a new release, update all of them
together — see commit `20592cc5f` ("ci: update CTS nightly to VK-GL-CTS 0.0.9") for the shape of that
change. The passing-test and waiver lists are checked out from the fork's `main` branch, not from the
release, so they track the fork automatically.
