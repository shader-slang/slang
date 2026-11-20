// Verify the dimension ordering of multidimensional array declarations.
//
// In a type expression, `int[2][3]` is a 3-element array of `int[2]`. A
// declarator suffix `arr[3][2]` follows the C convention, so `int arr[3][2]`
// is also a 3-element array of `int[2]`, as is `var arr : int[2][3]`, which
// uses the same type expression form. When both forms are combined, the
// declarator dimensions are outermost: `int[2][3] arr[5][4]` is a 5-element
// array of 4-element arrays of `int[2][3]`.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void computeMain()
{
    int[2][3] arr1 = { };
    int arr2[3][2] = { };
    var arr3 : int[2][3] = { };
    int[2][3] arr4[5][4] = { };

    // CHECK: {{^}}type: int32_t{{$}}
    // CHECK-NEXT: {{^}}3{{$}}
    output[0] = arr1.getCount();
    // CHECK-NEXT: {{^}}2{{$}}
    output[1] = arr1[0].getCount();

    // CHECK-NEXT: {{^}}3{{$}}
    output[2] = arr2.getCount();
    // CHECK-NEXT: {{^}}2{{$}}
    output[3] = arr2[0].getCount();

    // CHECK-NEXT: {{^}}3{{$}}
    output[4] = arr3.getCount();
    // CHECK-NEXT: {{^}}2{{$}}
    output[5] = arr3[0].getCount();

    // arr1, arr2, and arr3 have the same type, so the assignments type-check.
    arr2[2][1] = 42;
    arr1 = arr2;
    arr3 = arr2;
    // CHECK-NEXT: {{^}}42{{$}}
    output[6] = arr1[2][1];
    // CHECK-NEXT: {{^}}42{{$}}
    output[7] = arr3[2][1];

    // CHECK-NEXT: {{^}}5{{$}}
    output[8] = arr4.getCount();
    // CHECK-NEXT: {{^}}4{{$}}
    output[9] = arr4[0].getCount();
    // CHECK-NEXT: {{^}}3{{$}}
    output[10] = arr4[0][0].getCount();
    // CHECK-NEXT: {{^}}2{{$}}
    output[11] = arr4[0][0][0].getCount();

    // The innermost element type is `int`.
    arr4[4][3][2][1] = 7;
    // CHECK-NEXT: {{^}}7{{$}}
    output[12] = arr4[4][3][2][1];
}
