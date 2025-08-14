//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

int testCommaOperator()
{
    // This should generate a warning - comma operator in variable initialization
    // CHECK: ([[# @LINE+1]]): warning 41024:
    float4 vColor = (0.f, 0.f, 0.f, 1.f);
    
    // This should NOT generate a warning - comma operator in for-loop side effect
    for (int i = 0; i < 10; i++, vColor.x++)
    {
    }
    
    // This should generate a warning - comma operator in regular expression
    // CHECK: ([[# @LINE+1]]): warning 41024:
    int x = (1, 2, 3);
    
    // This should now generate a warning - comma operator in return statement
    // CHECK: ([[# @LINE+2]]): warning 41024:
    int a = 5;
    return a *= 2, a + 1;
}

void someFunction(int value) {}