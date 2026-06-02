//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#constructor
//META: doc_section_digest=508a36baffb3b3fb0a209db200b9264ba975f5e186451437b5114720d356e55e
//META: purpose=A user-defined no-parameter `__init` constructor is invoked on plain `TestClass obj;` instantiation; members take the constructor-assigned values.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that declaring an object without an initializer list invokes the no-parameter `__init`.
// obj1 is declared bare; the constructor sets a=1 and b=2. CHECK confirms those values are
// live after construction, proving the no-arg path ran.

//TEST:INTERPRET(filecheck=CHECK):

struct TestClass
{
    int a, b;

    __init()
    {
        a = 1;
        b = 2;
    }
}

void main()
{
    TestClass obj1;
    // CHECK: 1
    // CHECK: 2
    printf("%d\n%d\n", obj1.a, obj1.b);
}
