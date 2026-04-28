// Test that array-typed varying parameters legalize correctly.
// Exercises the array-recursion path in `_createLegalVaryingVal`
// (`slang-ir-legalize-varying-params.cpp`) which handles the "outer
// array brackets" tracking when decomposing a varying parameter type
// into per-element registers.
//
// The PR's other vertex/fragment tests use scalar / vector / struct
// varyings only; array-typed varyings hit a separate recursion path.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -entry vertexMain -stage vertex
//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv-asm -entry vertexMain -stage vertex -emit-spirv-directly

struct VertexOutput
{
    // Array-typed varying output — legalization must enumerate each
    // element with its own location/semantic.
    float values[4] : OUT_VALUES;
    float4 position : SV_Position;
};

[shader("vertex")]
VertexOutput vertexMain(
    float4 pos : POSITION,
    // Array-typed varying input — same recursion path on the input
    // side.
    float inValues[4] : IN_VALUES)
{
    VertexOutput o;
    [ForceUnroll]
    for (int i = 0; i < 4; ++i)
    {
        o.values[i] = inValues[i] + float(i);
    }
    o.position = pos;
    return o;
}

// HLSL emission preserves the user-declared semantics on the
// struct field and on the function parameter. Both must round-trip
// through the array-recursion legalization path.
// CHECK_HLSL: OUT_VALUES
// CHECK_HLSL: SV_Position
// CHECK_HLSL: IN_VALUES

// SPIRV emits one Location decoration per array element (or per
// the top-level array, depending on the SPIR-V Location-bumping
// rules). Either way, the entry point + Location decorations are
// emitted.
// CHECK_SPV: OpEntryPoint
// CHECK_SPV: Location
