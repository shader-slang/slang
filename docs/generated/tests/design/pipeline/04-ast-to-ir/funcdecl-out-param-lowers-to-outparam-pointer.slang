//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A FuncDecl with an `out int y` parameter lowers to an IRFunc whose param has type `OutParam(Int)`; writes through `y` lower to `store(%y, ...)` and the call site passes the address of a local IRVar.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: function-parameter pass-by-out axis. The doc's row says
// "FuncDecl -> IRFunc"; the param-direction surface (in / out /
// inout) is observable on the IR `Func(...)` type signature. For
// `out int`, the param type is `OutParam(Int)` and writes through
// it lower to a `store`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int outHelper(int x, out int y)
{
    y = x * 2;
    return x + 1;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int o;
    int r = outHelper(a, o);
    buf[0] = r + o;
}

// CHECK-LABEL: func %outHelper
// CHECK: OutParam(Int)
// CHECK: param %y
// CHECK: store(%y, %{{.*}})
