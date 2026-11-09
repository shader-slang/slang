//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[noinline]
float2 evaluateAngles(float2 y, float2 x)
{
    return asin(y) + atan2(y, x);
}

[noinline]
uint shiftValue(uint value, uint amount)
{
    return value << amount;
}

[noinline]
bool reduceVector(float3 value)
{
    return any(value);
}

[host]
void main(uint value, uint amount, float2 y, float2 x, float3 vectorValue)
{
    print(evaluateAngles(y, x)[0]);
    print(shiftValue(value, amount));
    print(reduceVector(vectorValue));
}

// CHECK-DAG: OpAsin
// CHECK-DAG: OpAtan2
// CHECK-DAG: OpLsh
// CHECK-DAG: OpAny
