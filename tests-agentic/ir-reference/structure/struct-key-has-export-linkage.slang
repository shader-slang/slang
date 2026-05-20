//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#key-structkey
//META: doc_section_digest=2117d4dbe3c230380c22066ed6bf928ab2ce23e921d10d25ad32ec7e6e134733
//META: purpose=A field-name key is a top-level let with an export-linkage decoration so the same field compares equal across compilation units.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc, `StructKey` is the identity of a struct field; unlike a
// string name it is a globally-linkable IR value. The dump confirms
// this by emitting each key as a top-level `let %name : _ = key`
// carrying an `[export("...")]` decoration that records its mangled
// linkage name.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

struct Pair
{
    int lo;
    int hi;
};

[numthreads(1, 1, 1)]
void main()
{
    Pair p;
    p.lo = 1;
    p.hi = 2;
    buf[0] = p.lo + p.hi;
}

// CHECK: [export("{{.*}}2lo")]
// CHECK-NEXT: [nameHint("lo")]
// CHECK-NEXT: let  %lo	: _	= key
// CHECK: [export("{{.*}}2hi")]
// CHECK-NEXT: [nameHint("hi")]
// CHECK-NEXT: let  %hi	: _	= key
