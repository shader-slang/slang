//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-returns-and-yields
//META: doc_section_digest=f192f41be797c74b1114b7ff57ec1c86180c872b87ad52a64a206e75eec3e27e
//META: purpose=Boundary: an early `return expr;` from three-deep nested ifs produces a `return_val` terminator on the innermost block — return is a terminator on the block that hosts it, independent of surrounding ifElse depth.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A helper with three-deep nested ifs and an early return from the
// innermost arm. The early return terminates the innermost block with
// `return_val`; the outer ifElse merges fall through to a final
// `return_val(-1)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int a;
uniform int b;
uniform int c;
uniform RWStructuredBuffer<int> buf;

int helper(int x, int y, int z)
{
    if (x > 0)
    {
        if (y > 0)
        {
            if (z > 0)
            {
                return x + y + z;
            }
        }
    }
    return -1;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(a, b, c);
}

// CHECK-LABEL: func %helper
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: return_val(%{{[0-9]+}})
// CHECK: return_val(-1 : Int)
