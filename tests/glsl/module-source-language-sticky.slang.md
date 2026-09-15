# Sticky GLSL source language across preprocessed segments

This test verifies that a module's recorded source language is sticky toward GLSL
across preprocessed segments. A literate module is parsed as one segment per code
block, and a `#version` directive marks its segment as GLSL, so this module has a
GLSL segment followed by Slang segments. The module must stay GLSL-flavored
module-wide (a later non-GLSL segment does not downgrade it), so the global `const`
with an initializer below does **not** require `static` here — it would otherwise
error with 31224 under plain Slang — and the module compiles. The plain-Slang
counterpart (no `#version`) is covered by
`tests/diagnostics/global-const-uniform-with-init.slang`, which asserts that error.

```slang
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage fragment
#version 450
```

```slang
const int gValue = 42;
```

```slang
float4 main() : SV_Target
{
    // CHECK: OpEntryPoint
    return float4(float(gValue));
}
```
