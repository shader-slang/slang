//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=100b91a908217e68795cbc9dac7393e0325718d4c3df831fe3616ee38716a45a
//META: purpose=A mutable function-static local is hoisted to a module-scope global_var with a Ptr-typed result, not to a function-local var.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second AST origin of `global_var`: a `static` local inside a
// function body. The local `c` is declared inside `counter()` yet CHECK
// requires it to appear as a module-scope `global_var %c : Ptr(Int)` — outside
// any `func` — proving the declaration was hoisted rather than lowered to a
// block-local `var`. The uniform read keeps the store live.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int step;
uniform RWStructuredBuffer<int> buf;

int counter()
{
    static int c = 0;
    c = c + step;
    return c;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = counter();
}

// CHECK: [nameHint("c")]
// CHECK-NEXT: global_var %c	: Ptr(Int);
