//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

import specialized_entry_point_module;

struct LocalValue : IValueProvider
{
    uint value;

    uint getValue()
    {
        return value;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void fill<T : __BuiltinArithmeticType, let value : int>(RWStructuredBuffer<T> output)
{
    output[0] = T(value);
}

[host]
void main()
{
    RWStructuredBuffer<float> floatOutput = RWStructuredBuffer<float>::alloc(1);
    RWStructuredBuffer<uint> uintOutput = RWStructuredBuffer<uint>::alloc(1);
    RWStructuredBuffer<uint> importedOutput = RWStructuredBuffer<uint>::alloc(1);

    dispatch(fill<float, 6>, uint3(1, 1, 1))(floatOutput);
    dispatch(fill<uint, 9>, uint3(1, 1, 1))(uintOutput);
    LocalValue localValue;
    localValue.value = 27;
    dispatch(writeImportedValue<LocalValue>, uint3(1, 1, 1))(importedOutput, localValue);

    print(readBack(floatOutput, 0));
    print(readBack(uintOutput, 0));
    print(readBack(importedOutput, 0));
}

// CHECK: 6.000000
// CHECK-NEXT: 9
// CHECK-NEXT: 27
