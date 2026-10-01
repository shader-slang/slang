//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=8c6eae7c2fbeb81e2aa53b0b571a66020f7197ad1677c38daf0d744e65bab956
//META: purpose=A string literal fed to getStringHash is collected into the module-scope global_hashed_string_literals aggregate.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the hashed-string-literal pool is a real module-scope inst and
// names the literals it holds. `getStringHash("structure-bundle")` is the
// surface that contributes to the pool; CHECK requires the aggregate to appear
// with that literal as an operand, so a change that dropped the pool or moved
// the literal elsewhere fails.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(getStringHash("structure-bundle")) + seed;
}

// CHECK: global_hashed_string_literals("structure-bundle")
