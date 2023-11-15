void f(out int x)
{
    // Shouldn't warn
    for(int i = 0; i < 5; ++i){
        if(i == 4)
            x = 0;
    }
}

void g(out int x)
{
    // Should warn
    for(int i = 0; i < 5; ++i){
        if(i == 5)
            x = 0;
    }
}

// CHECK-NOT: warning
// CHECK-NOT: error
