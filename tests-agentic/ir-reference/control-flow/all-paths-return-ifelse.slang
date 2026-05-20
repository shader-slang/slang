//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#terminators-no-continuation
//META: doc_section_digest=d6429801087e19d2dcf039c0f032b312e1c65a39be1d3ce805c2351b2a2c8208
//META: purpose=Boundary: an if/else in which both arms `return` leaves the synthesized merge block terminated by `unreachable` — `unreachable` is not unique to switch all-arms-return.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Both arms of the if/else return, so no path reaches the synthesized
// merge block. The lowering pass terminates that merge with
// `unreachable` — the natural surface for the no-continuation
// terminator alongside the switch case already in the bundle.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int x)
{
    if (x > 0)
    {
        return x;
    }
    else
    {
        return -x;
    }
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(a);
}

// CHECK-LABEL: func %helper
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: return_val(%x)
// CHECK: unreachable
