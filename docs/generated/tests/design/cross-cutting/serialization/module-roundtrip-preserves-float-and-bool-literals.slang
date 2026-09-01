//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#ir-flat-module-read-path
//META: doc_section_digest=10281c5d9eb08c66e1e13b05f5697bb685e45b6016f92a04519f1ba6030c0993
//META: purpose=The flat-module literals table stores the raw bits of every bool and float constant, so 0.0, -0.0, the largest finite float, +inf and both bool values reload unchanged.
//META: intent=boundary
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the float and bool edge of the raw-bits claim. Storing bits
// rather than a decimal rendering is what keeps -0.0 distinct from 0.0
// and keeps +inf from degrading to the largest finite value, so those
// three values plus the largest finite float are exactly where a lossy
// encode/decode would show. `1.0f / 0.0f` is folded to +inf before
// serialization, which is how the infinity gets into the literals table
// without an unrepresentable-literal warning. The bool pair is included
// because bools share that same table.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-float-and-bool-literals.slang -o docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-float-and-bool-literals.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-float-and-bool-literals.slang-module

module module_roundtrip_preserves_float_and_bool_literals;

public float positiveZero()
{
    return 0.0f;
}

public float negativeZero()
{
    return -0.0f;
}

public float largestFinite()
{
    return 3.402823466e+38f;
}

public float positiveInfinity()
{
    return 1.0f / 0.0f;
}

public bool alwaysTrue()
{
    return true;
}

public bool alwaysFalse()
{
    return false;
}

// CHECK-DAG: return_val(0 : Float)
// CHECK-DAG: return_val(-0 : Float)
// CHECK-DAG: return_val(3.40282e+38 : Float)
// CHECK-DAG: return_val(inf : Float)
// CHECK-DAG: return_val(true)
// CHECK-DAG: return_val(false)
