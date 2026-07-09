//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=b49b3d499615e99101dda545c0f7fef50e62178dcf5f11f8dcef9ff0becec030
//META: purpose=Expansion: a struct field of Double lowers to a field(...) row carrying Double.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's struct row says fields carry their IR type. A struct
// holding `double` fields produces `field(%key, Double)` rows.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Wide { double x; double y; }

uniform Wide w;
uniform RWStructuredBuffer<double> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = w.x + w.y;
}

// IR-LABEL: struct %Wide
// IR: field(%{{.*}}, Double)
