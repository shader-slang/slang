//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

IFunc<int> foo<int x>(int s)
{
     static int v = x;
     let f = ()=>{return v;};
     v += s;
     return f;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    IFunc<int> lambda1;
    IFunc<int> lambda2;

    {
        static int a = 4;
        int b = a;
        lambda1 = () => { return a; };
        lambda2 = () => { return b; };
        a++;
    }

    let lambda3 = foo<3>(0);
    let lambda4 = foo<3>(2);
    let lambda5 = foo<8>(0);

    // CHECK: 5
    outputBuffer[0] = lambda1();
    // CHECK-NEXT: 4
    outputBuffer[1] = lambda2();
    // CHECK-NEXT: 5
    outputBuffer[2] = lambda3();
    // CHECK-NEXT: 5
    outputBuffer[3] = lambda4();
    // CHECK-NEXT: 8
    outputBuffer[4] = lambda5();
}
