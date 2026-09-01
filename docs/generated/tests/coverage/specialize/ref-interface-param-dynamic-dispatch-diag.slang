//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T14:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization
//META: doc_section_digest=281f505f04ffec7b524f03c8f78892260d45c554e3e06b9a86d32cfe7c6ec9c0
//META: purpose=A __ref parameter of interface type used in a dynamic-dispatch context is rejected with E52010.
//META: intent=characterization
//META: covers=source/slang/slang-ir-typeflow-specialize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the interprocedural-edge analysis: when a callee parameter has
// __ref (or __constref) direction and its type includes a dynamically-dispatched
// interface, the tagged-union representation cannot back the reference, so the
// pass emits RefParamWithInterfaceTypeInDynamicDispatch (E52010). The call site
// `mutate(f)` passes a runtime-tagged `IFoo` into the `__ref IFoo` parameter.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry computeMain -stage compute

interface IFoo { int get(); }
struct A : IFoo { int x; int get() { return x; } }
struct B : IFoo { int y; int get() { return y * 2; } }

void mutate(__ref IFoo f) {}

RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    IFoo f;
    if (tid.x == 0) { A a; a.x = 1; f = a; }
    else { B b; b.y = 2; f = b; }
    mutate(f);
    outputBuffer[0] = f.get();
}
//CHECK: E52010
//CHECK: '__ref' parameter of type 'IFoo' cannot be used in a dynamic dispatch context
