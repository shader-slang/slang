//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang legacy -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 2025 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 2026 -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -xslang -std -xslang 202c -output-using-type

// Regression for https://github.com/shader-slang/slang/issues/13996. The specializations
// `IBase<int>` and `IBase<float>` share one inheritance declaration. For each conforming type,
// the compiler must create only one canonical `IRoot` witness table.
interface IRoot
{
    int read();
}

interface IBase<T> : IRoot {}
interface IDerived : IBase<int>, IBase<float> {}

struct SharedValue : IDerived
{
    int payload;
    int read() { return payload; }
}

int readRoot<V : IRoot>(V value)
{
    return value.read();
}

int readBase<T, V : IBase<T>>(V value)
{
    return readRoot(value);
}

int readDerived<V : IDerived>(V value)
{
    return readBase<int, V>(value) + readBase<float, V>(value);
}

// When checking `V : IParamDerived<T>`, we construct inherited path tables using the
// specialized interface reference `IParamDerived<T>`. For `T = float`, the base order is
// `IBase<float>, IBase<int>`, reversing the two specializations in `IDerived`.
interface IParamDerived<T> : IBase<T>, IBase<int> {}

struct GenericValue : IParamDerived<float>
{
    int payload;
    int read() { return payload; }
}

int readParamDerived<T, V : IParamDerived<T>>(V value)
{
    return readBase<T, V>(value) + readBase<int, V>(value);
}

// The compiler must create distinct witness tables for `IGenericRoot<int>` and
// `IGenericRoot<float>`. Overloaded requirements make an incorrect selection observable,
// including when a type argument is written through an alias.
interface IGenericRoot<T>
{
    T read(T input);
}

interface IGenericBase<T> : IGenericRoot<T> {}
typealias FloatAlias = float;
interface IDistinctDerived : IGenericBase<int>, IGenericBase<FloatAlias> {}

struct DistinctValue : IDistinctDerived
{
    int read(int input) { return input + 10; }
    float read(float input) { return input + 20.0; }
}

T readTypedRoot<T, V : IGenericRoot<T>>(V value, T input)
{
    return value.read(input);
}

int readDistinctDerived<V : IDistinctDerived>(V value)
{
    return readTypedRoot<int, V>(value, 3) + int(readTypedRoot<float, V>(value, 4.0));
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    SharedValue shared = SharedValue(7);
    outputBuffer[0] = readRoot(shared);
    outputBuffer[1] = readBase<int, SharedValue>(shared);
    outputBuffer[2] = readBase<float, SharedValue>(shared);
    outputBuffer[3] = readDerived(shared);

    GenericValue generic = GenericValue(9);
    outputBuffer[4] = readRoot(generic);
    outputBuffer[5] = readParamDerived<float, GenericValue>(generic);

    DistinctValue distinct = DistinctValue();
    outputBuffer[6] = readTypedRoot<int, DistinctValue>(distinct, 3);
    outputBuffer[7] = int(readTypedRoot<FloatAlias, DistinctValue>(distinct, 4.0));
    outputBuffer[8] = readDistinctDerived(distinct);
}

// CHECK: type: int32_t
// CHECK-NEXT: {{^7$}}
// CHECK-NEXT: {{^7$}}
// CHECK-NEXT: {{^7$}}
// CHECK-NEXT: {{^14$}}
// CHECK-NEXT: {{^9$}}
// CHECK-NEXT: {{^18$}}
// CHECK-NEXT: {{^13$}}
// CHECK-NEXT: {{^24$}}
// CHECK-NEXT: {{^37$}}
