//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

float foo(out float3 v)
{
    // This should error as we haven't set v before we read from it
    float r = v.x + 1.0;
    // CHECK-DAG: tests/diagnostics/uninitialized-out.slang([[#@LINE-1]]): error 41100: use of uninitialized value 'v'

    // This should warn as we haven't set v before we return
    return r;
    // CHECK-DAG: tests/diagnostics/uninitialized-out.slang([[#@LINE-1]]): warning 41104: returning without initializing out parameter 'v'
}

// This should warn as we return without x being initialized
float bar(out float x)
{
    return 0;
    // CHECK-DAG: tests/diagnostics/uninitialized-out.slang([[#@LINE-1]]): warning 41104: returning without initializing out parameter 'x'
}

// This should also warn pointing at the implicit return
void baz(out float x) {}
// CHECK-DAG: tests/diagnostics/uninitialized-out.slang([[#@LINE-1]]): warning 41104: returning without initializing out parameter 'x'

void twoReturns(bool b, out float y)
{
    if(b)
    {
        // Should warn
        return;
        // CHECK-DAG: tests/diagnostics/uninitialized-out.slang([[#@LINE-1]]): warning 41104: returning without initializing out parameter 'y'
    }
    y = 0;
    // Shouldn't warn
    return;
}

void twoOkReturns(bool b, out float y)
{
    if(b)
    {
        // Shouldn't warn
        unused(y);
        return;
    }
    y = 0;
    // Shouldn't warn
    return;
}

// TODO: This should warn that arr isn't fully initialized
void partial(out float arr[2])
{
    arr[0] = 1;
}

// CHECK-NOT: : warning
// CHECK-NOT: : error
