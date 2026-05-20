//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#loop
//META: doc_section_digest=04d0daf0564c731241b1f5e0a49a1a7dc27737ec3435d045559640aef919740e
//META: purpose=Boundary: a `while (true)` loop whose only exit is an internal `return` lowers to `loop` with the body's continueLabel pointing at the body itself; the break block is reached only via the inner return path and is terminated by `unreachable`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An infinite loop with a single conditional return exits only through
// the inner `return s;`. The structured `loop` terminator names a
// break label whose block — never reached — is terminated by
// `unreachable`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

int helper(int x)
{
    int s = x;
    while (true)
    {
        s = s + 1;
        if (s > 100) return s;
    }
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(n);
}

// CHECK-LABEL: func %helper
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %x)
// CHECK: return_val(%{{[a-zA-Z0-9_]+}})
// CHECK: unreachable
