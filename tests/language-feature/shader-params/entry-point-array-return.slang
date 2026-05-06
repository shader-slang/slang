// Entry points cannot return array types. The compiler should emit
// a diagnostic error for all array return patterns.

//TEST:SIMPLE(filecheck=SINGLE): -target spirv -O0 -entry singleField -warnings-disable 38052
//TEST:SIMPLE(filecheck=MULTI): -target spirv -O0 -entry multiField -warnings-disable 38052
//TEST:SIMPLE(filecheck=NESTED): -target spirv -O0 -entry nestedArray -warnings-disable 38052

struct SingleFieldStruct
{
    int a;
}

// SINGLE: error[E38012]
[shader("vertex")]
Array<SingleFieldStruct, 1> singleField()
{
    return Array<SingleFieldStruct, 1>();
}

struct MultiFieldStruct
{
    int a;
    float b;
}

// MULTI: error[E38012]
[shader("vertex")]
Array<MultiFieldStruct, 3> multiField()
{
    Array<MultiFieldStruct, 3> result;
    result[0].a = 1; result[0].b = 1.0;
    result[1].a = 2; result[1].b = 2.0;
    result[2].a = 3; result[2].b = 3.0;
    return result;
}

struct NestedStruct
{
    int a;
    float b;
}

// NESTED: error[E38012]
[shader("vertex")]
Array<Array<NestedStruct, 2>, 3> nestedArray()
{
    Array<Array<NestedStruct, 2>, 3> result;
    return result;
}
