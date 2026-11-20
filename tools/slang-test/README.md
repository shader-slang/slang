# Slang Test

Slang Test (`slang-test`) is a command-line tool that coordinates and runs the Slang test suite. It acts as a test runner hub, executing various types of tests and collecting their results.

## Basic Usage

```bash
slang-test [options] [test-prefix...]
```

If no test prefix is specified, all tests will be run. Test prefixes can be used to filter which tests to run, and include the path with directories separated by '/'.

Example:

```bash
slang-test -bindir path/to/bin -category full tests/compute/array-param
```

## Command Line Options

### Core Options

- `-h, --help`: Show help message
- `-bindir <path>`: Set directory for binaries (default: the path to the slang-test executable)
- `-test-dir <path>`: Set directory for test files (default: tests/)
- `-v`: Enable verbose output
- `-verbose-paths`: Use verbose paths in output
- `-hide-ignored`: Hide results from ignored tests

### Test Selection and Categories

- `-category <name>`: Only run tests in specified category
- `-exclude <name>`: Exclude tests in specified category

Available test categories:

- `full`: All tests
- `quick`: Quick tests
- `smoke`: Basic smoke tests
- `render`: Rendering-related tests
- `compute`: Compute shader tests
- `vulkan`: Vulkan-specific tests
- `compatibility-issue`: Tests for compatibility issues

A test may be in one or more categories. The categories are specified on top of a test, for example: //TEST(smoke,compute):COMPARE_COMPUTE:

### API Control Options

- `-api <expr>`: Enable specific APIs (e.g., 'vk+dx12' or '+dx11')
- `-api-only`: Only run tests that use specified APIs
- `-compile-test [true|false]`: Run the tests that use no render API, such as `SIMPLE` tests
  (default: true, false with `-api-only`). With `false`, a `COMPILE` line that prepares the input
  of a test that uses an API, and a line that needs NVRTC, still run.
- `-synthesizedTestApi <expr>`: Set APIs for synthesized tests
- `-skip-api-detection`: Skip API availability detection

API expression syntax:

- Use `+` or `-` to add or remove APIs from defaults
- Examples:
  - `vk`: Vulkan only
  - `+vk`: Add Vulkan to defaults
  - `-dx12`: Remove DirectX 12 from defaults
  - `all`: All APIs
  - `all-vk`: All APIs except Vulkan
  - `gl+dx11`: Only OpenGL and DirectX 11

Available APIs:

- OpenGL: `gl`, `ogl`, `opengl`
- Vulkan: `vk`, `vulkan`
- DirectX 12: `dx12`, `d3d12`
- DirectX 11: `dx11`, `d3d11`

### Test Execution Options

- `-server-count <n>`: Set number of test servers (default: 1)
- `-use-shared-library`: Run tests in-process using shared library
- `-use-test-server`: Run tests using test server
- `-use-fully-isolated-test-server`: Run each test in isolated server

### Output Options

- `-appveyor`: Use AppVeyor output format
- `-travis`: Use Travis CI output format
- `-teamcity`: Use TeamCity output format
- `-xunit`: Use xUnit output format
- `-xunit2`: Use xUnit 2 output format
- `-show-adapter-info`: Show detailed adapter information

### Other Options

- `-generate-hlsl-baselines`: Generate HLSL test baselines
- `-emit-spirv-via-glsl`: Emit SPIR-V through GLSL instead of directly
- `-expected-failure-list <file>`: Specify file containing expected failures
- `-skip-list <file>`: Specify file containing tests to skip. Each line is
  either a source-path prefix, or the name of one expanded subtest
  (`<path>.<n>`, e.g. `tests/compute/parameter-block.slang.6`).
- `-exclude-list <file>`: The same as `-skip-list`.
- `-include-list <file>`: Specify file containing the only tests to run, in the
  same format as `-skip-list`. The file must have at least one entry.
- `-exclude-prefix <prefix>`: Exclude tests, with the same two forms as
  `-skip-list`.

Both forms are matched by the same rule: an entry of the form `<path>.<n>`
selects that subtest only — `foo.slang.6` never matches `foo.slang.60` —
and anything else is a path prefix matching every subtest of a matching
file. Subtest exclusion happens _before_ the subtest is dispatched, which is
what makes it usable for a variant that crashes the worker; an
`-expected-failure-list` entry cannot help there, because it reclassifies a
result only after the test returns.

## Test Types

Tests are identified by a special comment at the start of the test file: `//TEST:<type>:`

To ignore a test, use `//DISABLE_TEST` instead of `//TEST`.

Available test types:

- `SIMPLE`: Runs the slangc compiler with specified options after the command
- `REFLECTION`: Runs slang-reflection-test with the options specified after the command
- `COMPARE_COMPUTE`: Runs render-test to execute a compute shader and writes the result to a text file. The test passes if the output matches the expected content
- `COMPARE_COMPUTE_EX`: Same as COMPARE_COMPUTE, but supports additional parameter specifications
- `COMPARE_RENDER_COMPUTE`: Runs render-test with "-slang -gcompute" options and compares text file outputs
- `LANG_SERVER`: Tests Language Server Protocol features by sending requests (like completion, hover, signatures) and comparing responses with expected outputs. See [Language server tests](#language-server-tests)

Deprecated test types (do not create new tests of these kinds, and we need to slowly migrate existing tests to use SIMPLE, COMPARE_COMPUTE(\_EX) or COMPARE_RENDER_COMPUTE instead):

- `COMPARE_HLSL`: Runs the slangc compiler with forced DXBC output and compares with a file having the '.expected' extension
- `COMPARE_HLSL_RENDER`: Runs render-test to generate two images - one using HLSL (expected) and one using Slang, saving both as .png files. The test passes if the images match
- `COMPARE_HLSL_CROSS_COMPILE_RENDER`: Runs render-test to generate two images - one using Slang and one using -glsl-cross. The test passes if the images match
- `COMPARE_HLSL_GLSL_RENDER`: Runs render-test to generate two images - one using -hlsl-rewrite and one using -glsl-rewrite. The test passes if the images match
- `COMPARE_GLSL`: Runs the slangc compiler both through Slang and directly, then compares the SPIR-V assembly output
- `HLSL_COMPUTE`: Runs render-test with "-hlsl-rewrite -compute" options and compares text file outputs
- `CROSS_COMPILE`: Compiles using GLSL pass-through and through Slang, then compares the outputs

### Runtime test with multiple targets

Use `COMPARE_COMPUTE`, `COMPARE_COMPUTE_EX` or `COMPARE_RENDER_COMPUTE` to test the runtime behavior of a shader.

```
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type
```

By default, the test runs for all targets with an API that the machine supports. To specify or limit the targets, use the option `targets=`:

```
//TEST:COMPARE_COMPUTE(targets=all-cuda, filecheck-buffer=CHECK): -any-options-as-needed
```

By default, `targets=all` is implied.

Such a line runs once for each target, like a loop over the targets. The line itself does not run. For each target, `slang-test` adds the arguments of the target to the line and runs it. In the test result, the name of each test has the target in parentheses, for example:

```
tests/compute/my-test.slang.1 (dx11)
tests/compute/my-test.slang.1 (dx12)
tests/compute/my-test.slang.1 (vk)
tests/compute/my-test.slang.1 (glsl)
tests/compute/my-test.slang.1 (wgpu)
...
```

The number in the name is the index of the test line in the file. It is the same for all targets of a line. Line 0 has no number. The number does not depend on the APIs of the machine, so a name is the same on every machine. Use the whole name in a list of expected failures, or in `-exclude-prefix` and `-skip-list`. The entry `tests/compute/my-test.slang.1` selects all targets of the line, and the entry `tests/compute/my-test.slang.1 (vk)` selects one target.

To test with different command-line arguments, add more lines. Each line makes its own tests for each target:

```
//TEST:COMPARE_COMPUTE(targets=all-cuda, filecheck-buffer=CHECK): -Xslang -DTEST_A
//TEST:COMPARE_COMPUTE(targets=all-cuda, filecheck-buffer=CHECK): -Xslang -DTEST_B
```

#### The commands

The three commands differ only in the arguments that they add to the `render-test` command line:

| Command                  | Adds        |
| ------------------------ | ----------- |
| `COMPARE_COMPUTE`        | `-compute`  |
| `COMPARE_COMPUTE_EX`     | nothing     |
| `COMPARE_RENDER_COMPUTE` | `-gcompute` |

Use `COMPARE_COMPUTE` for a compute test. Do not write `-compute` on the line, because the command adds it.

Use `COMPARE_COMPUTE_EX` only for a test that must choose its own mode, for example `-mesh`, `-task` or `-rt`. The command adds no argument, so the line must have the mode argument.

#### Old format: a render API argument

A line of these commands should not choose its target with an argument. The arguments that choose a target are a render API (`-vk`, `-cpu`), a language that implies one (`-wgsl`), and the path of the compiler to the code of the target (`-emit-spirv-directly`, `-emit-spirv-via-glsl`, `-emit-cpu-via-cpp`, `-emit-cpu-via-llvm`). A line like this has the old format. `slang-test` runs it as one test on that target, and prints a warning. The text of the warning names the option to use, for example `targets=glsl` for `-emit-spirv-via-glsl`. For example, the line below gets a warning that asks for `targets=vk`:

```
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type
```

Write the line like this:

```
//TEST:COMPARE_COMPUTE(targets=vk, filecheck-buffer=CHECK): -output-using-type
```

A line that has both `targets=` and an argument that chooses the target is an error, because the two choose the target twice. A line that `DISABLE_TEST` disables does not run, so it gets no warning.

#### The option `targets=`

The value of `targets=` is a list of names, separated by plus signs (`+`). These names are valid:

| Name         | Arguments of `render-test`   | Predefined macro   |
| ------------ | ---------------------------- | ------------------ |
| `vk`         | `-vk -emit-spirv-directly`   | `SLANG_TEST_VK`    |
| `glsl`       | `-vk -emit-spirv-via-glsl`   | `SLANG_TEST_GLSL`  |
| `dx11`       | `-dx11`                      | `SLANG_TEST_DX11`  |
| `dx12`       | `-dx12`                      | `SLANG_TEST_DX12`  |
| `metal`      | `-metal`                     | `SLANG_TEST_METAL` |
| `cuda`       | `-cuda`                      | `SLANG_TEST_CUDA`  |
| `cpu`        | `-cpu`                       | `SLANG_TEST_CPU`   |
| `wgpu`       | `-wgpu`                      | `SLANG_TEST_WGPU`  |
| `llvm`       | `-llvm`                      | `SLANG_TEST_LLVM`  |
| `all`        | every target in this table   |                    |
| `all-<name>` | every target except `<name>` |                    |
| `none`       | no target                    |                    |

Use `all-<name>` for a test that cannot run on a target. For example, `targets=all-vk-cuda` runs on every target except `vk` and `cuda`, and `targets=vk+metal+cpu` runs on these three targets only.

Use `none` for a line that must not run on any target, for example a disabled test that has no target to name. The line makes no test. `none` is the whole value, so `none+vk` is an error. A list that removes every target, such as `all-vk-glsl-dx11-dx12-metal-cuda-cpu-wgpu-llvm`, is also an error, because the author must write `none` to say it.

The result is the union of the tokens. A hyphen between names removes more than one target: `all-vk-cuda` is `all` without `vk` and `cuda`. Only `all-<name>` removes a target. A token such as `-vk` is an unknown name, and `slang-test` reports an error. The tests have the order of the table above, and a target does not run twice.

Follow these rules:

- `slang-test` makes a test only for an API that the machine has, and that the option `-synthesizedTestApi` allows. By default, all APIs are allowed.
- A line runs for all of its targets. Another line of the file does not change this, even if that line tests the same target. For example, `targets=all` with `-option-a` and `targets=cuda` with `-option-b` give two different `cuda` tests.
- A `DISABLE_TEST` line does not switch a target off for another line. Use `targets=all-<name>`.
- Use `+` between the names, and not a comma. A comma separates the options of the test line, so `targets=vk,cpu` is the option `targets=vk` and the option `cpu`. `slang-test` reports an error for this.
- An unknown name is an error. `slang-test` does not skip it.
- Other test commands, such as `SIMPLE`, do not support `targets=`. `slang-test` reports an error for them.

#### Predefined macros

`slang-test` passes one macro definition to the Slang compiler for each test, as `-Xslang -DSLANG_TEST_<TARGET>`. The last column of the table above lists the macro of each target. `<TARGET>` is the name of the target in capital letters. A test defines the macro of its own target and no other macro. A shader uses the macro to pick the code for one target:

```
#if defined(SLANG_TEST_METAL)
// Code for the Metal test only.
#endif
```

`vk` and `glsl` use the same API, but they have two macros, because the Slang compiler takes a different path for each. Every test of the three commands has the macro of its target. The tests of other commands do not have the macro.

#### Other details

The test of a target reads the expected file of its line. For example, the tests of the line at index 1 read `foo.slang.1.expected.txt`. A test writes files that have the target in their name, for example `foo.slang.1.vk.actual.txt`. The tests of the targets of a line do not write the same file.

A `.hlsl` test file needs the option `-slang`, so the test commands add it to the line that they run. A `.slang` file does not need it. For a `.slang` file, the command adds `-slang` only if the line has another language option, such as `-hlsl`. The last language option wins.

### Unused `.expected` files

A test line with the `filecheck=PREFIX` or `diag=PREFIX` option checks its output with FileCheck or with the diagnostic annotations. It never reads a `.expected` file. If such a file exists for the test line, `slang-test` reports an error and the test fails. Remove the file, or remove the option from the test line.

The `.expected` file of test line N is `foo.slang.N.expected`. For line 0 it is `foo.slang.expected`. The error is not reported for line 0 when another test line of the same file uses `foo.slang.expected` as its shared file.

### Exit code of tests

A `SIMPLE`, `INTERPRET` or `REFLECTION` test with the `filecheck=PREFIX` or `diag=PREFIX` option checks the exit code of the tool, in addition to the FileCheck lines or the diagnostic annotations. The test fails if the exit code is not the expected one. The expected exit code is 0 by default. A test that compares the output with an `.expected` file does not need this check, because the file contains the line `result code = N`.

A test that must fail sets the expected exit code with the `exit-code=N` option. N must be `0`, `1` or `2`. The test fails for another value. A test must never expect `3`, because it reports an internal compiler error, which is a bug in the compiler:

```
//TEST:SIMPLE(filecheck=CHECK,exit-code=2): -target spirv -entry main -stage compute
```

The exit codes of `slangc` are in the output of `slangc -help`. The values that a test can expect are:

- `0`: success.
- `1`: the command line is not valid, for example an unknown or malformed option, or the tool failed to run.
- `2`: the compilation failed because of errors in the input, for example an error diagnostic.

A test with an expected failure should also have a `CHECK` line for the diagnostic that it wants to see, for example `// CHECK: error[E30015]`. Without it, the test can pass for an unrelated error. A `CHECK-NOT` line alone cannot detect a failed compilation, because the error output does not contain the text that it rejects. Use the diagnostic code, such as `E36107`, and not only the word `error`. The word `error` is also part of the header `standard error` in the FileCheck input.

The exception is a test with the `diag=PREFIX` option (see [diagnostics](../../docs/diagnostics.md)): its expected exit code is `2` by default, because a diagnostic test usually checks an error. A diagnostic test that has only warnings must say `exit-code=0`:

```
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target spirv
```

A test line cannot have both `diag=PREFIX` and `filecheck=PREFIX`. `slang-test` reports an error and the test fails, because only the `diag=` check runs and the `CHECK` lines of `filecheck=` are never run. The `diag=` check looks at the diagnostics in the standard error, and the `filecheck=` check looks at the whole output. A test that needs both checks uses one test line for each of them. For example, a `-E` test uses `DIAGNOSTIC_TEST:SIMPLE(diag=diag):-E` for the diagnostics and `TEST:SIMPLE(filecheck=CHECK):-E` for the preprocessed text in the standard output.

### Validating emitted code with the downstream compiler

A `SIMPLE` test with a text target, such as `-target metal`, checks only the text that `slangc` writes. It passes even if the downstream compiler rejects that text. To check the text, add `-validate-with-downstream-compiler` to the `slangc` options of the test:

```
//TEST:SIMPLE(filecheck=CHECK): -target metal -entry main -stage compute -validate-with-downstream-compiler
```

With this option, `slangc` runs the downstream compiler or validator for the target on the emitted code, and returns an exit code other than 0 when there is a problem. The tools are:

| Target               | Downstream tool                                                   |
| -------------------- | ----------------------------------------------------------------- |
| `spirv`, `spirv-asm` | SPIR-V validator (also runs without `SLANG_RUN_SPIRV_VALIDATION`) |
| `metal`              | Metal compiler                                                    |
| `hlsl`               | DXC for shader model 6.0 or higher, FXC below that                |
| `glsl`               | glslang                                                           |
| `wgsl`               | Tint                                                              |
| `cuda`               | NVRTC                                                             |
| `cpp`                | C++ compiler                                                      |

Not every machine has every tool. For example, there is no Metal compiler on Linux. `slang-test` checks which tools the machine has when it starts. For a `SIMPLE` test, `slang-test` removes `-validate-with-downstream-compiler` from the `slangc` command line when the tool for the `-target` of the test is not available. At startup, `slang-test` prints a warning for each target whose validation is removed in this way, for example:

```
warning: Metal compiler not found; -validate-with-downstream-compiler is removed from SIMPLE tests with -target metal
```

Only `SIMPLE` tests do this. A test of another type that passes the option to `slangc` gets the error from `slangc` when the tool is not available. Use the option only in tests that must produce valid output. A test that expects a failed compile does not use the option; it uses `exit-code=N` instead.

## Language server tests

A `LANG_SERVER` test opens the test file in `slangd` and runs the `//` directives in the file from top to bottom. Every `LANG_SERVER` test shares one `slangd` process. `initialize` replaces the workspace, but formatting, inlay-hint, commit-character and trace options carry over, so a test that changes one of them sets it back to its default before it ends.

Request directives print a `--------` separator followed by the result:

- `//COMPLETE:<line>,<col>`, `//SIGNATURE:<line>,<col>`, `//HOVER:<line>,<col>` (1-based) print the completion items, signature help or hover, or `null`.
- `//INLAY` prints every inlay hint in the document as `<line>,<col> <label>`.
- `//ON_TYPE_FORMAT:<line>,<col>` prints whether on-type formatting is `enabled` or `disabled`.
- `//DIAGNOSTICS` prints the diagnostics published for the document.

Configuration directives:

- `//CONFIG:<key>=<json>` sends `workspace/didChangeConfiguration` with one flat key, for example `//CONFIG:slang.inlayHints.deducedTypes=true` or `=null`, and prints `config: <key>=<json>`.
- `//CONFIG_REPULL` sends `workspace/didChangeConfiguration` with `settings: null`, which makes `slangd` request its settings again, and answers that request from the `//CONFIG_REPLY:` lines.
- `//CONFIG_REPLY:<section>=<json>` declares the value the test client returns for one section when `slangd` sends `workspace/configuration`. These lines are collected before the test runs, so their position does not matter. Every other section is answered with JSON null, which is what LSP requires from a client that cannot provide a setting.

Options in the `LANG_SERVER(...)` list choose the `initialize` handshake. `config-pull` sends `initialized` after `initialize` and answers the configuration request that follows. It is required by `//CONFIG_REPLY:` and `//CONFIG_REPULL`.

## Compiler Optimization in Tests

`slang-test` adds `-O0` when it builds a Slang compiler command line and the
test directive does not already specify an optimization option. This keeps
ordinary test runs fast and avoids expected-output churn from optimizer changes.

Pass `-OX`, where X is between 0 and 3, to `slang-test` to change that default for
a selected test run. For example, this compiles tests that do not specify their own
level with `-O2`:

```text
slang-test tests/render*/smo* -O2
```

Compiler-backed tests, such as `SIMPLE`, `CROSS_COMPILE`, and diagnostic tests,
can opt in to optimized output with a normal slangc option:

```text
//TEST:SIMPLE: -target spirv -O3
```

Render-test-backed tests, such as `COMPARE_COMPUTE_EX`, must forward the
optimization option to slangc:

```text
//TEST(compute):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -Xslang -O3
```

Besides `-Xslang <option>`, the forwarding forms `-compile-arg <option>`,
`-xslang <option>`, and a `-Xslang... <options...> -X.` block are also
recognized. Prefer the `-Xslang <option>` form shown above for new tests.

Metal render tests are the one exception to the `-O0` default: they receive
`-Xslang -O1` instead, because the downstream `metal` toolchain that produces
the metallib is unstable at `-O0` on macOS CI. The generated MSL source is
identical at every level, so this only affects the downstream compilation.

The recognized optimization spellings are `-O`, `-O0`, `-Onone`, `-O1`,
`-Odefault`, `-O2`, `-Ohigh`, `-O3`, and `-Omaximal`. Prefer the lowest level
that preserves the test's expected output.

`slang-test` inserts its default at the front of the argument list for both
compiler-backed and render-test-backed commands, so directive arguments keep
their meaning — in particular, a directive that leaves a trailing option
without its required argument (intentionally in a diagnostic test, or by
mistake) cannot consume the inserted default.

## Unit Tests

In addition to the above test tools, there are also `slang-unit-test-tool` and `gfx-unit-test-tool`, which are invoked as in the following examples; but note that the unit tests do get run as part of `slang-test` as well.

To ignore a unit test, use the `SLANG_IGNORE_TEST` macro:

```cpp
SLANG_UNIT_TEST(foo)
{
    if (condition)
    {
        SLANG_IGNORE_TEST
    }

    // ...
}
```

### slang-unit-test-tool

```bash
# Regular unit tests
slang-test slang-unit-test-tool/<test-name>
# e.g. run the `byteEncode` test.
slang-test slang-unit-test-tool/byteEncode
```

These tests are located in the [tools/slang-unit-test](https://github.com/shader-slang/slang/tree/master/tools/slang-unit-test) directory, and defined with macros like `SLANG_UNIT_TEST(byteEncode)`.

### gfx-unit-test-tool

```bash
# Graphics unit tests
slang-test gfx-unit-test-tool/<test-name>

# e.g. run the `precompiledTargetModule2Vulkan` test.
slang-test gfx-unit-test-tool/precompiledTargetModule2Vulkan
```

These tests are located in [tools/gfx-unit-test](https://github.com/shader-slang/slang/tree/master/tools/gfx-unit-test), and likewise defined using macros like `SLANG_UNIT_TEST(precompiledTargetModule2Vulkan)`.
