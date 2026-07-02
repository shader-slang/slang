// used-uniforms-funccall.slang

// Tests the function call escape. The pass does no interprocedural
// analysis, so when a uniform value flows into a call it cannot peel, it
// widens to the whole sub region instead of narrowing to the fields the
// callee actually touches. Here the entire Inner struct is passed by
// value and the callee reads only one field, yet the recorded range must
// cover the whole struct (a narrowing pass would wrongly record just the
// first field). A sibling field that is never passed stays out of the
// ranges.

//TEST:REFLECTION:-stage compute -entry main -target hlsl

struct Inner
{
    float a;
    float b;
    float c;
    float d;
}

float helper(Inner s)
{
    return s.a;
}

cbuffer CB
{
    Inner  passed;
    float4 unused;
}

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = helper(passed);
}
