//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=07908c83d735faee5ec2bad31d6fd52d29e333c23af2fd0650793137f330b2ad
//META: purpose=A user-named declaration carries a `nameHint` decoration in the IR carrying its identifier name.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `NameHintDecoration`/`IRNameHintDecoration` row
// ("Carries an identifier name for debug / link output"). The
// user-named helper `myHelper` carries a `[nameHint("myHelper")]`
// decoration in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int myHelper(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = myHelper(a);
}

// IR: [nameHint("myHelper")]
