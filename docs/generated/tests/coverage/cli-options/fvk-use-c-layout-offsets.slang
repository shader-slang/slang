//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies -fvk-use-c-layout lays out a constant-buffer struct with C/C++ rules, producing an `_c`-suffixed SPIR-V struct with member offsets 0/4/16 where the default std140 layout produces an `_std140` struct with offsets 0/16/28.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// White-box target: the `OptionKind::ForceCLayout` arm of
// `OptionsParser::_parse` in slang-options.cpp, the flag spelled
// `-fvk-use-c-layout`. It is the one member of the layout-override family
// (`-force-glsl-scalar-layout`, `-fvk-use-dx-layout`, `-fvk-use-c-layout`) that
// neither shipped suite reached.
//
// The struct below is chosen so the three layout rules disagree: `float a;
// float3 b; float c;`. Under std140 the `float3` must start at a 16-byte
// boundary, so the members land at 0/16/28 and the legalized struct is named
// `S_std140`. Under C rules the `float3` only needs 4-byte alignment, so the
// members land at 0/4/16 and the struct is named `S_c`. The struct-name suffix
// and the offsets are two independent handles on the same choice, and both are
// pinned; the offsets are what a host writing the buffer would actually have to
// match.

//TEST:SIMPLE(filecheck=DEFAULT):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CLAYOUT):-target spirv-asm -entry main -stage compute -fvk-use-c-layout

struct S
{
    float a;
    float3 b;
    float c;
};

ConstantBuffer<S> cb;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = cb.a + cb.b.x + cb.c;
}

//DEFAULT: OpMemberDecorate %S_std140 0 Offset 0
//DEFAULT: OpMemberDecorate %S_std140 1 Offset 16
//DEFAULT: OpMemberDecorate %S_std140 2 Offset 28

//CLAYOUT: OpMemberDecorate %S_c 0 Offset 0
//CLAYOUT: OpMemberDecorate %S_c 1 Offset 4
//CLAYOUT: OpMemberDecorate %S_c 2 Offset 16
