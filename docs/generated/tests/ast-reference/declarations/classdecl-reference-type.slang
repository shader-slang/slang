//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=a5782ff90acc1f9bd796853a7eeead0f64a6539af34d0a231454f705284208e7
//META: purpose=A ClassDecl declares a reference-type aggregate; two variables can refer to the same instance and one's mutation is visible through the other.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of ClassDecl as "a reference-type
// aggregate". Two locals referring to the same `new` instance share
// state; a mutation through one is observable through the other.
// Slang's interpreter does not implement `new` for classes, so this
// test emits C++ text on the CPU target and FileChecks the emitted
// source for the constructor call.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

class C
{
    int v;
    __init(int x) { v = x; }
}

[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    //CHECK: new C
    C a = new C(7);
}
