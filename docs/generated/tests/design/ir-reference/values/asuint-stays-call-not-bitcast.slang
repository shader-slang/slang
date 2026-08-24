//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=The HLSL-compatibility reinterpret builtins `asuint` / `asfloat` are not a surface for `bitCast`: they stay `call` instructions carrying a `targetIntrinsic` decoration.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the distinction between the two bit-reinterpret spellings a
// reader is likely to confuse. `bit_cast<T>(x)` becomes the `bitCast`
// opcode (covered by conversion-bitcast.slang), but `asuint` /
// `asfloat` are declared as target-assembly intrinsics rather than
// as IR opcodes, so nothing rewrites them during lowering. CHECK
// asserts that the call survives as `call %asuint(...)` /
// `call %asfloat(...)` inside `main`, that the callee declaration
// carries a `targetIntrinsic` decoration naming the HLSL spelling, and
// that no `bitCast` inst appears in the function at all.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float f = float(tid.x) * 1.5;
    uint bits = asuint(f);
    buf[0] = bits + asuint(asfloat(tid.y));
}

// IR-DAG: [targetIntrinsic({{.*}}"asuint")]
// IR-DAG: [targetIntrinsic({{.*}}"asfloat")]
// IR-LABEL: func %main
// IR: call %asuint(
// IR-NOT: bitCast
