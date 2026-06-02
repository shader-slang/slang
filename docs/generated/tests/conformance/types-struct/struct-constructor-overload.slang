//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#constructor
//META: doc_section_digest=508a36baffb3b3fb0a209db200b9264ba975f5e186451437b5114720d356e55e
//META: purpose=Multiple `__init` constructors may be defined; overload resolution picks the most appropriate one based on initializer list argument count and types.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "More than one constructors may be defined in which case overload resolution is
// performed to select the most appropriate constructor given the initialization parameters."
// TestClass has __init(), __init(int), and __init(int,int); each should be selected by overload resolution.

//TEST:INTERPRET(filecheck=CHECK):

struct TestClass
{
    int a, b;

    __init()
    {
        a = 1;
        b = 2;
    }

    __init(int _a)
    {
        a = _a;
        b = 2;
    }

    __init(int _a, int _b)
    {
        a = _a;
        b = _b;
    }
}

void main()
{
    TestClass obj1;          // calls __init()
    TestClass obj2 = { 42 }; // calls __init(int)
    TestClass obj3 = { 42, 43 }; // calls __init(int,int)

    // CHECK: 1 2
    printf("%d %d\n", obj1.a, obj1.b);
    // CHECK: 42 2
    printf("%d %d\n", obj2.a, obj2.b);
    // CHECK: 42 43
    printf("%d %d\n", obj3.a, obj3.b);
}
