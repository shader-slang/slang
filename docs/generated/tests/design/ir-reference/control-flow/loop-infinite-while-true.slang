//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#loop
//META: doc_section_digest=41069943e707403bf4e509b55db22b8e4cad325fff0b4de1f2732b878f000ad2
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
