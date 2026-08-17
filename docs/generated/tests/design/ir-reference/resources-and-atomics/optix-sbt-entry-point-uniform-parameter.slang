//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#raytracing-payload
//META: doc_section_digest=67617f4eb68ef6a0c6b50b0fba002690b874dbf4cf412014b8b61a11403f1548
//META: purpose=A single scalar `uniform` **parameter** on a non-compute entry point is enough to produce `getOptiXSbtDataPointer`, typed `ConstantBuffer(%ShaderRecordParams, DefaultLayout)` in the IR dump and read as `((ShaderRecordParams_0*)optixGetSbtDataPointer())->scale_0` in the emitted CUDA.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the worked example that settles what actually produces this
// opcode: *where* the uniform is declared, not how large it is.
//
// The shader is a `closesthit` entry point whose only uniform is one
// `float` parameter. The IR directive pins the opcode together with the
// result type that shows the pass built a shader-record constant buffer
// for it; the CUDA directive pins the emitted cast-and-dereference, which
// is the artifact a user sees. Both are needed: the opcode is created by
// `collectOptiXEntryPointUniformParams`, so a text-only check would not
// distinguish it from any other route to the same call.

//TEST:SIMPLE(filecheck=IR):-target cuda -dump-ir -o /dev/null -stage closesthit -entry ch
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage closesthit -entry ch

struct Payload
{
    float3 color;
};

[shader("closesthit")]
void ch(inout Payload p, uniform float scale)
{
    p.color = float3(scale, scale, scale);
}

// IR: ConstantBuffer(%ShaderRecordParams, DefaultLayout){{.*}}getOptiXSbtDataPointer

// CUDA: ((ShaderRecordParams_{{[0-9]+}}*)optixGetSbtDataPointer())->scale_{{[0-9]+}}
