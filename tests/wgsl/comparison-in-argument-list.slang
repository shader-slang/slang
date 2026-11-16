//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOBARE): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// WGSL template-list discovery treats `ident <` as the start of a template list when a `>` or
// `>=` follows later in the same comma-separated list, so an argument list such as
// `f(a < b, a > b)` parses as `f(a<b, a> b)` and the shader is rejected (#13566). The WGSL
// emitter wraps every relational comparison in parentheses so that argument lists stay
// unambiguous.

//TEST_INPUT:ubuffer(data=[1.0 2.0 1.0], stride=4):name=floatInput
RWStructuredBuffer<float> floatInput;

//TEST_INPUT:ubuffer(data=[2], stride=4):name=intInput
RWStructuredBuffer<int> intInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Flags
{
    bool first;
    bool second;
};

int encode(bool x, bool y)
{
    return (x ? 2 : 0) + (y ? 1 : 0);
}

[noinline]
int pick(bool x, bool y)
{
    return encode(x, y);
}

[noinline]
bool2 compareInConstructor(float a, float b)
{
    // WGSL-DAG: vec2<bool>((a{{[_0-9]+}} < b{{[_0-9]+}}), (a{{[_0-9]+}} > b{{[_0-9]+}}))
    return bool2(a < b, a > b);
}

[noinline]
int compareInCall(float a, float b)
{
    // WGSL-DAG: pick{{[_0-9]+}}((a{{[_0-9]+}} < b{{[_0-9]+}}), (a{{[_0-9]+}} >= b{{[_0-9]+}}))
    return pick(a < b, a >= b);
}

[noinline]
int compareWithLiteral(int i)
{
    // WGSL-DAG: pick{{[_0-9]+}}((i{{[_0-9]+}} < i32(2)), (i{{[_0-9]+}} > i32(2)))
    return pick(i < 2, i > 2);
}

// An initializer list is lowered to a call of the struct's synthesized constructor.
[noinline]
Flags compareInInitializerList(float a, float b)
{
    // WGSL-DAG: Flags{{[_0-9a-z]+}}((a{{[_0-9]+}} < b{{[_0-9]+}}), (a{{[_0-9]+}} > b{{[_0-9]+}}))
    Flags f = { a < b, a > b };
    return f;
}

// WGSL has no vector `&&`/`||`, so the emitter lowers them to `select(...)` argument lists with
// the right-hand operand first. The operands are ordered so that a `<` precedes a `>`/`>=` there.
[noinline]
bool2 compareInVectorAnd(float2 u, float2 v, float2 w)
{
    // WGSL-DAG: select(vec2<bool>(false), (u{{[_0-9]+}} < w{{[_0-9]+}}), (u{{[_0-9]+}} >= v{{[_0-9]+}}))
    return (u >= v) && (u < w);
}

[noinline]
bool2 compareInVectorOr(float2 u, float2 v, float2 w)
{
    // WGSL-DAG: select((u{{[_0-9]+}} < w{{[_0-9]+}}), vec2<bool>(true), (u{{[_0-9]+}} > v{{[_0-9]+}}))
    return (u > v) || (u < w);
}

// A bare comparison followed by a list `,` is the shape that lets its `<` pair with a later `>`.
// The pattern matches a spaced relational operator (the emitter prints ` op `, which excludes
// `vec2<bool>` and `->`) whose right-hand operand, skipping up to two levels of nested
// parentheses, runs into a `,`. It has its own run so that it scans the whole output rather
// than only the text after the last WGSL-DAG match.
// NOBARE-NOT: {{ (<|>|<=|>=) ([^,;()]|\(([^()]|\([^()]*\))*\))+, }}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float a = floatInput[0];
    float b = floatInput[1];
    float c = floatInput[2];
    float2 u = float2(a, b);
    float2 v = float2(c, a);

    bool2 ctor = compareInConstructor(a, c);
    Flags flags = compareInInitializerList(a, c);
    bool2 vecAnd = compareInVectorAnd(u, v, float2(b, b));
    bool2 vecOr = compareInVectorOr(u, v, float2(c, b + 1.0));

    // Operands are chosen so that swapping `<` with `<=`, or `>` with `>=`, changes the result.
    // BUF: 0
    outputBuffer[0] = encode(ctor.x, ctor.y);
    // BUF-NEXT: 1
    outputBuffer[1] = compareInCall(a, c);
    // BUF-NEXT: 0
    outputBuffer[2] = compareWithLiteral(intInput[0]);
    // BUF-NEXT: 0
    outputBuffer[3] = encode(flags.first, flags.second);
    // BUF-NEXT: 2
    outputBuffer[4] = encode(vecAnd.x, vecAnd.y);
    // BUF-NEXT: 1
    outputBuffer[5] = encode(vecOr.x, vecOr.y);
}
