//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-unconditional-branches
//META: doc_section_digest=a0f708a11d81970e17fbe525500eba453951d03287c41957fe027cbfb06d85f6
//META: purpose=A catch statement lowers to the loop terminator: the guarded block and each handler become loop bodies whose break label is the post-catch join.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the CatchStmt entry in the `loop` row's AST-origin list. The
// surface program has no for/while/do-while at all, so every `loop`
// terminator in the dump must have come from the do/catch construct: the
// structured loop encoding is reused to give the guarded region a break
// label that both the success path and the handler can branch to. The
// CHECK also pins the error test the `try` expands into, which is what
// selects between the two.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

enum MyError { Fail = 7 }

int risky(int v) throws MyError
{
    if (v < 0)
        throw MyError.Fail;
    return v + 1;
}

void guarded(int v)
{
    do
    {
        buf[0] = try risky(v);
    }
    catch (e: MyError)
    {
        buf[0] = -1;
    }
}

[numthreads(1, 1, 1)]
void main()
{
    guarded(n);
}

// CHECK-LABEL: func %guarded
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: isResultError
