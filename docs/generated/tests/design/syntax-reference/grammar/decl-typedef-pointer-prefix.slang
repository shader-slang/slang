//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable--binding-declarations
//META: doc_section_digest=34ae2014df6f94e86e6f1c0103e2b1bc5da580d48dd7ba67686044b7b740ee79
//META: purpose=`Declarator` folds a prefix `'*'` onto a `typedef` alias type, so `typedef int* IntPtr;` parses.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the prefix-'*' Declarator form in TypedefDecl: the alias
// must denote pointer-to-int. Writing through a value of the alias
// type mutates the pointee, observable in the output buffer — which
// requires the '*' to have folded onto the alias type.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

typedef int* IntPtr;

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int x = 41;
    IntPtr p = &x;
    *p = *p + 1;
    // CHECK: 42
    outputBuffer[0] = x;
}
