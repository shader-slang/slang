//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=The doc's comprehensive ITest/TestClass example compiles and all requirement types (static const, static method, property, constructor, member function, call operator, subscript, associated types) are callable and return expected values.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that all requirement kinds an interface may declare — static const,
// static method, property, constructor, instance method, default method, call
// operator, subscript, and associated types — are simultaneously satisfiable
// and callable on a single conforming struct. CHECK asserts all computed values.

//TEST:INTERPRET(filecheck=CHECK):

interface IReq
{
}

interface ITest
{
    static const int staticDataMember;
    static float staticMethod(int a);

    property testProp : float
    {
        get; set;
    }

    __init(float f);

    float someMethod(int a);

    float someMethodWithDefaultImplementation(int a)
    {
        return testProp + float(a);
    }

    float operator()(uint x, uint y);
    __subscript(uint i0) -> float { get; set; }

    associatedtype AssocType;
    associatedtype AssocTypeWithRequirement : IReq;
}

struct TestClass : ITest
{
    static const int staticDataMember = 5;

    static float staticMethod(int a)
    {
        return float(a) * float(a);
    }

    float propUnderlyingValue;
    float arr[10];

    __init(float f)
    {
        propUnderlyingValue = f + 1.0f;
        for (uint i = 0; i < 10; i++)
            arr[i] = 0.0f;
    }

    property testProp : float
    {
        get { return propUnderlyingValue - 1.0f; }
        set(float newVal) { propUnderlyingValue = newVal + 1.0f; }
    }

    float someMethod(int64_t a)
    {
        return float(a) * propUnderlyingValue;
    }

    float operator()(uint x, uint y)
    {
        return float(x * y);
    }

    __subscript(uint i0) -> float
    {
        get { return arr[i0]; }
        set { arr[i0] = newValue; }
    }

    typealias AssocType = int;

    struct AssocTypeWithRequirement : IReq
    {
    }
}

void main()
{
    TestClass obj = TestClass(3.0f); // propUnderlyingValue = 4.0f

    // staticDataMember == 5
    int sdm = TestClass.staticDataMember;

    // staticMethod(3) = 9.0
    float sm = TestClass.staticMethod(3);

    // testProp getter: propUnderlyingValue - 1 = 3.0f
    float prop = obj.testProp;

    // someMethod(2): 2 * 4.0 = 8.0
    float meth = obj.someMethod(2);

    // default implementation: testProp + 1 = 3.0 + 1 = 4.0
    float defImpl = obj.someMethodWithDefaultImplementation(1);

    // call operator: 3 * 4 = 12
    float callOp = obj(3, 4);

    // subscript write and read
    obj[2] = 7.5f;
    float sub = obj[2];

    //CHECK: sdm=5 sm=9.0 prop=3.0 meth=8.0 defImpl=4.0 callOp=12.0 sub=7.5
    printf("sdm=%d sm=%.1f prop=%.1f meth=%.1f defImpl=%.1f callOp=%.1f sub=%.1f\n",
           sdm, sm, prop, meth, defImpl, callOp, sub);
}
