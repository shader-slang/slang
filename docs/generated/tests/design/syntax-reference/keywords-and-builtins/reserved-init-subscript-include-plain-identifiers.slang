//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#reserved-identifier-prefixes
//META: doc_section_digest=405c1dcbf86de884cd29e998392a217bcd5570f5aefd6ae1874ec63241cb54b8
//META: purpose=`__init`, `__subscript`, and `__include` have no underscore-free spelling in `g_parseSyntaxEntries[]`; the bare names `init`, `subscript`, and `include` are ordinary identifiers usable as function names and call expressions.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the underscore-free spellings of __init / __subscript /
// __include carry no parser-registered meaning. Free functions named
// `init`, `subscript`, and `include` are declared and then called at
// statement-head position, where a registered decl keyword would
// hijack the parse; CHECK asserts the computed value, which is only
// printed if all three parsed as plain identifiers.

//TEST:INTERPRET(filecheck=CHECK):

int init(int x)
{
    return x + 1;
}

int subscript(int x)
{
    return x * 2;
}

int include(int x)
{
    return x + 10;
}

void main()
{
    int a = init(1);      // 2
    int b = subscript(a); // 4
    int c = include(b);   // 14
    // CHECK: v=14
    printf("v=%d\n", c);
}
