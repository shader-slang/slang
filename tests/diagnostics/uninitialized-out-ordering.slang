//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct S
{
    float x;
    float y;
}

void foo(out S s, out float f, out float g)
{
    f = s.x; // This should because s.x hasn't been written to yet
    // CHECK-DAG: tests/diagnostics/uninitialized-out-ordering.slang([[#@LINE-1]]): error 41100: use of uninitialized value 's'
    s.x = 0;
    f = s.x; // This should not warn because it's reading only from the written portion of x
    g = s.y; // This should warn because it's not yet initialized
    // CHECK-DAG: tests/diagnostics/uninitialized-out-ordering.slang([[#@LINE-1]]): error 41100: use of uninitialized value 's'
    s.y = 1;
}

// Check we don't have any extra warnings not caught by the above diagnostics
// CHECK-NOT: : warning
// CHECK-NOT: : error
