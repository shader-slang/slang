//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void foo(bool b, out float y)
{
    if(b)
    {
        y = 0;
    }
    // Should warn that y is possibly uninitialized
    return;
    // CHECK:      tests/diagnostics/uninitialized-out-possibly.slang([[#@LINE-1]]): warning 41106: returning possibly without initializing out parameter 'y'
}

// Check we don't have any extra warnings not caught by the above diagnostics
// CHECK-NOT: : warning
// CHECK-NOT: : error
