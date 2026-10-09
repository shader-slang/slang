//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#module-and-namespace
//META: doc_section_digest=d2c80504726d0b51640b7748ed125adb523e7071072f9cdcfe5e98a9dcd4661a
//META: purpose=The recursive declaration pass reaches a later module-scope using declaration's ScopesWired state before it checks an earlier ordinary declaration's signature, so that header can resolve the injected name.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the declaration-pass ordering around scope wiring. The function's
// return type and local both name Widget, which lives in namespace NS and is
// only reachable because of a `using namespace NS` written *after* the
// function. The recursive pass brings the using declaration to ScopesWired
// before checking the earlier function's signature. CHECK asserts 8
// round-tripping through that function.

//TEST:INTERPRET(filecheck=CHECK):

namespace NS { struct Widget { int v; }; }

Widget makeW(int n) { Widget w; w.v = n; return w; }

using namespace NS;

void main()
{
    // CHECK: r=8
    printf("r=%d\n", makeW(8).v);
}
