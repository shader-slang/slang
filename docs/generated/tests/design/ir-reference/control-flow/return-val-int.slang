//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-returns-and-yields
//META: doc_section_digest=7a22c485cd43a8f37c76060d034a3035010f4828abcf759c51b28677698d3453
//META: purpose=A `return expr;` in a value-returning function terminates the block with return_val(val) where val is the returned IR value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A helper with two return paths surfaces two return_val terminators,
// each carrying its returned value as the single operand.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int x)
{
    if (x > 0)
        return x;
    return -x;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(a);
}

// CHECK-LABEL: func %helper
// CHECK: return_val(%x)
// CHECK: return_val(%{{[0-9]+}})
