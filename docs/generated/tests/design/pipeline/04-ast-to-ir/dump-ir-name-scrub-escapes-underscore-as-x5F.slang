//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=00fef3057358760d4225321cd326293f0cb1ef37b4a7e3a927e9a7448fe83891
//META: purpose=Names in a `-dump-ir` listing go through `scrubName`, which escapes every non-alphanumeric byte as `x` plus its hex value, so an underscore prints as `x5F` and `IFoo_x` prints as `IFoox5Fx`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the dump-name scrubbing rule. The interface `IFoo_x` and its
// requirement `zero_val` contain underscores (0x5F); the dump spells them
// `IFoox5Fx` and `...x5Fzerox5Fval`. The `[nameHint(...)]` decoration keeps
// the original spelling, so the CHECKs pin both forms.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -stage compute -entry main

interface IFoo_x { int zero_val(); }
struct S_t : IFoo_x { int zero_val() { return 0; } }
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    S_t s;
    buf[0] = s.zero_val();
}

// CHECK: [nameHint("IFoo_x")]
// CHECK: let %IFoox5Fx{{.*}}: Type = interface(
