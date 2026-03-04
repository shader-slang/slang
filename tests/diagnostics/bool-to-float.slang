// bool-to-float.slang

// Diagnostic messages related to converting bool to float

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Test
{
    void test()
    {
        float f = true;
//CHECK:          ^^^^ implicit conversion not recommended
//CHECK:          ^^^^ implicit conversion from 'bool' to 'float' is not recommended
    }
}
