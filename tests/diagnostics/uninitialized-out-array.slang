//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void foo(out int x[10])
{
    x[0] = 0;
}

struct S
{
    int a[10];
}

void bar(out S s)
{
    s.a[0] = 0;
}

// CHECK-NOT: : warning
// CHECK-NOT: : error
