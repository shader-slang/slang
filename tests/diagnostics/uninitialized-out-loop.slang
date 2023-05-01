//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void foo(out int x)
{
    for (int i = 0; i < 8; i++)
        x = 0;
}

// CHECK-NOT: warning

