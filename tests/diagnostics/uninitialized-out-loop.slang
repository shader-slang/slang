//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// This tests that we check that x is written to, even though it appears in a
// loop.
void foo(out int x)
{
    for (int i = 0; i < 8; i++)
        x = 0;
}

// CHECK-NOT: : warning
// CHECK-NOT: : error

