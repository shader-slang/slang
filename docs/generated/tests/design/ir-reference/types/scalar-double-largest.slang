//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=fc3f64cf1767fc468943c658dcb39e5c71cdbb3af8d9335eb8de55db2a3c15f7
//META: purpose=Boundary: largest floating-point scalar (Double, 64-bit) appears alone on a global_param's IR type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// largest floating-point scalar (Double, 64-bit) appears alone on a
// global_param's IR type. A single `uniform double` exercises the
// largest-edge floating-point boundary.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform double v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = float(v);
}

// IR: let {{.*}}%v{{.*}}: Double{{.*}}= global_param
