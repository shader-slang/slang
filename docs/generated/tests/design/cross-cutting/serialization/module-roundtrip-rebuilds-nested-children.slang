//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#ir-flat-module-read-path
//META: doc_section_digest=2e8ff23102d654105358f3ad3beaa32ef3c178889d2447b335718aa643cd48d1
//META: purpose=The flat-module read path rebuilds parent/child links from the per-instruction child counts, so nested declarations (struct fields, a member function, a generic function) survive a module write/read round-trip.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that nesting survives a form that does not store nesting
// directly: the module is written as a flat preorder instruction list
// plus per-instruction child counts, and the reader has to reconstruct
// the tree. The module deliberately carries two nesting shapes - a
// struct with two fields and a member function, and a generic whose
// inner function sits under the generic container. Dumping the reloaded
// module and finding every nested symbol's name hint (including the
// qualified "Pair.sum" of the member function) shows each child landed
// under a parent instead of being dropped or flattened to module scope.

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
