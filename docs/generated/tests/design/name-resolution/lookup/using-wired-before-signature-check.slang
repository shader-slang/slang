//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#module-and-namespace
//META: doc_section_digest=66e95bef72b71123f8c26c9d474bc27387f02bb7f3d1da7825ec57b318af2eef
//META: purpose=Sibling-scope wiring runs as its own check phase before any signature is checked, so a declaration header resolves a name injected by a using declaration that appears later in the file.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the phase-ordering claim about scope wiring. The function's
// return type and local both name Widget, which lives in namespace NS
// and is only reachable because of a `using namespace NS` written
// *after* the function. If wiring ran lazily at the point of use, the
// header would fail to resolve. CHECK asserts 8 round-tripping through
// that function.

//TEST:INTERPRET(filecheck=CHECK):

namespace NS { struct Widget { int v; }; }

Widget makeW(int n) { Widget w; w.v = n; return w; }

using namespace NS;

void main()
{
    // CHECK: r=8
    printf("r=%d\n", makeW(8).v);
}
