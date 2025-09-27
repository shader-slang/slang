//TEST:SIMPLE(filecheck=CHECK):-target spirv

int test(int arr[32]) {
    int sum = 0;
    for (int i =0; i < 32; i++) sum += arr[i];
    return sum;
}

uniform int gArr[32];
uniform int* result;

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK-NOT: OpCompositeConstruct
    *result = test(gArr);
}