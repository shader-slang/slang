# Slang Test

Slang Test (`slang-test`) is the command-line runner for the Slang test suite. It scans
test files for `//TEST` directives, expands each directive into one or more subtests,
dispatches the matching test tool, and compares the produced output with expected output.

## Basic Usage

```bash
slang-test [options] [test-prefix...]
```

If no test prefix is specified, all tests will be run. Test prefixes filter the selected
tests by path, with directories separated by `/`.

Example:

```bash
slang-test -bindir path/to/bin -category full tests/compute/array-param
```

## Command Line Options

### Core Options

- `-h, --help`: Show help message
- `-bindir <path>`: Set directory for binaries (default: the path to the `slang-test`
  executable)
- `-test-dir <path>`: Set directory for test files (default: `tests/`)
- `-v [level]`: Set verbosity level. Without `-v`, the default is `info`; `-v` alone
  selects `verbose`; `-v <level>` selects the named level (`verbose`, `info`, or
  `failure`). The levels are ordered from most to least output as `verbose`, `info`,
  then `failure`.
- `-verbose-paths`: Use verbose paths in output
- `-hide-ignored`: Hide results from ignored tests

### Test Selection and Categories

- `-category <name>`: Only run tests in the specified category
- `-exclude <name>`: Exclude tests in the specified category
- `-exclude-prefix <prefix>`: Exclude tests whose path prefix or expanded subtest name
  matches the prefix

Available test categories include:

- `full`: All tests
- `quick`: Quick tests
- `smoke`: Basic smoke tests
- `render`: Rendering-related tests
- `compute`: Compute shader tests
- `vulkan`: Vulkan-specific tests
- `compatibility-issue`: Tests for compatibility issues

A test may be in one or more categories. The categories are written after `TEST` and before
the command name:

```text
//TEST(smoke,compute):COMPARE_COMPUTE:
```

Additional categories may appear in the tree. Common examples include `unit-test`, `cuda`,
`optix`, `wave`, `wave-mask`, `wave-active`, `windows`, `unix`, `64-bit`, and
`shared-library`.

Category filters are supported because existing automation and local workflows still use
them, but they are not the preferred way to debug an individual test. Prefer passing a
specific test path, or excluding a known-bad subtest with `-exclude-prefix`, when narrowing a
local run.

### API Control Options

- `-api <expr>`: Enable specific APIs, for example `vk+dx12` or `+dx11`
- `-api-only`: Only run tests that use the APIs selected by `-api`
- `-synthesizedTestApi <expr>`: Set APIs used for synthesized tests
- `-skip-api-detection`: Skip API availability detection

API expression syntax:

- Use `+` or `-` to add or remove APIs from the default set
- Examples:
  - `vk`: Vulkan only
  - `+vk`: Add Vulkan to defaults
  - `-dx12`: Remove DirectX 12 from defaults
  - `all`: All APIs
  - `all-vk`: All APIs except Vulkan
  - `vk+dx11`: Vulkan and DirectX 11

Available API names:

- Vulkan: `vk`, `vulkan`
- DirectX 12: `dx12`, `d3d12`
- DirectX 11: `dx11`, `d3d11`
- Metal: `mtl`, `metal`
- CPU: `cpu`
- CUDA: `cuda`
- WebGPU: `wgpu`, `webgpu`
- LLVM: `llvm`

API filters are supported for compatibility with existing test selection flows, but they are
not the preferred way to debug backend behavior. Prefer a targeted test directive or direct
tool arguments that name the backend/API being exercised.

### Test Execution Options

- `-server-count <n>`: Set number of test servers (default: 1)
- `-use-shared-library`: Run tests in-process using the shared library
- `-use-test-server`: Run tests through the test server
- `-use-fully-isolated-test-server`: Run each test in an isolated test server

### Output Options

- `-appveyor`: Use AppVeyor output format
- `-travis`: Use Travis CI output format
- `-teamcity`: Use TeamCity output format
- `-xunit`: Use xUnit output format
- `-xunit2`: Use xUnit 2 output format
- `-show-adapter-info`: Show detailed adapter information

### Other Options

- `-generate-hlsl-baselines`: Generate HLSL test baselines
- `-skip-reference-image-generation`: Skip generating reference images for render tests
- `-emit-spirv-via-glsl`: Emit SPIR-V through GLSL instead of directly
- `-expected-failure-list <file>`: Specify file containing expected failures
- `-skip-list <file>`: Specify file containing tests to skip. Each line is either a
  source-path prefix, or the name of one expanded subtest (`<path>.<n>`, for example
  `tests/compute/parameter-block.slang.6`)
- `-capability <name>`: Compile with the given capability
- `-shuffle-tests`: Shuffle tests in directories
- `-shuffle-seed <seed>`: Set shuffle seed (default: 1)
- `-enable-debug-layers [true|false]`: Enable or disable Validation Layer for Vulkan and
  Debug Device for DirectX
- `-cache-rhi-device [true|false]`: Enable or disable RHI device caching (default: true)

Abort-dialog suppression is not a `slang-test` command-line option. On Windows, configure the
build with the `SLANG_IGNORE_ABORT_MSG` CMake option and use the `SLANG_ASSERT` environment
variable to control assertion behavior.

`-skip-list` and `-exclude-prefix` use the same matching rule: an entry of the form
`<path>.<n>` selects exactly that expanded subtest, so `foo.slang.6` never matches
`foo.slang.60`. Any other entry is a path prefix matching every subtest of a matching file.
Subtest exclusion happens before the subtest is dispatched, which makes it usable for a
variant that crashes the worker. An `-expected-failure-list` entry cannot help in that case,
because it only reclassifies a result after the test returns.

## Test Directives

Tests are identified by special comments at the start of a test file. The general form is:

```text
//TEST(<categories>):<command>(<command-options>): <tool-arguments>
//DIAGNOSTIC_TEST(<categories>):<command>(<command-options>): <tool-arguments>
```

Both the category list and command option list are optional. The final colon before
`<tool-arguments>` is required, even when there are no arguments.

Examples:

```text
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv
```

`slang-test` recognizes a disabled form by applying the `DISABLE_` prefix to a directive
name:

```text
//DISABLE_TEST:SIMPLE: -target spirv
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
```

`//TEST_IGNORE_FILE` clears every test discovered in the file. `//TEST_CATEGORY(...)`
adds file-wide categories that are combined into subsequent test directives.

### Command Options

Command options are parsed from the parentheses after the test command, separated by commas.
Options may be `name=value` pairs or bare flags.

- `filecheck=PREFIX`: Run FileCheck over the tool's normal output. `SIMPLE`,
  `REFLECTION`, `INTERPRET`, `COMPILE`, `SPVDB_DEBUGGER`, and several comparison tests
  support this path through the shared output validator.
- `filecheck-buffer=PREFIX`: For compute render tests, run FileCheck over the buffer text
  written by `render-test` instead of comparing `<test>.expected.txt`.
- `diag=PREFIX`: For diagnostic tests, check source annotations such as `// PREFIX: ...`
  with the diagnostic annotation checker. This also enables machine-readable diagnostics for
  `SIMPLE`-based diagnostic tests.
- `non-exhaustive`: Used with `diag=...` when a diagnostic test intentionally checks only
  the annotated diagnostics it names.

Unknown command options are not rejected at parse time. Some commands use their own keys
(for example language-server tests have LSP-specific options), and otherwise unused keys are
ignored by the selected test command.

## Test Types

The command name after `//TEST:` selects the runner callback. The most common commands are:

| Command                  | What it runs                                                                                                                                                                                                          | Output check                                                                                                  |
| ------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------- |
| `SIMPLE`                 | Runs `slangc` on the current test file plus directive arguments.                                                                                                                                                      | Compares the wrapped result output with `<test>.expected`, or uses `filecheck`.                               |
| `SIMPLE_EX`              | Runs `slangc` with only the directive arguments. The current file is not added automatically, so the directive must name every input file itself.                                                                     | Same as `SIMPLE`.                                                                                             |
| `SIMPLE_LINE`            | Runs `slangc`, parses diagnostics, and checks the first diagnostic line number.                                                                                                                                       | Compares that line number with expected output, or uses `filecheck`.                                          |
| `INTERPRET`              | Runs `slangi` on the current file.                                                                                                                                                                                    | Same wrapped-output validation as `SIMPLE`.                                                                   |
| `REFLECTION`             | Runs `slang-reflection-test` on the current file.                                                                                                                                                                     | Validates JSON output when compilation succeeds, then compares or FileChecks the wrapped output.              |
| `CPU_REFLECTION`         | Same as `REFLECTION`, but writes architecture-specific reflection baselines using a `.32` or `.64` suffix.                                                                                                            | Same as `REFLECTION`.                                                                                         |
| `LANG_SERVER`            | Starts the language-server test flow for requests such as completion, hover, and signature help.                                                                                                                      | Uses language-server-specific expected output.                                                                |
| `COMPARE_COMPUTE`        | Runs `render-test` and appends implicit `-slang -compute` arguments.                                                                                                                                                  | Validates process output, then compares the rendered buffer with `<test>.expected.txt` or `filecheck-buffer`. |
| `COMPARE_COMPUTE_EX`     | Runs `render-test` without implicit language/stage arguments. Use this when the directive needs to name the API and stage explicitly, such as `-vk -compute`, `-dx12 -compute`, `-cuda -compute`, or `-cpu -compute`. | Same as `COMPARE_COMPUTE`.                                                                                    |
| `COMPARE_RENDER_COMPUTE` | Runs `render-test` and appends implicit `-slang -gcompute` arguments.                                                                                                                                                 | Same as `COMPARE_COMPUTE`.                                                                                    |
| `COMPILE`                | Runs `slangc` with exactly the directive arguments and expects compilation to succeed.                                                                                                                                | Fails on a non-zero compiler result; `filecheck` can be used for compiler output.                             |
| `COMPILE_TARGET`         | Synthesized internally from render tests to make sure an explicit render target also compiles. It is rarely written by hand.                                                                                          | Fails on a non-zero `render-test -compile-only` result.                                                       |

Specialized commands exist for narrower infrastructure tests:

| Command                       | Purpose                                                                                                                                                                                     |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `COMMAND_LINE_SIMPLE`         | Reuses the `SIMPLE` runner but treats the source file path as the output stem. This is mainly useful for command-line diagnostics where the expected files are anchored to the source path. |
| `COMPARE_DXIL`                | Compiles through Slang and `dxc`, then compares DXIL assembly output. It is not currently used by tests in the tree.                                                                        |
| `CPP_COMPILER_COMPILE`        | Compiles C or C++ output through the configured downstream C/C++ compiler and fails if compilation fails.                                                                                   |
| `CPP_COMPILER_SHARED_LIBRARY` | Builds a C/C++ source file as a shared library and calls an exported `test` function.                                                                                                       |
| `CPP_COMPILER_EXECUTE`        | Builds C/C++ output as an executable, runs it, and compares the executable output.                                                                                                          |
| `PERFORMANCE_PROFILE`         | Runs `render-test -performance-profile`, extracts the measured time, and records it in the test report.                                                                                     |
| `DOC`                         | Runs `slangc` and compares wrapped output against `<test>.expected`, defaulting to an empty-success result when no expected file exists. It is not currently used by tests in the tree.     |
| `EXECUTABLE`                  | Compiles the Slang file to a host executable, runs that executable, and compares its wrapped output with `<test>.expected`.                                                                 |
| `SPVDB_DEBUGGER`              | When built with SPVDB support, compiles SPIR-V with debug info and runs `//SPVDB-CMD:` debugger commands embedded in the test file.                                                         |
| `DISPATCHER`                  | Runs `slang-dispatcher`; this is a narrow tool test rather than a typical shader test.                                                                                                      |

Deprecated commands remain available for old tests, but new tests should normally use
`SIMPLE`, `COMPARE_COMPUTE`, `COMPARE_COMPUTE_EX`, `COMPARE_RENDER_COMPUTE`, or one of the
specialized commands above when their behavior is specifically needed:

- `COMPARE_HLSL`
- `COMPARE_HLSL_RENDER`
- `COMPARE_HLSL_CROSS_COMPILE_RENDER`
- `COMPARE_HLSL_GLSL_RENDER`
- `COMPARE_GLSL`
- `HLSL_COMPUTE`
- `CROSS_COMPILE`

## Compiler Optimization in Tests

`slang-test` adds `-O0` when it builds a Slang compiler command line and the test directive
does not already specify an optimization option. This keeps ordinary test runs fast and
avoids expected-output churn from optimizer changes.

Pass `-OX`, where X is between 0 and 3, to `slang-test` to change that default for a
selected test run. For example, this compiles tests that do not specify their own level with
`-O2`:

```text
slang-test tests/render*/smo* -O2
```

Compiler-backed tests, such as `SIMPLE`, `CROSS_COMPILE`, and diagnostic tests, can opt in
to optimized output with a normal `slangc` option:

```text
//TEST:SIMPLE: -target spirv -O3
```

Render-test-backed tests, such as `COMPARE_COMPUTE_EX`, must forward the optimization option
to `slangc`:

```text
//TEST(compute):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -Xslang -O3
```

Besides `-Xslang <option>`, the forwarding forms `-compile-arg <option>`,
`-xslang <option>`, and a `-Xslang... <options...> -X.` block are also recognized. Prefer
the `-Xslang <option>` form shown above for new tests.

Metal render tests are the one exception to the `-O0` default: they receive `-Xslang -O1`
instead, because the downstream `metal` toolchain that produces the metallib is unstable at
`-O0` on macOS CI. The generated MSL source is identical at every level, so this only
affects the downstream compilation.

The recognized optimization spellings are `-O`, `-O0`, `-Onone`, `-O1`, `-Odefault`,
`-O2`, `-Ohigh`, `-O3`, and `-Omaximal`. Prefer the lowest level that preserves the test's
expected output.

`slang-test` inserts its default at the front of the argument list for both compiler-backed
and render-test-backed commands, so directive arguments keep their meaning. In particular, a
directive that leaves a trailing option without its required argument, intentionally in a
diagnostic test or by mistake, cannot consume the inserted default.

## Test Input Specification

`//TEST_INPUT:` directives are consumed by `render-test`, not by the initial `slang-test`
directive scanner. They bind shader parameters for render-test-backed commands such as
`COMPARE_COMPUTE`, `COMPARE_COMPUTE_EX`, `COMPARE_RENDER_COMPUTE`, `HLSL_COMPUTE`, and
`PERFORMANCE_PROFILE`.

Each `TEST_INPUT` directive is parsed one line at a time. Do not split a single `set` value
or resource declaration across several source lines unless you are using the explicit
`begin_object` / `begin_array` form described below.

### Basic Forms

```text
//TEST_INPUT: <resource-type>(<options>):<bindings>
//TEST_INPUT: set <name> = <value-expression>
//TEST_INPUT: type_conformance <derived-type>:<interface-type>=<id>
//TEST_INPUT: globalSpecializationArg <type-name>
//TEST_INPUT: entryPointSpecializationArg <type-name>
//TEST_INPUT: render_targets <count>
```

Resource bindings after the colon are comma-separated:

- `out`: marks the value as output, so render-test will read it back for comparison
- `name=<path>`: binds the value to a shader parameter path

The parser also accepts `name <path>` without `=`, and many older tests use that spelling.
Prefer `name=<path>` in new tests. Names may include fields and array indices, for example
`name=scene.material[0]`.

### Resource Types

| Resource                                                                         | Meaning                                                       | Options                                                                       |
| -------------------------------------------------------------------------------- | ------------------------------------------------------------- | ----------------------------------------------------------------------------- |
| `ubuffer`                                                                        | Storage or structured buffer.                                 | `data`, `stride`, `count`, `counter`, `random`, `format`                      |
| `cbuffer`                                                                        | Constant-buffer object initialized from `data`.               | `data`                                                                        |
| `uniform`                                                                        | Uniform data used in object fields or entry-point parameters. | `data`                                                                        |
| `Texture1D`, `Texture2D`, `Texture3D`, `TextureCube`                             | Read-only textures.                                           | `size`, `arrayLength`, `content`, `format`, `depth`, `sampleCount`, `mipMaps` |
| `RWTexture1D`, `RWTexture2D`, `RWTexture3D`, `RWTextureCube`                     | Read-write textures.                                          | Same texture options                                                          |
| `RWTextureBuffer`                                                                | Buffer resource represented as a texture buffer.              | Same options as `ubuffer`                                                     |
| `Sampler`                                                                        | Sampler state.                                                | `depthCompare`, `filteringMode`                                               |
| `TextureSampler1D`, `TextureSampler2D`, `TextureSampler3D`, `TextureSamplerCube` | Combined texture/sampler handle.                              | Texture options plus sampler options                                          |
| `AccelerationStructure`                                                          | Ray-tracing acceleration structure placeholder.               | No options                                                                    |

### Buffer Options

- `data=[values...]`: Explicit data words. Values may be integers, floats, half-float
  literals such as `1.0h`, hexadecimal integers, negative numeric literals, or buffer names
  used as device-address references.
- `stride=N`: Buffer element stride in bytes. `stride=0` means an unstructured buffer.
- `count=N`: Number of elements.
- `counter=N`: Counter value for append/consume buffers. If omitted, no counter buffer is
  assigned.
- `random(type, size[, min[, max]])`: Generate random data. `type` is `int`, `uint`, or
  `float`.
- `format=FORMAT`: Buffer format, for example `R32Uint`, `R32Sint`, `R32Float`,
  `RG32Float`, `RGBA32Float`, or `RGBA8Unorm`.

Examples:

```text
//TEST_INPUT: ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
//TEST_INPUT: ubuffer(stride=4, count=256):name=largeBuffer
//TEST_INPUT: ubuffer(random(float, 1024, -1.0, 1.0), stride=4):name=randomData
```

### Texture And Sampler Options

- `size=N`: Texture width, and also height/depth for 2D and 3D textures.
- `arrayLength=N`: Number of array layers. For example, `Texture2D(..., arrayLength=3)`
  binds a single `Texture2DArray` resource with three 2D textures.
- `content=zero|one|chessboard|gradient`: Generated texture contents.
- `format=FORMAT`: Texture format.
- `depth`: Marks the texture as a depth texture.
- `sampleCount=one|two|four|eight|sixteen|thirtyTwo|sixtyFour`: MSAA sample count.
- `mipMaps=N`: Number of mip levels. `0`, the default, means render-test binds the maximum
  number of mips it generates.
- `depthCompare`: Creates a comparison sampler.
- `filteringMode=point|linear`: Sampler filtering mode. The default is `linear`.

Texture arrays and arrays of texture handles are different things:

```text
// A single Texture2DArray handle with three homogeneous 2D textures.
//TEST_INPUT: Texture2D(size=16, content=one, arrayLength=3):name=t2DArray
Texture2DArray<float4> t2DArray;

// Two independent Texture2D handles in a shader array.
//TEST_INPUT: set textures=[Texture2D(size=4, content=zero), Texture2D(size=8, content=one)]
Texture2D<float4> textures[2];
```

Combined texture/sampler resources accept both texture and sampler options:

```text
//TEST_INPUT: TextureSampler2D(size=4, content=one, filteringMode=point):name=t2D
```

### Value Expressions

`set` assigns a value expression to a named shader parameter. It is the most convenient form
for arrays, objects, parameter blocks, and nested structures.

Supported expression forms include:

- Numeric literals: `42`, `-3.14`, `0x40003C00`
- Arrays: `[1, 2, 3]`
- Aggregates: `{field: value}` or positional `{value0, value1}`
- Objects: `new TypeName { ... }`
- Output marking: `out <value-expression>`
- Specialization: `specialize(TypeArg0, TypeArg1) <value-expression>`
- Dynamic dispatch: `dynamic <value-expression>`
- Resource values such as `ubuffer(...)`, `Texture2D(...)`, `Sampler`, and
  `AccelerationStructure`

Example:

```text
//TEST_INPUT: set scene = new Scene { { {1,2,3,4} }, ubuffer(data=[1 2 3 4], stride=4), new MaterialSystem {{ {1,2,3,4} }, ubuffer(data=[1 2 3 4], stride=4)} }
//TEST_INPUT: set pb2 = new MyBuffer { out ubuffer(data=[0 0 0 0], stride=16, count=4) }
```

### Type Conformances And Dynamic Dispatch

`type_conformance` registers the concrete type/interface pair used by dynamic dispatch. The
optional integer after `=` is the runtime type id. Shader code that stores or constructs a
dynamic object must use the same id when it asks render-test to create that object at run
time.

```slang
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -profile sm_5_0 -use-dxbc -output-using-type
//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=gOutputBuffer
//TEST_INPUT: type_conformance Add:IInterface=1
//TEST_INPUT: type_conformance Mul:IInterface=2

interface IValue
{
    float getVal();
}

struct SimpleVal : IValue
{
    float val;
    float getVal() { return val; }
}

[anyValueSize(16)]
interface IInterface
{
    associatedtype V : IValue;
    V run<let N : int>(float arr[N]);
}

struct Add : IInterface
{
    float base;
    typealias V = SimpleVal;
    V run<let N : int>(float arr[N])
    {
        float sum = base;
        for (int i = 0; i < N; i++)
            sum += arr[i];
        V result;
        result.val = sum;
        return result;
    }
}

struct Mul : IInterface
{
    float base;
    typealias V = SimpleVal;
    V run<let N : int>(float arr[N])
    {
        float product = base;
        for (int i = 0; i < N; i++)
            product *= arr[i];
        V result;
        result.val = product;
        return result;
    }
}

RWStructuredBuffer<float> gOutputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    var obj = createDynamicObject<IInterface>(1, 1.0); // Runtime id 1 selects Add.
    float arr[3] = { 2, 3, 4 };
    gOutputBuffer[0] = obj.run(arr).getVal();
}
```

### Specialization Arguments

`globalSpecializationArg`, also spelled by the legacy aliases `global_type` and
`globalExistentialType`, appends a module-level specialization type argument. The type name
must be visible in the module, and the number of listed arguments must match the module's
specialization parameter count.

`entryPointSpecializationArg`, also spelled by the legacy aliases `type` and
`entryPointExistentialType`, does the same for entry-point specialization parameters.

```text
//TEST_INPUT: globalSpecializationArg VertImpl
//TEST_INPUT: entryPointSpecializationArg Impl
```

These tokens are directive keywords, not variable names. The value after the keyword is the
type argument consumed by Slang reflection when `render-test` specializes the module or
entry point.

### Hierarchical Input

For complex layouts, `begin_array` and `begin_object(type=...)` create a parent value and
subsequent `TEST_INPUT` lines add children to it until `end` returns to the previous parent.
Use this form only when the single-line `set` expression becomes too hard to read.

```text
//TEST_INPUT: begin_object(type=Impl):name=params.obj
//TEST_INPUT: uniform(data=[1]):name=val
//TEST_INPUT: end
```

`render_targets N` sets the number of render targets for render tests:

```text
//TEST_INPUT: render_targets 2
```

### Complete Compute Example

```slang
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 42.0;
    // CHECK: 42.0
}
```

## Unit Tests

In addition to the above test tools, there are also `slang-unit-test-tool` and
`gfx-unit-test-tool`, which are invoked as in the following examples. The unit tests also
run as part of `slang-test`.

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

These tests are located in
[tools/slang-unit-test](https://github.com/shader-slang/slang/tree/master/tools/slang-unit-test),
and defined with macros like `SLANG_UNIT_TEST(byteEncode)`.

### gfx-unit-test-tool

```bash
# Graphics unit tests
slang-test gfx-unit-test-tool/<test-name>

# e.g. run the `precompiledTargetModule2Vulkan` test.
slang-test gfx-unit-test-tool/precompiledTargetModule2Vulkan
```

These tests are located in
[tools/gfx-unit-test](https://github.com/shader-slang/slang/tree/master/tools/gfx-unit-test),
and likewise defined using macros like `SLANG_UNIT_TEST(precompiledTargetModule2Vulkan)`.
