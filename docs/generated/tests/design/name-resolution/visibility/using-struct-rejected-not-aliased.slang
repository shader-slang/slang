//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=b2029736d63943de091f689a9a0ed7f6ade82400bc257a00a34b24d27b223f09
//META: purpose=A `using` declaration only brings a namespace-like container into scope, so `using SomeStruct;` is rejected with expected-a-namespace (E30061) rather than aliasing the struct.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the reason there is no `using`-specific visibility rejection: a
// `using` cannot name an individual decl at all, so it can never be the route
// by which a less-visible member is re-exported. Naming a struct is the shape
// that makes the point — if `using` aliased types this would be accepted and a
// per-decl visibility check would be needed. The CHECK pins E30061 with the
// message text naming the resolved type, which shows the argument was
// resolved and then rejected for not being namespace-like.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct SomeStruct
{
    int v;
}

namespace N
{
    using SomeStruct;
//CHECK:  ^^^^^^^^^^ expected a namespace
//CHECK:  ^^^^^^^^^^ expected a namespace, got a 'typeof(SomeStruct)'
// CHECK: E30061
}

void main()
{
    printf("done\n");
}
