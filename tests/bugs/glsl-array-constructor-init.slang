// glsl-array-constructor-init.slang
//
// Regression for #11802: `-target glsl` emitted C-style brace initializers
// (`const ivec2 a_0[2] = { ivec2(1,1), ivec2(2,2) };`) for array values, which
// are invalid GLSL in every version. GLSL requires array-constructor syntax
// `elementType[]( e0, e1, ... )`. The existing tests/bugs/glsl-static-const-array.slang
// only runs `-vk` (SPIR-V), so it never exercised the GLSL *text* emit path.
//
// The arrays are indexed by a runtime value so the literals materialize in the
// emitted GLSL rather than being folded away.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage vertex -entry main
// Also round-trip the emitted GLSL through glslang (-emit-spirv-via-glsl) so this
// guards validity, not just the textual form: a brace initializer, or a wrongly
// ordered nested array constructor, would be rejected by glslang and fail to
// produce SPIR-V.
//TEST:SIMPLE(filecheck=GLSLSPV): -target spirv-asm -emit-spirv-via-glsl -stage vertex -entry main

static const int2 a[2] = {{1, 1}, {2, 2}};
static const int b[3] = {10, 20, 30};
static const int m[2][3] = {{1, 2, 3}, {4, 5, 6}};

struct Result
{
    float4 pos : SV_Position;
};

[shader("vertex")]
Result main(uint vid : SV_VertexID)
{
    Result r;
    int s = a[vid][0] + b[vid] + m[vid % 2][vid % 3];
    r.pos = float4(float(s), 0, 0, 0);
    return r;
}

// Array values must use GLSL array-constructor syntax, never C-style brace init.
// GLSL-DAG: ivec2[](
// GLSL-DAG: int[](
// GLSL-DAG: int[][3](

// The GLSL above compiles through glslang to valid SPIR-V, proving the
// constructor syntax (including the nested int[][3](...)) is accepted.
// GLSLSPV: OpEntryPoint Vertex
