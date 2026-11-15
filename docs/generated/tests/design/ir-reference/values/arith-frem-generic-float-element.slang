//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:26+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=156280c3625197a4ed3b683ed5771a1ea94cd998308b23695241f7f96edbb6a7
//META: purpose=`%` on an abstract generic floating-point type parameter still lowers to `frem`, because the floating-point decision is made at check time rather than from the lowered operand type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the refreshed `frem` row: `lowerBuiltinOperatorExpr` picks
// `frem` from `BuiltinOperatorExpr::elementTypeIsFloatingPoint`, which
// the checker resolves up front because inside a generic the element
// type is still an abstract parameter during lowering. The generic
// `rem<T>` uses `%` on `T : __BuiltinFloatingPointType`; CHECK asserts
// the generic function body contains `frem` over the parameter type
// and no `irem`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform RWStructuredBuffer<float> buf;

[noinline]
T rem<T : __BuiltinFloatingPointType>(T a, T b) { return a % b; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = rem<float>(buf[0], buf[1]);
}

// IR-LABEL: func %rem
// IR-NOT: irem
// IR: : %T{{[A-Za-z0-9_]*}}{{[[:space:]]}}= frem(%a, %b)
