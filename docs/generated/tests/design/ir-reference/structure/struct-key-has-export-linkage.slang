//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#key--structkey
//META: doc_section_digest=5b29b137b6d9c4b0dc69ab53f9cc5d592cf4a8960514db67698c83c00c81867d
//META: purpose=A field-name key is a module-scope let with an export-linkage decoration so the same field compares equal across compilation units.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies what makes a key linkable rather than merely local: each key inst
// carries an `[export("...")]` decoration recording its mangled name. The
// ordered CHECK/CHECK-NEXT groups require, for each of two fields, the export
// decoration to sit directly above the `let %name : _ = key` line it belongs
// to, and the mangled name to end in the field's own name.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;
uniform int seed;

struct Pair
{
    int lo;
    int hi;
}

[numthreads(1, 1, 1)]
void main()
{
    Pair p;
    p.lo = seed;
    p.hi = seed + 1;
    buf[0] = p.lo + p.hi;
}

// CHECK: [export("{{.*}}2lo")]
// CHECK-NEXT: [nameHint("lo")]
// CHECK-NEXT: let  %lo	: _	= key
// CHECK: [export("{{.*}}2hi")]
// CHECK-NEXT: [nameHint("hi")]
// CHECK-NEXT: let  %hi	: _	= key
