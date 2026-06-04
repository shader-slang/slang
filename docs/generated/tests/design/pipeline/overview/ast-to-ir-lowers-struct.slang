//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/overview.md#ast-ir-lowering
//META: doc_section_digest=8186b395b3684ed4e6e22f74790f541ad117cdf9f69fed4adc7bd83bf333f222
//META: purpose=Decls in the AST (here a struct) lower into IR types and re-emerge in the emitted target text after IR passes.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the overview claim that the lowering visitor turns AST
// decls into IR (`IRStructType`, here) and that the IR survives the
// passes and emit. A struct declared in source has to be visible in
// the emitted target text in some form: as a struct in HLSL, GLSL,
// Metal, and as an OpTypeStruct (member layout) in SPIR-V.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

struct Pair
{
    int a;
    float b;
};

RWStructuredBuffer<Pair> gOut;

int useThePair(Pair p)
{
    return p.a + int(p.b);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Pair p;
    p.a = 3;
    p.b = 4.0;
    if (useThePair(p) > 0)
    {
        gOut[0] = p;
    }
}

// HLSL: struct Pair
// GLSL: struct Pair
// SPIRV: OpTypeStruct
