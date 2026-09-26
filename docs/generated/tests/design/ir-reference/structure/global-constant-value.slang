//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=8c6eae7c2fbeb81e2aa53b0b571a66020f7197ad1677c38daf0d744e65bab956
//META: purpose=A static const int at module scope lowers to a globalConstant opcode wrapping the literal value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a module-scope `static const` keeps a dedicated opcode instead
// of being propagated into its uses at lowering time. CHECK requires
// `let %gK : Int = globalConstant(42 : Int)`; a plain `let %gK : Int = 42 : Int`
// would mean the constant lost its module-scope identity (and with it the
// linkage that lets another module refer to it).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

static const int gK = 42;

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = gK + n;
}

// CHECK: let  %gK	: Int	= globalConstant(42 : Int)
