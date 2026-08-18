//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=29130252ebb8566d4abf275df716b28c339ea28ecaa9f518d2ddee6bfdd47125
//META: purpose=`CoreType` accepts the function-type spelling `'functype' '(' Type (',' Type)* ')' '->' Type`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `functype` keyword spelling of a function type: a
// typealias names `functype (int, int) -> int`, and a parameter of
// that type receives a named function and is invoked. The printed
// result proves the whole chain (including the functype parse)
// compiled and ran. (A local variable of the alias type is not used
// because calling through a functype-typed local is rejected by the
// checker; passing the function as a functype parameter works.)

//TEST:INTERPRET(filecheck=CHECK):

typealias BinOp = functype (int, int) -> int;

int add(int a, int b)
{
    return a + b;
}

int apply(BinOp f, int a, int b)
{
    return f(a, b);
}

void main()
{
    // CHECK: r=42
    printf("r=%d\n", apply(add, 40, 2));
}
