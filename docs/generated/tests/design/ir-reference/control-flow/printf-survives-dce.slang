//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#moving-and-deleting-control-flow-instructions
//META: doc_section_digest=1a35f3e6e880b02476bb7fbe3db3c3bfa0fe959a20d8b9005e5bd64d9b003102
//META: purpose=Printf survives dead-code elimination at -O3 because it reports as possibly-side-effecting, while a genuinely unused arithmetic value next to it is removed.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the practical consequence of Printf having no case in the
// side-effect classifier and therefore landing in the conservative arm:
// nothing consumes the print, so a purely value-based DCE would delete it.
// The shader is compiled at -O3 with an unused arithmetic expression sitting
// right next to the call; the CHECK asserts the printf is still in the
// emitted HLSL while CHECK-NOT asserts the unused multiply is gone, so the
// test distinguishes "DCE did not run" from "DCE ran and kept the printf".

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -O3 -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int unusedValue = n * 3 + 7;
    printf("kept %d\n", n);
    buf[0] = n;
}

// CHECK-LABEL: void main()
// CHECK-NOT: {{[*]}} int(3)
// CHECK: printf("kept %d\n"
