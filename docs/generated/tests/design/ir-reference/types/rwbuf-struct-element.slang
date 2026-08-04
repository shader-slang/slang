//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=4995d79f1d575606442d0357f0fb015ffcc2489a1b4116d09d5d6d1d0bde2e0f
//META: purpose=Boundary: RWStructuredBuffer parameterised by a user struct names that struct as its first operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `RWStructuredBuffer` is one of the documented
// resource opcodes. Its element-type operand can be any value
// type — including a user struct — and the dump preserves the
// struct's IR identity as that first operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Particle { float3 pos; float mass; int alive; }

uniform RWStructuredBuffer<Particle> particles;
uniform float a;

[numthreads(1, 1, 1)]
void main()
{
    particles[0].mass = a;
}

// IR: let {{.*}}%particles{{.*}}: RWStructuredBuffer(%Particle{{.*}}= global_param
