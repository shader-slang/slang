//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#ptr-and-the-access-qualifier--address-space-operands
//META: doc_section_digest=0590daabe37f0725bd791f52ce8453f35ec5ecfd6f228bf4bc3a12a46ad36613
//META: purpose=A local var of struct type produces a value whose IR type is Ptr<T>.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A local var of struct type produces a value whose IR type is Ptr<T>.
// A local struct variable accessed by field address keeps its `var`
// form and the result is typed `Ptr(<StructName>)` in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

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
