//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Verifies an enum with explicit tag values type-checks and its cases evaluate to the assigned integers under slangi.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the enum-decl check assigns the explicit tag values
// (Red=1, Green=2, Blue=4) to each case constant. Casting cases to int and
// summing Green+Blue under slangi proves the values were checked and
// retained. CHECK pins the sum, validated locally by the interpreter.

//TEST:INTERPRET(filecheck=CHECK):

enum Color { Red = 1, Green = 2, Blue = 4 }

void main()
{
    int v = int(Color.Green) + int(Color.Blue);
    // CHECK: v=6
    printf("v=%d\n", v);
}
