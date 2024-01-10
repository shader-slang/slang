//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct S
{
    int x;
    int y;
}

struct T
{
    S s;
}

S foo(out T t)
{
    t.s.x = 0;
    S n = t.s;
    // CHECK-DAG: tests/diagnostics/uninitialized-out-members.slang([[#@LINE-1]]): warning 41101: use of partially initialized value 't'
    return n;
    // CHECK-DAG: tests/diagnostics/uninitialized-out-members.slang([[#@LINE-1]]): warning 41105: returning without fully initializing out parameter 't'
}

void bar(bool b, out T t)
{
    t.s.x = 0;
    if(b)
        t.s.y = 1;
    else
        t.s.y = 2;
    // Regardless of which branch was taken, `t` is fully initialized
    int n = t.s.y;
}
// CHECK-NOT: : warning
// CHECK-NOT: : error
