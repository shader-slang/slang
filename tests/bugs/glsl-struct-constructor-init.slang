// glsl-struct-constructor-init.slang
//
// Regression for #11899: `-target glsl` emitted C-style brace initializers
// (`Inner_0 _S1 = { vin_inner_a_0, vin_inner_b_0 };`) for struct values. Brace
// initializers are only valid in GLSL 4.20+ (GL_ARB_shading_language_420pack), so
// they break on earlier profiles such as glsl_330; the portable spelling is the
// struct constructor `StructType( f0, f1, ... )`, which is core since GLSL 1.10.
// This is the struct sibling of #11802 / PR #11819, which fixed the same defect for
// array values. Slang floors the emitted #version to 450 regardless of -profile, so
// the emitted text is exercised (and validated below) at #version 450.
//
// The struct is delivered as a vertex entry-point parameter, so it is scalarized
// into per-field varying inputs and then reassembled with a MakeStruct to pass to
// the helper; that reassembly is what reaches GLSL emit. The nested struct exercises
// the recursive path: the outer constructor takes the (reassembled) inner struct
// value as one of its operands.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage vertex -entry main
// Also round-trip the emitted GLSL through glslang (-emit-spirv-via-glsl): this proves
// the emitted struct-constructor syntax (including the nested outer constructor whose
// operand is an inner struct value) is valid GLSL that glslang accepts. Note this does
// NOT prove the old brace form is rejected: Slang floors the emitted #version to 450,
// where aggregate initializers are valid, so both forms compile here. The portability
// regression is guarded by the textual GLSL check above, which requires the
// constructor spelling and would not match a brace initializer.
//TEST:SIMPLE(filecheck=GLSLSPV): -target spirv-asm -emit-spirv-via-glsl -stage vertex -entry main

struct Inner
{
    float a, b;
};

struct Outer
{
    Inner inner;
    float w;
};

float4 useInner(Inner t)
{
    return float4(t.a, t.b, 0, 1);
}

float4 useOuter(Outer o)
{
    return useInner(o.inner) + float4(0, 0, o.w, 1);
}

[shader("vertex")]
float4 main(Outer vin) : SV_Position
{
    return useOuter(vin);
}

// Struct values must use GLSL struct-constructor syntax, never C-style brace init.
// The flat/inner constructor is pinned with its stable varying-input operands. The
// nested/outer constructor is pinned as a full temp-variable initializer whose
// right-hand side is `Outer_0(<inner-temp>, vin_w_0)`, proving the outer struct is
// also constructed (a regressed `Outer_0 _S2 = { ... }` would not match). The
// `= Outer_0(` anchor is required because a bare `Outer_0(` also matches the emitted
// helper name `useOuter_0(`; the temp names use FileCheck regex since they are
// synthesized (e.g. `_S1`, `_S2`).
// GLSL-DAG: Inner_0(vin_inner_a_0, vin_inner_b_0)
// GLSL-DAG: Outer_0 _S{{[0-9]+}} = Outer_0(_S{{[0-9]+}}, vin_w_0)

// The GLSL above compiles through glslang to valid SPIR-V, proving the struct
// constructor syntax (including the nested outer constructor) is accepted.
// GLSLSPV: OpEntryPoint Vertex
