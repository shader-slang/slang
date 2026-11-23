//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=1ab53409e2cba6b92a18270a5bfb136836158c640f0e5dc49f907805c463cf92
//META: purpose=A user-named declaration carries a `nameHint` decoration in the IR carrying its identifier name.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `NameHintDecoration`/`IRNameHintDecoration` row
// ("Carries an identifier name for debug / link output"). The
// user-named helper `myHelper` carries a `[nameHint("myHelper")]`
// decoration in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int myHelper(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = myHelper(a);
}

// IR: [nameHint("myHelper")]
