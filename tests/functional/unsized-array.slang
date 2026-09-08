//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target hlsl -entry computeMain -stage compute -experimental-feature

import slang.functional;

int increment(int value)
{
    return value + 1;
}

void mapUnsizedArrayWithLambda(int values[])
{
    let result = values.map((int value) => value + 1);
    /*CHECK:
                           ^ static assertion failed, map requires an array with a statically known length
    */
}

void mapUnsizedArrayWithFunction(int values[])
{
    let result = values.map(increment);
//CHECK: static assertion failed, map requires an array with a statically known length
}

int generateForArray<T, let N : int>(Array<T, N> values)
{
    let result = Array<int, N>.generate((int index) => index);
    /*CHECK:
                                       ^ static assertion failed, generate requires an array with a statically known length
    */
    return result[0];
}

int generateForUnsizedArray(int values[])
{
    return generateForArray(values);
}

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int values[1] = {0};
    mapUnsizedArrayWithLambda(values);
    mapUnsizedArrayWithFunction(values);
    outputBuffer[0] = generateForUnsizedArray(values);
}
