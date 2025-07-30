//DIAGNOSTIC_TEST:SIMPLE:

void testCommaOperator()
{
    // This should generate a warning - comma operator in variable initialization
    float4 vColor = (0.f, 0.f, 0.f, 1.f);
    
    // This should NOT generate a warning - comma operator in for-loop side effect
    for (int i = 0; i < 10; i++, vColor.x++)
    {
    }
    
    // This should generate a warning - comma operator in regular expression
    int x = (1, 2, 3);
}

void someFunction(int value) {}