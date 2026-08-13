//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A FuncDecl that returns a multi-field struct lowers to an IRFunc whose function type names the struct as its return type; the body builds the struct in an IRVar and returns its loaded value.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: function-return type edge — returning a composite
// struct rather than a scalar. The IR `func %make : Func(%Big)`
// declares Big as return type. Body uses `get_field_addr` + `store`
// to fill the local then returns it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Big { int a; int b; int c; int d; }

uniform int x;
uniform RWStructuredBuffer<int> buf;

Big make()
{
    Big b;
    b.a = x; b.b = x; b.c = x; b.d = x;
    return b;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Big r = make();
    buf[0] = r.a + r.b + r.c + r.d;
}

// CHECK-LABEL: func %make
// CHECK: Func(%Big)
// CHECK: get_field_addr
// CHECK: return_val
