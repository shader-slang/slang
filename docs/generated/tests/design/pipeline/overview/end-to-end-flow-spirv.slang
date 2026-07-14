//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/overview.md#end-to-end-flow
//META: doc_section_digest=83e0cd09591679dc891de1a189fb967b34696a3e15bf5b249cb10e3505f9897f
//META: purpose=A non-trivial source successfully traverses lex, preprocess, parse, check, lower, IR passes, and emit to produce SPIR-V text.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the overview end-to-end-flow diagram: source ->
// lex/preprocess -> parse -> check -> lower -> IR passes -> emit ->
// target artefact. The shader exercises a preprocessor macro, a
// struct declaration, a function call, and control flow so that the
// success of compilation implicitly requires every stage to do its
// work. We FileCheck for canonical SPIR-V markers (OpCapability,
// OpEntryPoint, OpFunction) in the emitted assembly.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

#define LIMIT 4

struct Counter
{
    int value;
};

int bump(Counter c)
{
    return c.value + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Counter c;
    c.value = LIMIT;
    int next = bump(c);
    if (next > 0)
    {
        return;
    }
    return;
}

//CHECK: OpCapability Shader
//CHECK: OpEntryPoint GLCompute %main "main"
//CHECK: OpFunction
