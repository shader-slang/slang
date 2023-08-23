//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void foo(out int x[10])
{
    // We currently assume that a write to any element of an array is a write
    // to all elements. Loop inversion makes it so that the write is known to
    // be executed at least once, so we don't warn in this case.
    for(int i = 0; i < 10; i++)
        x[i] = 0;
}

// CHECK-NOT: : warning
// CHECK-NOT: : error
