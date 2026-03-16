//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

// Verify that a generic typealias can partially bind type parameters,
// leaving a subset to be specified at the use site.

struct ArrayOfElements<T : IArithmetic, let size : int>
{
    T elems[size];

    T sum()
    {
        T total = T(0);
        for (int i = 0; i < size; i++)
            total = total + elems[i];
        return total;
    }
}

typealias ArrayOfFive<T : IArithmetic> = ArrayOfElements<T, 5>;
typealias ArrayOfThree<T : IArithmetic> = ArrayOfElements<T, 3>;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ArrayOfFive<int> arr5;
    arr5.elems[0] = 1;
    arr5.elems[1] = 2;
    arr5.elems[2] = 3;
    arr5.elems[3] = 4;
    arr5.elems[4] = 5;
    // CHECK: 15
    outputBuffer[0] = arr5.sum();

    ArrayOfThree<int> arr3;
    arr3.elems[0] = 10;
    arr3.elems[1] = 20;
    arr3.elems[2] = 30;
    // CHECK: 60
    outputBuffer[1] = arr3.sum();
}
