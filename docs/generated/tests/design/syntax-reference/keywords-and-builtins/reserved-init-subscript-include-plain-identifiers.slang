//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T00:00:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#reserved-identifier-prefixes
//META: doc_section_digest=80647084abaebbe609d96190648e92969798dc148e077cfaa14fde905e6b2ed2
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
