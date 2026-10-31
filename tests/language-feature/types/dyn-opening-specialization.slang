//TEST:SIMPLE(filecheck=CHECK): -std legacy -target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECK): -std 2025 -target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECK): -std 2026 -target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CHECK): -std 202c -target hlsl -stage compute -entry main

interface IFoo
{
    int get();
}
struct Foo : IFoo
{
    int value;
    int get()
    {
        return value;
    }
}

int evaluate<T : IFoo>(T value)
{
    return value.get();
}
int helper(dyn IFoo value)
{
    if (value is IFoo)
        return evaluate(value);
    return -1;
}

RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void main()
{
    Foo value = {7};
    outputBuffer[0] = helper(value);
}

// Both functions specialize to the known payload type across the boxed helper parameter.
// This must remain possible even in language versions that restrict dynamic specialization.
// CHECK: int evaluate_0(Foo_0
// CHECK: int helper_0(Foo_0
// CHECK: return evaluate_0(
