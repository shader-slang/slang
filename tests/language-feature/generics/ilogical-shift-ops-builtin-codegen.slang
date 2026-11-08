//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

// Shifts whose operands are builtin integer types bind the builtin intrinsic shift rather than the
// generic `ILogical` shift operators, so the shift amount keeps its own type instead of being
// converted to `int` for `shl`/`shr`. A `bool` left operand is promoted to `int`.

// CHECK-LABEL: int shiftIntByUInt_0(int value_{{[0-9]+}}, uint amount_{{[0-9]+}})
// CHECK: return value_{{[0-9]+}} >> amount_{{[0-9]+}};
[noinline]
int shiftIntByUInt(int value, uint amount)
{
    return value >> amount;
}

// CHECK-LABEL: uint3 shiftVectorByScalar_0(uint3 value_{{[0-9]+}}, uint amount_{{[0-9]+}})
// CHECK: return value_{{[0-9]+}} >> amount_{{[0-9]+}};
[noinline]
uint3 shiftVectorByScalar(uint3 value, uint amount)
{
    return value >> amount;
}

// CHECK-LABEL: int3 shiftVectorByVector_0(int3 value_{{[0-9]+}}, int3 amount_{{[0-9]+}})
// CHECK: return value_{{[0-9]+}} >> amount_{{[0-9]+}};
[noinline]
int3 shiftVectorByVector(int3 value, int3 amount)
{
    return value >> amount;
}

// CHECK-LABEL: int64_t shiftWideByNarrow_0(int64_t value_{{[0-9]+}}, int16_t amount_{{[0-9]+}})
// CHECK: return value_{{[0-9]+}} << amount_{{[0-9]+}};
[noinline]
int64_t shiftWideByNarrow(int64_t value, int16_t amount)
{
    return value << amount;
}

// CHECK-LABEL: int shiftAssign_0(int value_{{[0-9]+}}, uint amount_{{[0-9]+}})
// CHECK: return value_{{[0-9]+}} << amount_{{[0-9]+}};
[noinline]
int shiftAssign(int value, uint amount)
{
    value <<= amount;
    return value;
}

// CHECK-LABEL: int shiftBool_0(bool value_{{[0-9]+}}, uint amount_{{[0-9]+}})
// CHECK: return int(value_{{[0-9]+}}) << amount_{{[0-9]+}};
[noinline]
int shiftBool(bool value, uint amount)
{
    return value << amount;
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int value = outputBuffer[0];
    outputBuffer[1] = shiftIntByUInt(value, 1u) + int(shiftVectorByScalar(uint3(value), 2u).x) +
                      shiftVectorByVector(int3(value), int3(1, 2, 3)).y +
                      int(shiftWideByNarrow(value, int16_t(3))) + shiftAssign(value, 4u) +
                      shiftBool(value != 0, 5u);
}
