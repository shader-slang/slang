# Test generators

This directory has scripts that generate Slang tests. A generated test lives under `tests/generated/`. Do not edit a generated test by hand. The next run of the script overwrites your edit.

This page describes one script: `generate-tests-capabilities-texture-types.py`. The section "CI check" at the end applies to every script in this directory.

## generate-tests-capabilities-texture-types.py

The script writes the capability tests for the texture types. For each backend and each texture type, it writes one `.slang` file.

```bash
extras/test-generators/generate-tests-capabilities-texture-types.py
```

The script takes no arguments. You can run it from any directory.

| Item              | Value                                         |
| ----------------- | --------------------------------------------- |
| Output directory  | `tests/generated/capabilities/texture/types/` |
| File name         | `gen-types-<backend>-<texture-type>.slang`    |
| Backends          | `wgsl`, `metal`, `glsl-vk`                    |
| Files per backend | One per texture type (37)                     |

The file name uses the texture type in lower case, without the template arguments. The type `RasterizerOrderedTexture3D<float4>` on the `glsl-vk` backend gives `gen-types-glsl-vk-rasterizerorderedtexture3d.slang`.

The goal of the tests is in `tests/generated/capabilities/texture/types/README.md`. In short, each test checks that a texture type compiles on the minimum backend version, and fails to compile on the version before it.

## The template

Every generated file comes from one template. The template is the Python f-string `testStr` in `generateSingleTest()`. All files have the same text. Only the marked parts change.

In the template below, a marker like `<<NAME>>` is a part that the script replaces. Each marker has a letter in the table after the template. Text without a marker is fixed.

```text
// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-tests-capabilities-texture-types.py, and
// regenerate this test by running:
//
//     extras/test-generators/generate-tests-capabilities-texture-types.py
//
// Texture types capability test: <<BACKEND>> / <<TEXTURE_TYPE>>
// - Type supported since target version:  <<MIN_VERSION>>
// - Target version for positive test:     <<POSITIVE_VERSION>>
// - Target version for negative test:     <<NEGATIVE_VERSION>>
//

<<DISABLE_COMMENTS>>
//<<DISABLE_POS>>TEST:SIMPLE(filecheck=POSITIVE): -restrictive-capability-check -entry fragMain -stage fragment -target <<POSITIVE_TARGET>>
//<<DISABLE_CMP>>TEST:COMPARE_COMPUTE(filecheck-buffer=POSITIVE_RESULT): <<COMPUTE_TARGET>>
//<<DISABLE_NEG>>TEST:SIMPLE(diag=NEGATIVE,non-exhaustive): -restrictive-capability-check -entry fragMain -stage fragment -target <<NEGATIVE_TARGET>>

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT: <<TEXTURE_INPUT>>:name texHandle
<<TEXTURE_TYPE>> texHandle;
<<EXTRA_INPUT>>
[shader("fragment")]
void fragMain()
{
// POSITIVE-NOT: {{(error|warning).*}}:
// POSITIVE-LABEL: <<LABEL>>
// POSITIVE-NOT: {{(error|warning).*}}:

//NEGATIVE: error

    int ret = 0;
    <<OP_STATEMENTS>>
    outputBuffer[0] = 0x12345 + ret;
}

[numthreads(4, 1, 1)]
void computeMain()
{
    int ret = 0;
    <<OP_STATEMENTS>>
    outputBuffer[0] = 0x12345 + ret;
}
// POSITIVE_RESULT: 12346
```

### What each marker becomes

| Marker                                                  | Replaced with                                                                                                            | Source in the script                  |
| ------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ | ------------------------------------- |
| `<<BACKEND>>`                                           | The backend name, for example `glsl-vk`.                                                                                 | `backend["name"]`                     |
| `<<TEXTURE_TYPE>>`                                      | The Slang texture type, for example `RWTexture2D<float4>`. It appears twice: in the header and in the declaration.       | Key of the `tests` table              |
| `<<MIN_VERSION>>`                                       | The first backend version that supports the type. `None` means no version supports it.                                   | `TestInfo.minVersion`                 |
| `<<POSITIVE_VERSION>>`                                  | The version that the positive test uses. This is the lowest version that supports the type.                              | `getPositiveNegativeBackendTargets()` |
| `<<NEGATIVE_VERSION>>`                                  | The version that the negative test uses. This is the highest version below the minimum that Slang supports.              | `getPositiveNegativeBackendTargets()` |
| `<<DISABLE_COMMENTS>>`                                  | Zero or more comment lines with an issue link, one for each disabled test. Empty when no test is disabled.               | `TestInfo` disable flags              |
| `<<DISABLE_POS>>`, `<<DISABLE_CMP>>`, `<<DISABLE_NEG>>` | `DISABLE_` or nothing. `DISABLE_` turns the line into a comment that `slang-test` ignores.                               | See "When a test is disabled"         |
| `<<POSITIVE_TARGET>>`                                   | The compiler options for the positive test, for example `glsl -profile glsl_450`.                                        | Second item of the version tuple      |
| `<<COMPUTE_TARGET>>`                                    | The `slang-test` options for the compute test, for example `-vk -emit-spirv-via-glsl`.                                   | Third item of the version tuple       |
| `<<NEGATIVE_TARGET>>`                                   | The compiler options for the negative test.                                                                              | Second item of the version tuple      |
| `<<TEXTURE_INPUT>>`                                     | The `TEST_INPUT` description of the texture, for example `Texture3D(size=4, content = one)`.                             | `testInputForTestType`                |
| `<<EXTRA_INPUT>>`                                       | Extra resources that some operations need. Empty for most operations.                                                    | See "Extra resources"                 |
| `<<LABEL>>`                                             | The label that `POSITIVE-LABEL` looks for in the compiler output. `main` on `glsl-vk`. `fragMain` on the other backends. | `backend["positive-label"]`           |
| `<<OP_STATEMENTS>>`                                     | The statements that use the texture. It appears twice: in `fragMain` and in `computeMain`.                               | `getStatementsForTestOp()`            |

### What the fixed text does

The three `TEST` lines are the three tests in each file.

| Test       | Directive                                   | What it checks                                                                               |
| ---------- | ------------------------------------------- | -------------------------------------------------------------------------------------------- |
| Positive   | `TEST:SIMPLE(filecheck=POSITIVE)`           | `fragMain` compiles on the minimum version without an error or a warning (exit code `0`).    |
| Functional | `TEST:COMPARE_COMPUTE`                      | `computeMain` runs on the real backend. The buffer holds `12346`. This test needs a GPU.     |
| Negative   | `TEST:SIMPLE(diag=NEGATIVE,non-exhaustive)` | `fragMain` fails to compile (exit code `-1`) with an error on the version below the minimum. |

The `POSITIVE` check lines are FileCheck lines. They are inside the body of `fragMain`. The `-restrictive-capability-check` option makes the compiler report a missing capability as an error.

The negative test is a diagnostic test. The line `//NEGATIVE: error` is an annotation that expects at least one diagnostic with the severity `error`. The `non-exhaustive` option lets the other diagnostics of the compiler stay without annotation. An error always comes with notes, for example "see call to ...", and these notes depend on the backend and the texture type. A test with `non-exhaustive` fails if all the diagnostics have an annotation. This cannot happen here, because every error has at least one note.

The functional test expects `12346`, which is `0x12345 + 1`. The texture content is `one`, so each operation that runs in the functional test makes `ret` equal to `1`.

## The tables that fill the markers

The script keeps all the variable data in a few tables.

### Backend description

Each backend function (`getWgslTests()`, `getMetalTests()`, `getGlslTests()`) returns a backend dictionary and a `tests` table.

```python
backend = {
    "name" : "metal",                  # <<BACKEND>> and the file name
    "positive-label" : "main",         # <<LABEL>> (optional, default "fragMain")
    "versions" : [                     # in ascending order
        ("1.0", None, None),           # not supported by Slang: no test target
        ("2.3", "metal -profile metallib_2_3", "-mtl"),
        #  ^ version   ^ <<*_TARGET>> for SIMPLE tests   ^ <<COMPUTE_TARGET>>
    ],
}
```

A version with `None` targets is a version that Slang does not support. The script cannot compile for it, so it never uses it as a test target.

### Test table

The `tests` table has one line for each texture type. The value is a `TestInfo`.

```python
"RWTexture2D<float4>" : TestInfo("420", 12).disableNegativeTest(8908),
#  texture type          ^ minVersion ^ testOp   ^ optional disable flags
```

- `minVersion` is a version from the `versions` list of the backend. `None` means the backend does not support the type.
- `testOp` is the number of the operation. It selects `<<OP_STATEMENTS>>` and `<<EXTRA_INPUT>>`.
- The disable flags are described in the next section.

### Positive and negative versions

`getPositiveNegativeBackendTargets()` walks the `versions` list and skips the versions that Slang does not support.

- The positive version is the first supported version that is at least `minVersion`.
- The negative version is the last supported version before `minVersion`.
- If no version qualifies, the result is `None`. The marker then shows `None`.

For `RasterizerOrderedTexture3D<float4>` on `glsl-vk`, `minVersion` is `450`. The positive version is `450` and the negative version is `440`.

### Operations

The operation number selects the code that uses the texture. The full list is in `getStatementsForTestOp()`.

| Operation | Statement pattern                                              |
| --------- | -------------------------------------------------------------- |
| 1 to 4    | `SampleLevel` with a 1 to 4 dimensional coordinate             |
| 11 to 14  | `Load` with a 1 to 4 dimensional coordinate                    |
| 15, 16    | `Load` with a 2 or 3 dimensional coordinate and a sample index |
| 21 to 23  | `Store` with a 1 to 3 dimensional coordinate                   |
| 32, 33    | `WriteSamplerFeedbackLevel`                                    |
| 41 to 44  | `SampleCmpLevelZero` with a 1 to 4 dimensional coordinate      |

An operation can have one statement or a list of statements. The script puts each statement on its own line, with four spaces of indent.

### Extra resources

Some operations need a resource in addition to the texture. The script adds it to `<<EXTRA_INPUT>>`.

| Operations     | Added declaration                                                                        |
| -------------- | ---------------------------------------------------------------------------------------- |
| 1 to 4, 32, 33 | `SamplerState samplerState` with `TEST_INPUT: Sampler(filteringMode=point)`              |
| 41 to 44       | `SamplerComparisonState samplerComparisonState` with `TEST_INPUT: Sampler(depthCompare)` |
| 32             | `Texture2D<float4> feedbackSamplerInput2D`                                               |
| 33             | `Texture2DArray<float4> feedbackSamplerInput2DArray`                                     |

## When a test is disabled

A disabled test stays in the file. Its `TEST` line becomes `//DISABLE_TEST...`, so `slang-test` ignores it. The file keeps the line as a record of the intended test.

The script sets the three `<<DISABLE_*>>` markers by these rules.

| Condition                                                           | Positive | Compute  | Negative |
| ------------------------------------------------------------------- | -------- | -------- | -------- |
| The backend does not support the type (`minVersion` is `None`)      | Disabled | Disabled | Enabled  |
| No version below `minVersion` exists (there is no negative version) | Enabled  | Enabled  | Disabled |
| `.bug(issue)`                                                       | Disabled | Disabled | Disabled |
| `.disableComputeTest(issue)`                                        | Enabled  | Disabled | Enabled  |
| `.disableNegativeTest(issue)`                                       | Enabled  | Enabled  | Disabled |

Each row applies on its own. The results add up. For a disabled test, the target marker shows `None` when the script has no target for it.

Each `.bug()`, `.disableComputeTest()` and `.disableNegativeTest()` call also adds a comment to `<<DISABLE_COMMENTS>>`:

```text
// Compute test disabled, see https://github.com/shader-slang/slang/issues/8787
// Negative test disabled, see https://github.com/shader-slang/slang/issues/8908
```

Always give an issue number when you disable a test.

## Example

This is the entry in the `glsl-vk` table:

```python
"RasterizerOrderedTexture3D<float4>" : TestInfo("450", 23).disableComputeTest(8787).disableNegativeTest(8908),
```

The markers get these values:

| Marker                                    | Value                                                                |
| ----------------------------------------- | -------------------------------------------------------------------- |
| `<<BACKEND>>`                             | `glsl-vk`                                                            |
| `<<TEXTURE_TYPE>>`                        | `RasterizerOrderedTexture3D<float4>`                                 |
| `<<MIN_VERSION>>`, `<<POSITIVE_VERSION>>` | `450`                                                                |
| `<<NEGATIVE_VERSION>>`                    | `440`                                                                |
| `<<DISABLE_COMMENTS>>`                    | The two comment lines from the previous section                      |
| `<<DISABLE_POS>>`                         | Nothing                                                              |
| `<<DISABLE_CMP>>`, `<<DISABLE_NEG>>`      | `DISABLE_`                                                           |
| `<<POSITIVE_TARGET>>`                     | `glsl -profile glsl_450`                                             |
| `<<COMPUTE_TARGET>>`                      | `-vk -emit-spirv-via-glsl`                                           |
| `<<NEGATIVE_TARGET>>`                     | `glsl -profile glsl_440`                                             |
| `<<TEXTURE_INPUT>>`                       | `Texture3D(size=4, content = one)`                                   |
| `<<EXTRA_INPUT>>`                         | Empty (operation 23 needs no extra resource)                         |
| `<<LABEL>>`                               | `main`                                                               |
| `<<OP_STATEMENTS>>`                       | `texHandle.Store(int3(0, 0, 0), float4(0, 0, 0, 0));` and `ret = 1;` |

The result is `tests/generated/capabilities/texture/types/gen-types-glsl-vk-rasterizerorderedtexture3d.slang`. Compare it with the template to see each replaced part.

## How to change the tests

Change the script, then run it. Do not edit the generated files.

| Task                                 | What to change                                                                                                                                                                            |
| ------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Change the text that all tests share | The `testStr` template in `generateSingleTest()`                                                                                                                                          |
| Change the minimum version of a type | The `TestInfo` of the type in the table of the backend                                                                                                                                    |
| Disable a test for a known bug       | Add `.bug()`, `.disableComputeTest()` or `.disableNegativeTest()` to the `TestInfo`                                                                                                       |
| Add a texture type                   | Add it to `testInputForTestType`. Then add a `TestInfo` for it in every backend table.                                                                                                    |
| Add a backend version                | Add a tuple to the `versions` list of the backend, in ascending order                                                                                                                     |
| Add a texture operation              | Add an entry to `opToStatements` in `getStatementsForTestOp()`. Add its resources to `generateSingleTest()` if needed. Update the operation list in the comment at the top of the script. |
| Add a backend                        | Write a new `get<Backend>Tests()` function. Call `generateTests()` for it in `main()`.                                                                                                    |

After a change, run the script and commit the changed generated files together with the script.

## CI check

The workflow `.github/workflows/check-generated-tests.yml` runs on every pull request and in the merge queue. It keeps the generated tests in sync with the scripts.

The workflow does these steps:

1. It runs every script that matches `extras/test-generators/generate-*.py`.
2. It checks that `git status` shows no change under `tests/generated/`.
3. It fails when a generated file is different, missing or new. The log shows the diff.

To get the same result on your machine, run the scripts from the repository root and look at the status:

```bash
for generator in extras/test-generators/generate-*.py; do python3 "$generator"; done
git status -- tests/generated
```

The check fails in two cases:

- You edited a generated file by hand. Make the same change in the script, then run the script again.
- You changed a script and did not commit the new output. Run the script and commit the changed files.

A new script must have a name that matches `generate-*.py`. It must write only below `tests/generated/`. It must not need arguments or other tools than Python 3.
