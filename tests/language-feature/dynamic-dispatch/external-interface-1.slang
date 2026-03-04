//TEST:SIMPLE(filecheck=CHECK): -target hpp -conformance "A:I=0" -conformance "B:I=1" -line-directive-mode none

[anyValueSize(64)]
interface I 
{
    float calc(float);
};

//CHECK: Tuple_{{[0-9]+}}
//CHECK-NEXT: {
//CHECK-NEXT:      Vector<uint32_t, 2>
//CHECK-NEXT:      Vector<uint32_t, 2>
//CHECK-NEXT:      AnyValue64
//CHECK-NEXT: };

struct A : I 
{
    float4 data;

    float calc(float x) 
    {
        return data.x + x;
    }
};

struct B : I
{
    float4 data;

    float calc(float x) 
    {
        return data.y * x;
    }
};

struct C 
{
    int4 value;
};

export __extern_cpp void copy(I *dest) 
{
    *dest = createDynamicObject<I, C>(0, {});
}

export __extern_cpp float calc(I *dest) 
{
    return dest->calc(10.f);
}

// needs dummy entrypoint as no output is generated to stdout
// otherwise it only works when slangc -o output.hpp is specified
[numthreads(1, 1, 1)]
void dummyEntrypoint_(uint3 dispatchThreadID: SV_DispatchThreadID) { }