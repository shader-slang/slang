//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-returns-and-yields
//META: doc_section_digest=f192f41be797c74b1114b7ff57ec1c86180c872b87ad52a64a206e75eec3e27e
//META: purpose=A `return expr;` in a value-returning function terminates the block with return_val(val) where val is the returned IR value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A helper with two return paths surfaces two return_val terminators,
// each carrying its returned value as the single operand.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

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
