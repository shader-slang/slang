//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable-binding-declarations
//META: doc_section_digest=b0a9ef0c603cd408e940f93b864d2b441f72e4bf4f04f967c2f2f359024af70e
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
