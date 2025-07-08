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

## Test Types

Tests are identified by a special comment at the start of the test file: `//TEST:<type>:`

Run `slang-test --help` to list the available and deprecated test types.

## Unit Tests
In addition to the above test tools, there are also `slang-unit-test-tool` and `gfx-unit-test-tool`, which are invoked as in the following examples.

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
