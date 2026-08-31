//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizeuniformbufferload
//META: doc_section_digest=dacbe0c33c670a59f3b8ad61a7d49e89ea75e095f3ed8e7f44d44926ea0cbb13
//META: purpose=Reading a whole `ConstantBuffer<S>` whose element type is a struct reaches the HLSL text as one member read per field recombined in a brace-list, never as a whole-object copy of the cbuffer variable.
//META: intent=functional
//META: pipeline_stage=legalize
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `legalizeUniformBufferLoad` rewrites the whole-object load of a constant
// buffer with a struct element type into a per-field `IRFieldAddress` +
// `IRLoad` sequence recombined by `makeStruct`. HLSL has no syntax for a
// whole-buffer load, so the split is what makes the read spellable at all.
//
// The rewrite is observable because `kIROp_MakeStruct` is on the emitter's
// never-fold list: the recombined struct materialises as a local declaration
// whose initializer is a brace-list of one member read per field, in field
// order.
//
// The recombined value has to be *consumed as a struct* for that declaration
// to survive. If the entry point reads only individual fields, later
// simplification forwards each field read to its use and no brace-list is
// emitted at all — the emitted body is just `gCB_0.a_0.x + ...`. The
// `[noinline]` consumer here takes the struct by value, so the whole-struct
// value must be materialised at the call site.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct S
{
    float4 a;
    int2 b;
    float c;
}

ConstantBuffer<S> gCB;
RWStructuredBuffer<float> outBuf;

[noinline]
float consume(S s)
{
    return s.a.x + float(s.b.y) + s.c;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = consume(gCB);
}

// The brace-list initializer reads each field of the cbuffer variable in
// declaration order; the three members arrive as separate reads rather than
// as one whole-object copy of the cbuffer.
// CHECK: S_0 {{[A-Za-z_0-9]+}} = { {{[A-Za-z_0-9]+}}.a_0, {{[A-Za-z_0-9]+}}.b_0, {{[A-Za-z_0-9]+}}.c_0 };
