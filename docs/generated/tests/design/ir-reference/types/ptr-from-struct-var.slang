//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#ptr-and-the-access-qualifier--address-space-operands
//META: doc_section_digest=4eb81c51191334ef3e30616d7d16be9c32be2a38ef15ecc8cf34a2886b8386ae
//META: purpose=A local var of struct type produces a value whose IR type is Ptr<T>.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A local var of struct type produces a value whose IR type is Ptr<T>.
// A local struct variable accessed by field address keeps its `var`
// form and the result is typed `Ptr(<StructName>)` in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

struct Foo { int x; int y; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Foo f;
    f.x = a;
    f.y = a + 1;
    buf[0] = f.x + f.y;
}

// IR-LABEL: func %main
// IR: let {{.*}}%f{{.*}}: Ptr(%Foo){{.*}}= var
