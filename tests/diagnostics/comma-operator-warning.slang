//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int testCommaOperator()
{
    // This should generate a warning - comma operator in variable initialization
    float4 vColor = (0.f, 0.f, 0.f, 1.f);
//CHECK:                ^ comma operator used in expression
//CHECK:                ^ comma operator used in expression (may be unintended)
//CHECK:                     ^ comma operator used in expression
//CHECK:                     ^ comma operator used in expression (may be unintended)
//CHECK:                          ^ comma operator used in expression
//CHECK:                          ^ comma operator used in expression (may be unintended)

    // This should NOT generate a warning - comma operator in for-loop side effect
    for (int i = 0; i < 10; i++, vColor.x++)
    {
    }

    // This should generate a warning - comma operator in regular expression
    int x = (1, 2, 3);
//CHECK:      ^ comma operator used in expression
//CHECK:      ^ comma operator used in expression (may be unintended)
//CHECK:         ^ comma operator used in expression
//CHECK:         ^ comma operator used in expression (may be unintended)

    // This should now generate a warning - comma operator in return statement
    int a = 5;
    return a *= 2, a + 1;
//CHECK:         ^ comma operator used in expression
//CHECK:         ^ comma operator used in expression (may be unintended)
}

void someFunction(int value) {}