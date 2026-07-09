//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T14:59:37Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#ir-flat-module-read-path
//META: doc_section_digest=80e97bb59ede8f0769e884020ec2c9bac33cbba14779573e137311992bc694c5
//META: purpose=deserializeFromFlatModule rebuilds parent/child links from the flat childCounts table; nested declaration structure (a struct with fields and a method, plus a generic function) survives a module write/read round-trip intact.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the read path's recursive rebuild of parent/child links:
// the on-disk form is a flat preorder instruction list plus per-inst
// child counts, so nesting only reappears if the reader reconstructs
// it correctly. The module carries two shapes that force non-trivial
// nesting in the serialized IR — a struct containing fields and a
// member function, and a generic whose inner function sits under the
// generic container. Dumping the reloaded module and finding all the
// nested symbols' name hints (the member function's hint is the
// qualified "Pair.sum") shows each child landed under a parent
// rather than being lost or flattened.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-rebuilds-nested-children.slang -o docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-rebuilds-nested-children.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-rebuilds-nested-children.slang-module

module module_roundtrip_rebuilds_nested_children;

public struct Pair
{
    public int first;
    public int second;

    public int sum()
    {
        return first + second;
    }
}

public T pickFirst<T>(T x, T y, bool takeFirst)
{
    if (takeFirst)
        return x;
    return y;
}

// CHECK-DAG: [nameHint("Pair")]
// CHECK-DAG: [nameHint("first")]
// CHECK-DAG: [nameHint("second")]
// CHECK-DAG: [nameHint("Pair.sum")]
// CHECK-DAG: [nameHint("pickFirst")]
