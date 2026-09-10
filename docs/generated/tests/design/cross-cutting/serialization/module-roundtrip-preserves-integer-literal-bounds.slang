//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#ir-flat-module-read-path
//META: doc_section_digest=10281c5d9eb08c66e1e13b05f5697bb685e45b6016f92a04519f1ba6030c0993
//META: purpose=The flat-module literals table stores the raw bits of every integer constant, so the extreme values 0, int MIN, int MAX, uint MAX and int64 MAX all reload with their exact values.
//META: intent=boundary
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the integer edge of the claim that constants are carried as
// raw bits in a side table rather than re-parsed on load: a sign-extend
// or a width mix-up while slicing that table shows up exactly at the
// representable extremes. The module returns 0, the most negative int,
// the largest int, the largest uint (whose top bit is set) and the
// largest int64_t; after the write/read round-trip the CHECKs require
// -dump-module to print each of those values back unchanged and with
// its original type tag.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-integer-literal-bounds.slang -o docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-integer-literal-bounds.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-integer-literal-bounds.slang-module

module module_roundtrip_preserves_integer_literal_bounds;

public int zero()
{
    return 0;
}

public int intMin()
{
    // Spelled as MAX-then-decrement so the literal itself stays in range.
    return -2147483647 - 1;
}

public int intMax()
{
    return 2147483647;
}

public uint uintMax()
{
    return 4294967295u;
}

public int64_t int64Max()
{
    return 9223372036854775807LL;
}

// CHECK-DAG: return_val(0 : Int)
// CHECK-DAG: return_val(-2147483648 : Int)
// CHECK-DAG: return_val(2147483647 : Int)
// CHECK-DAG: return_val(4294967295 : UInt)
// CHECK-DAG: return_val(9223372036854775807 : Int64)
