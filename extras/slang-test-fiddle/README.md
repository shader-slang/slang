# Test Generation with slang-fiddle

This directory contains examples and documentation for using `slang-fiddle` to generate Slang test files from templates.

## Overview

The `slang-fiddle` tool has been extended with a test generation mode that processes templates containing embedded Lua code to generate multiple test files. This uses the same FIDDLE template syntax already used throughout the Slang codebase for IR code generation.

## Command Line Usage

```bash
slang-fiddle --mode test-gen \
  --input <template.slang> \
  --output-dir <output-directory>
```

## Template Syntax

Templates use FIDDLE's standard syntax:

- Lines starting with `%` contain Lua code
- `$variable` splices Lua variables into the output
- All other lines are output as-is
- `emit_test_file(filename)` writes the accumulated content to a file

## Basic Example

```slang
#if 0 // FIDDLE TEMPLATE:
%local types = {"int", "uint", "float"}
%for _, typ in ipairs(types) do
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<$typ> outputBuffer;

[numthreads(1,1,1)]
void computeMain() {
    $typ result = ($typ)10 + ($typ)32;
    outputBuffer[0] = result;
}
// CHECK: 42
%    emit_test_file("numeric-ops/add-" .. typ .. ".slang")
%end
#else // FIDDLE OUTPUT:
#endif // FIDDLE END
```

Running this template generates three test files:
- `numeric-ops/add-int.slang`
- `numeric-ops/add-uint.slang`
- `numeric-ops/add-float.slang`

## Template Structure

### Interleaved Content

Templates interleave Lua control flow with Slang code:

```slang
#if 0 // FIDDLE TEMPLATE:
%for _, typ in ipairs({"int", "float"}) do
void test_$typ() {
    $typ x = 42;
}
%    emit_test_file("test-" .. typ .. ".slang")
%end
#else // FIDDLE OUTPUT:
#endif // FIDDLE END
```

The template processor accumulates non-Lua lines as content, substituting `$variable` references, until `emit_test_file()` is called.

### Variable Splicing

FIDDLE's `$` splice mechanism works with simple Lua variables only, not table field access:

```lua
%local config = {type="int", value=42}
%local typ = config.type    -- Extract to local variable
%local val = config.value
$typ x = $val;              -- This works
```

Direct field access like `$config.type` is not supported. Extract table fields to local variables before the template content that uses them.

### Output Organization

The `emit_test_file()` function supports subdirectories:

```lua
%emit_test_file("numeric-ops/add-int.slang")
%emit_test_file("texture-tests/sample.slang")
```

Directories are created automatically if they don't exist.

## Common Patterns

### Loop Over Array

```slang
#if 0 // FIDDLE TEMPLATE:
%local items = {"a", "b", "c"}
%for _, item in ipairs(items) do
Test content for: $item
%    emit_test_file("category/" .. item .. ".slang")
%end
#else // FIDDLE OUTPUT:
#endif // FIDDLE END
```

### Nested Loops

```slang
#if 0 // FIDDLE TEMPLATE:
%local types = {"int", "float"}
%local ops = {"+", "-"}
%for _, typ in ipairs(types) do
%    for _, op in ipairs(ops) do
$typ compute() { return 1 $op 2; }
%        emit_test_file("ops/" .. typ .. "-" .. op .. ".slang")
%    end
%end
#else // FIDDLE OUTPUT:
#endif // FIDDLE END
```

### Structured Configuration

```slang
#if 0 // FIDDLE TEMPLATE:
%local configs = {
%    {name="test1", type="int", value=42},
%    {name="test2", type="float", value=3.14}
%}
%for _, cfg in ipairs(configs) do
%    local name = cfg.name
%    local typ = cfg.type
%    local val = cfg.value
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
void test_$name() {
    $typ x = $val;
    outputBuffer[0] = x;
}
%    emit_test_file("tests/" .. name .. ".slang")
%end
#else // FIDDLE OUTPUT:
#endif // FIDDLE END
```

## Examples

The `examples/` directory contains working templates:

### simple-numeric-test.slang

Basic template demonstrating type iteration and subdirectory output.

### advanced-numeric-ops.slang

Nested loops with structured data and the local variable extraction pattern.

### texture-capability-test.slang

Texture test generation using Lua functions and conditional directives.

### texture-capabilities-full.slang

Comprehensive real-world example generating 29 test files with conditional test generation and dynamic directives.

## Implementation Details

### Modified Files

The following files in `tools/slang-fiddle/` were modified to add test generation support:

- `slang-fiddle-options.h` - Added `TestGen` mode enum and command-line options
- `slang-fiddle-main.cpp` - Added test generation mode handler
- `slang-fiddle-script.h` - Added test generation global state declarations
- `slang-fiddle-script.cpp` - Implemented `emit_test_file()` Lua function
- `slang-fiddle-template.h` - Added `hasTrailingNewline` field to splice statements
- `slang-fiddle-template.cpp` - Fixed newline handling after identifier splices

### Test Generation Mode

When invoked with `--mode test-gen`:

1. The template file is parsed using standard FIDDLE parsing
2. RAW() and SPLICE() functions accumulate content into a buffer instead of generating `.fiddle` files
3. Lua code executes, calling `emit_test_file()` to write accumulated content
4. Each `emit_test_file()` call writes the buffer to the specified file and clears it

### Newline Preservation

A fix was added to preserve newlines after `$identifier` splices at line endings. Without this fix, lines would collapse together in the generated output. The parser now detects and consumes trailing newlines after splices, emitting them explicitly during code generation.

## Workflow Example

### Generate All Examples at Once

```bash
# Generate tests from all example templates to generated-tests/
./extras/slang-test-fiddle/examples/generate-examples.sh
```

### Generate from Individual Template

```bash
# Generate tests from template
./build/generators/Release/bin/slang-fiddle \
  --mode test-gen \
  --input extras/slang-test-fiddle/examples/simple-numeric-test.slang \
  --output-dir tests/fiddle

# Run generated tests
./build/Release/bin/slang-test tests/fiddle/numeric-ops/
```

## Notes

- Templates use the same Lua 5.4 interpreter embedded in slang-fiddle
- The syntax is identical to FIDDLE templates used for IR code generation
- Template changes require re-running slang-fiddle to regenerate tests
- `slang-test` integration alternatives:
  - generated files can be merged under `tests/fiddle` from where they are executed automatically
  - generate tests during `slang-test` runtime and add `tests/fiddle` to `.gitignore`
