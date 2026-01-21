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
- `-v [level]`: Set verbosity level (verbose, info, failure). Default: verbose when -v used, info otherwise
- `-verbose-paths`: Use verbose paths in output
- `-hide-ignored`: Hide results from ignored tests

### Test Selection and Categories
- `-category <name>`: Only run tests in specified category
- `-exclude <name>`: Exclude tests in specified category
- `-exclude-prefix <prefix>`: Exclude tests with specified path prefix

Available test categories:
- `full`: All tests
- `quick`: Quick tests
- `smoke`: Basic smoke tests
- `render`: Rendering-related tests
- `compute`: Compute shader tests
- `vulkan`: Vulkan-specific tests
- `compatibility-issue`: Tests for compatibility issues

A test may be in one or more categories. The categories are specified on top of a test, for example: //TEST(smoke,compute):COMPARE_COMPUTE:

Note: Additional categories exist and may appear in the tree (non-exhaustive): `unit-test`, `cuda`, `optix`, `wave`, `wave-mask`, `wave-active`, `windows`, `unix`, `64-bit`, `shared-library`. The list above shows the most common ones.

### API Control Options
- `-api <expr>`: Enable specific APIs (e.g., 'vk+dx12' or '+dx11')
- `-api-only`: Only run tests that use specified APIs
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
  - `vk+dx11`: Vulkan and DirectX 11

Available APIs:
- Vulkan: `vk`, `vulkan`
- DirectX 12: `dx12`, `d3d12`
- DirectX 11: `dx11`, `d3d11`
- Metal: `mtl`, `metal`
- CPU: `cpu`
- CUDA: `cuda`
- WebGPU: `wgpu`, `webgpu`

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
- `-skip-reference-image-generation`: Skip generating reference images for render tests
- `-emit-spirv-via-glsl`: Emit SPIR-V through GLSL instead of directly
- `-expected-failure-list <file>`: Specify file containing expected failures
- `-capability <name>`: Compile with the given capability
- `-shuffle-tests`: Shuffle tests in directories
- `-shuffle-seed <seed>`: Set shuffle seed (default: 1)
- `-ignore-abort-msg`: Ignore abort message dialog popup on Windows
- `-enable-debug-layers [true|false]`: Enable or disable Validation Layer for Vulkan and Debug Device for DX
- `-cache-rhi-device [true|false]`: Enable or disable RHI device caching (default: true)

## Test Types

Tests are identified by a special comment at the start of the test file: `//TEST:<type>:`

To ignore a test, use `//DISABLE_TEST` instead of `//TEST`.

Available test types:
- `SIMPLE`: Runs the slangc compiler with specified options after the command
- `REFLECTION`: Runs slang-reflection-test with the options specified after the command
- `INTERPRET`: Runs slangi interpreter to execute shader code and compare output
- `COMPARE_COMPUTE`: Runs render-test to execute a compute shader and writes the result to a text file. The test passes if the output matches the expected content
- `COMPARE_COMPUTE_EX`: Same as COMPARE_COMPUTE, but supports additional parameter specifications
- `COMPARE_RENDER_COMPUTE`: Runs render-test with "-slang -gcompute" options and compares text file outputs
- `LANG_SERVER`: Tests Language Server Protocol features by sending requests (like completion, hover, signatures) and comparing responses with expected outputs

Note: The list above covers the primary test kinds used in test files. There are additional, more specialized commands in the runner (e.g., `SIMPLE_EX`, `SIMPLE_LINE`, `CPU_REFLECTION`, `COMPARE_DXIL`, `COMMAND_LINE_SIMPLE`, `CPP_COMPILER_*`, `PERFORMANCE_PROFILE`, `COMPILE`, `DOC`, `EXECUTABLE`), which are typically not used when authoring new tests.

Deprecated test types (do not create new tests of these kinds, and we need to slowly migrate existing tests to use SIMPLE, COMPARE_COMPUTE(_EX) or COMPARE_RENDER_COMPUTE instead):
- `COMPARE_HLSL`: Runs the slangc compiler with forced DXBC output and compares with a file having the '.expected' extension
- `COMPARE_HLSL_RENDER`: Runs render-test to generate two images - one using HLSL (expected) and one using Slang, saving both as .png files. The test passes if the images match
- `COMPARE_HLSL_CROSS_COMPILE_RENDER`: Runs render-test to generate two images - one using Slang and one using -glsl-cross. The test passes if the images match
- `COMPARE_HLSL_GLSL_RENDER`: Runs render-test to generate two images - one using -hlsl-rewrite and one using -glsl-rewrite. The test passes if the images match
- `COMPARE_GLSL`: Runs the slangc compiler both through Slang and directly, then compares the SPIR-V assembly output
- `HLSL_COMPUTE`: Runs render-test with "-hlsl-rewrite -compute" options and compares text file outputs
- `CROSS_COMPILE`: Compiles using GLSL pass-through and through Slang, then compares the outputs

## Test Input Specification

The `TEST_INPUT` directive is used to specify shader input resources and parameters for tests. These directives are parsed by `render-test` to set up the test environment with buffers, textures, samplers, and other resources.

### Basic Syntax

```
//TEST_INPUT: <resource-type>(<options>):<modifiers>
//TEST_INPUT: set <name> = <value-expression>
//TEST_INPUT: <directive> <args>
```

### Resource Types

#### Buffers

**`ubuffer`** - Storage/structured buffer (RWStructuredBuffer)

Options: `data`, `stride`, `count`, `counter`, `random`, `format`

Examples:
```slang
//TEST_INPUT: ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
//TEST_INPUT: ubuffer(stride=4, count=256):name=largeBuffer
//TEST_INPUT: ubuffer(random(float, 1024, -1.0, 1.0), stride=4):name=randomData
```

**`cbuffer`** - Constant buffer (uniform buffer)

Options: `data`

Example:
```slang
//TEST_INPUT: cbuffer(data=[1.0 0.0 0.0 0.0  0.0 1.0 0.0 0.0]):name matrixBuffer
```

**`uniform`** - Uniform data

Options: `data`

Example:
```slang
//TEST_INPUT: uniform(data=[1.0 2.0]):name uniformData
```

#### Textures

**Read-only textures:** `Texture1D`, `Texture2D`, `Texture3D`, `TextureCube`

Options: `size`, `arrayLength`, `content`, `format`

Examples:
```slang
//TEST_INPUT: Texture2D(size=4, content=zero):name t2D
//TEST_INPUT: Texture2D(size=4, content=one, arrayLength=2):name t2DArray
//TEST_INPUT: Texture2D(size=4, format=D32Float, content=one):name depthTex
//TEST_INPUT: TextureCube(size=4, content=gradient):name tCube
```

Note: Specifying `arrayLength` on `Texture1D`, `Texture2D`, or `TextureCube` creates `Texture1DArray`, `Texture2DArray`, or `TextureCubeArray` respectively.

**Read-write textures:** `RWTexture1D`, `RWTexture2D`, `RWTexture3D`, `RWTextureCube`

Options: same as read-only textures. `sampleCount` and `mipMaps` are accepted for both read-only and read-write textures.

Examples:
```slang
//TEST_INPUT: RWTexture2D(format=R32Uint, size=4, content=one, mipMaps=1):name rwTex
//TEST_INPUT: RWTexture2D(format=R32Float, size=4, sampleCount=two):name msaaTexture
```

**`RWTextureBuffer`** - Read-write texture buffer

Options: same as `ubuffer`

Example:
```slang
//TEST_INPUT: RWTextureBuffer(format=R32Float, stride=4, data=[1.0 1.0 1.0 1.0])
```

#### Samplers

**`Sampler`** - Texture sampler

Options: `depthCompare`, `filteringMode`

Examples:
```slang
//TEST_INPUT: Sampler:name sampler
//TEST_INPUT: Sampler(depthCompare):name shadowSampler
//TEST_INPUT: Sampler(filteringMode=linear):name linearSampler
```

**Combined texture-samplers:** `TextureSampler1D`, `TextureSampler2D`, `TextureSampler3D`, `TextureSamplerCube`

Options: combination of texture and sampler options

Example:
```slang
//TEST_INPUT: TextureSampler2D(size=4, content=one, filteringMode=point):name t2D
```

#### Ray Tracing

**`AccelerationStructure`** - Ray tracing acceleration structure

Example:
```slang
//TEST_INPUT: set accelStruct = AccelerationStructure
```

### Resource Options

#### Buffer Options

**`data=[values...]`** - Explicit buffer data

Values can be integers, floats (including negative), or hexadecimal (e.g., `0x40003C00`)

Examples:
```slang
data=[1 2 3 4]
data=[1.0 -2.5 3.14]
data=[0x00100000 0x00000000]
```

**`stride=N`** - Buffer element stride in bytes

Example: `stride=4`

**`count=N`** - Number of elements in buffer

Example: `count=256`

**`counter=N`** - Counter value for append/consume buffers

Example: `counter=0`

**`random(type, size[, min[, max]])`** - Generate random buffer data

Types: `int`, `uint`, `float`

Default ranges:
- `int`: full int32 range
- `uint`: full uint32 range
- `float`: [-1.0, 1.0]

Examples:
```slang
random(float, 256, -1.0, 1.0)    // 256 random floats in [-1.0, 1.0]
random(int, 1024, 0, 100)        // 1024 random ints in [0, 100]
random(uint, 512)                // 512 random uints (full range)
```

**`format=FORMAT`** - Buffer/texture format

Common formats: `R32Uint`, `R32Sint`, `R32Float`, `RG32Float`, `RGBA32Float`, `RGBA8Unorm`, `D32Float`

Example: `format=R32Uint`

#### Texture Options

**`size=N`** - Texture dimensions (width/height/depth)

Example: `size=4` creates a 4×4 2D texture or 4×4×4 3D texture

**`arrayLength=N`** - Number of array elements

Example: `arrayLength=2` on Texture2D creates Texture2DArray with 2 elements

**`content=TYPE`** - Texture content pattern

Values:
- `zero` - All zeros
- `one` - All ones (255 for normalized, 1 for integer formats)
- `gradient` - Gradient based on coordinates
- `chessboard` - Checkerboard pattern

Example: `content=gradient`

**`depth`** - Marks the texture as a depth texture

Example: `Texture2D(depth, format=D32Float, size=4)`

**`sampleCount=COUNT`** - MSAA sample count

Values: `one`, `two`, `four`, `eight`, `sixteen`, `thirtyTwo`, `sixtyFour`

Example: `sampleCount=four`

**`mipMaps=N`** - Number of mipmap levels

Example: `mipMaps=3`

#### Sampler Options

**`depthCompare`** - Enable depth comparison mode

Example: `Sampler(depthCompare)`

**`filteringMode=MODE`** - Texture filtering mode

Values: `point`, `linear`

Example: `filteringMode=linear`

### Modifiers

Resource declarations can include modifiers after a colon (`:`), separated by commas:

**`out`** - Marks resource as output (for comparison/verification)

**`name=IDENTIFIER`** - Assigns name to bind resource to shader parameter

Names support dotted paths and array indices: `name=scene.material[0]`

Examples:
```slang
:out,name=outputBuffer
:name=myTexture
```

### Advanced Directives

#### Setting Parameters with `set`

Use `set` to assign complex values or arrays to named parameters:

Simple array:
```slang
//TEST_INPUT: set textures=[Texture2D(size=4, content=zero), Texture2D(size=4, content=one)]
```

Complex nested structure:
```slang
//TEST_INPUT: set scene = new Scene {
    { {1,2,3,4} },
    ubuffer(data=[1 2 3 4], stride=4),
    new MaterialSystem {{ {1,2,3,4} }, ubuffer(data=[1 2 3 4], stride=4)}
}
```

Array of structured objects:
```slang
//TEST_INPUT: set gObj = new StructuredBuffer<UserType>[
    new UserType{[1.0, 2.0, 3.0], 3},
    new UserType{[2.0, 3.0, 4.0], 4}
]
```

#### Value Expressions

- **Literals**: `42`, `-3.14`, `0x40003C00`
- **Arrays**: `[1, 2, 3]`, `[1.0, -2.5, 3.14]`
- **Aggregates/Structs**: `{field1: value1, field2: value2}` or `{value1, value2}` (positional)
- **Objects**: `new TypeName { fields... }`
- **Output marking**: `out ubuffer(...)`
- **Specialization**: `specialize(Type1, Type2) value`
- **Dynamic dispatch**: `dynamic value`

#### Type Conformances

Specify type conformances for interface/dynamic dispatch testing:

```slang
//TEST_INPUT: type_conformance DerivedType:BaseInterface=id
```

Examples:
```slang
//TEST_INPUT: type_conformance Foo:IFoo=1
//TEST_INPUT: type_conformance Bar:IFoo=2
//TEST_INPUT: type_conformance FloatVal:IInterface=3
```

#### Specialization Arguments

For generic shader parameters:

**`globalSpecializationArg`** - Global specialization argument (type or value)
```slang
//TEST_INPUT: globalSpecializationArg MyType
//TEST_INPUT: globalSpecializationArg 7
```

**`entryPointSpecializationArg`** - Entry point specialization argument
```slang
//TEST_INPUT: entryPointSpecializationArg MyModifier
```

Legacy aliases (still supported):
- `type` / `entryPointExistentialType` → `entryPointSpecializationArg`
- `global_type` / `globalExistentialType` → `globalSpecializationArg`

#### Hierarchical Input (Advanced)

For complex hierarchical buffer structures (rare):

```slang
//TEST_INPUT: begin_object(type=Impl):name=params.obj
//TEST_INPUT: uniform(data=[1]):name=val
//TEST_INPUT: end
```

#### Render targets (Advanced)

Set the number of render targets for render tests:

```slang
//TEST_INPUT: render_targets 2
```

### Complete Examples

#### Basic Compute Test
```slang
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-compute -shaderobj
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain() {
    outputBuffer[0] = 42.0;
    // CHECK: 42.0
}
```

#### Texture Sampling Test
```slang
//TEST:SIMPLE(filecheck=WGSL): -stage fragment -entry fragMain -target wgsl
//TEST_INPUT: Texture2D(size=4, content=one):name t2D
//TEST_INPUT: Sampler(filteringMode=linear):name sampler
//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer

Texture2D<float4> t2D;
SamplerState sampler;
RWStructuredBuffer<int> outputBuffer;
```

#### Multiple Input Buffers
```slang
//TEST:COMPARE_COMPUTE:-compute -shaderobj
//TEST_INPUT: ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):name=input1
//TEST_INPUT: ubuffer(data=[0.0 1.0 2.0 3.0], stride=4):name=input2
//TEST_INPUT: ubuffer(stride=4, count=4):out,name=outputBuffer

RWStructuredBuffer<float> input1;
RWStructuredBuffer<float> input2;
RWStructuredBuffer<float> outputBuffer;
```

#### Complex Parameter Block
```slang
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST_INPUT: set scene = new Scene {
    { {1,2,3,4} },
    ubuffer(data=[1 2 3 4], stride=4),
    new MaterialSystem {{ {1,2,3,4} }, ubuffer(data=[1 2 3 4], stride=4)}
}
//TEST_INPUT: set pb2 = new MyBuffer { out ubuffer(data=[0 0 0 0], stride=4) }

struct MaterialSystem { CB cb; RWStructuredBuffer<uint4> data; }
struct Scene { CB sceneCb; RWStructuredBuffer<uint4> data; ParameterBlock<MaterialSystem> material; }

ParameterBlock<Scene> scene;
ParameterBlock<MyBuffer> pb2;
```

#### Random Data Generation
```slang
//TEST:COMPARE_COMPUTE:-compute -shaderobj
//TEST_INPUT: ubuffer(random(float, 1024, 0.0, 1.0), stride=4):name=randomFloats
//TEST_INPUT: ubuffer(random(int, 256), stride=4):name=randomInts
//TEST_INPUT: ubuffer(stride=4, count=1024):out,name=outputBuffer
```

#### Ray Tracing with Acceleration Structure
```slang
//TEST:COMPARE_COMPUTE:-compute -shaderobj
//TEST_INPUT: set accelStruct = AccelerationStructure
//TEST_INPUT: ubuffer(data=[0], stride=4):out,name outputBuffer

RaytracingAccelerationStructure accelStruct;
RWStructuredBuffer<float> outputBuffer;
```

#### Depth Texture with Comparison Sampler
```slang
//TEST:COMPARE_COMPUTE:-compute -shaderobj
//TEST_INPUT: Texture2D(size=4, format=D32Float, content=one):name texture
//TEST_INPUT: Sampler(depthCompare):name sampler
//TEST_INPUT: ubuffer(data=[0], stride=4):out,name output

Texture2D texture;
SamplerComparisonState sampler;
RWStructuredBuffer<float> output;
```

#### Array of Textures
```slang
//TEST:COMPARE_COMPUTE:-compute -shaderobj
//TEST_INPUT: set textures=[Texture2D(size=4, content=zero), Texture2D(size=4, content=one)]
//TEST_INPUT: set sampler=Sampler
//TEST_INPUT: ubuffer(stride=4, count=4):out,name=outputBuffer

Texture2D textures[2];
SamplerState sampler;
RWStructuredBuffer<float> outputBuffer;
```

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
