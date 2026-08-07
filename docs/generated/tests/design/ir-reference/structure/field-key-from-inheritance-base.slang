//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#struct-internals
//META: doc_section_digest=b9a3eeb4f30459f951dba340f8e24ccd1d1bf7aa9598b948a2a6457d2453d958
//META: purpose=The leading field of a derived struct comes from an InheritanceDecl rather than a member VarDecl, embedding the base type under an inheritance-named key.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second AST origin listed for `field`. `struct Derived : Base`
// declares only `d`, yet the ordered CHECKs require the struct body to open
// with `field(%base, %Base)` before `field(%d, Int)` — the base is embedded as
// the first member. The final CHECK pins that `%base`'s key carries an
// inheritance-derived linkage name, distinguishing it from a member key.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

struct Base { int b; }
struct Derived : Base { int d; }

[numthreads(1, 1, 1)]
void main()
{
    Derived v;
    v.b = seed;
    v.d = seed + 1;
    buf[0] = v.b + v.d;
}

// CHECK: struct %Derived	: Type
// CHECK: field(%base, %Base)
// CHECK-NEXT: field(%d, Int)
// CHECK: [export("key__{{.*}}xinheritance")]
// CHECK-NEXT: let  %base	: _	= key
