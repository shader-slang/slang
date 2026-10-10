//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=NOBARE): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// WGSL template-list discovery treats `ident <` as the start of a template list when a `>` or
// `>=` follows later in the same comma-separated list, so an argument list such as
// `f(a < b, a > b)` parses as `f(a<b, a> b)` and the shader is rejected. The WGSL emitter
// wraps every relational comparison in parentheses so that argument lists stay unambiguous.

//TEST_INPUT:ubuffer(data=[1.0 2.0], stride=4):name=floatInput
RWStructuredBuffer<float> floatInput;

//TEST_INPUT:ubuffer(data=[3], stride=4):name=intInput
RWStructuredBuffer<int> intInput;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Flags
{
    bool lt;
    bool gt;
};

[noinline]
int pick(bool x, bool y)
{
    return x ? 1 : (y ? 2 : 3);
}

bool2 compareInConstructor(float a, float b)
{
    // WGSL-DAG: vec2<bool>((a{{[_0-9]+}} < b{{[_0-9]+}}), (a{{[_0-9]+}} > b{{[_0-9]+}}))
    return bool2(a < b, a > b);
}

int compareInCall(float a, float b)
{
    // WGSL-DAG: pick{{[_0-9]+}}((a{{[_0-9]+}} < b{{[_0-9]+}}), (a{{[_0-9]+}} >= b{{[_0-9]+}}))
    return pick(a < b, a >= b);
}

int compareWithLiteral(int i)
{
    // WGSL-DAG: pick{{[_0-9]+}}((i{{[_0-9]+}} < i32(2)), (i{{[_0-9]+}} > i32(1)))
    return pick(i < 2, i > 1);
}

Flags compareInStructInit(float a, float b)
{
    // WGSL-DAG: Flags{{[_0-9a-z]+}}((a{{[_0-9]+}} < b{{[_0-9]+}}), (a{{[_0-9]+}} > b{{[_0-9]+}}))
    Flags f = { a < b, a > b };
    return f;
}

// No comparison may be followed directly by the `,` of an enclosing argument list.
// NOBARE-NOT: {{ (<|>|<=|>=) [^,()]+(\([^()]*\))?, }}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float a = floatInput[0];
    float b = floatInput[1];

    bool2 c = compareInConstructor(a, b);
    Flags f = compareInStructInit(a, b);

    // BUF: 10
    outputBuffer[0] = (c.x ? 10 : 0) + (c.y ? 1 : 0);
    // BUF-NEXT: 1
    outputBuffer[1] = compareInCall(a, b);
    // BUF-NEXT: 2
    outputBuffer[2] = compareWithLiteral(intInput[0]);
    // BUF-NEXT: 10
    outputBuffer[3] = (f.lt ? 10 : 0) + (f.gt ? 1 : 0);
}
