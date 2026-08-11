//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=078844f9de8080a010d3c70d4af44f40667b8ce158b13e5adda01e318abce99c
//META: purpose=A C-style function declarator such as `float f<T>(T x)` introduces a GenericDecl, because the `<` after the declarator is itself what selects the function branch.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the non-keyword-led half of the "generic declarations are
// unambiguous" claim. `scale` and `firstArg` are written with a leading return
// type rather than a `func` keyword, so the parser only learns they are
// functions when it reaches the `<`. If that `<` had been read as a comparison
// the declaration would not parse at all; if it were read as a parameter list
// but no GenericDecl were introduced, the explicit `scale<float>` /
// `firstArg<int>` instantiations below would not resolve. Two distinct
// instantiations of each prove the collected parameters really became generic
// parameters.

//TEST:INTERPRET(filecheck=CHECK):

float scale<T : IArithmetic>(T x, float k) { return k; }

int firstArg<T>(T a, T b) { return 1; }

void main()
{
    float a = scale<int>(3, 2.5);
    float b = scale<float>(3.0, 4.0);
    int c = firstArg<int>(4, 5) + firstArg<float>(1.0, 2.0);
    // CHECK: total=8
    printf("total=%d\n", int(a) + int(b) + c);
}
