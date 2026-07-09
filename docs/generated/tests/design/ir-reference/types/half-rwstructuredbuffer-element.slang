//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=4995d79f1d575606442d0357f0fb015ffcc2489a1b4116d09d5d6d1d0bde2e0f
//META: purpose=Expansion: RWStructuredBuffer<half> names Half as its element-type operand.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's RWStructuredBuffer row admits any value type as the
// element type. A `RWStructuredBuffer<half>` exposes the narrow
// floating-point scalar as the element-type operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<half> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = half(1.5);
}

// IR: let {{.*}}%buf{{.*}}: RWStructuredBuffer(Half{{.*}}= global_param
