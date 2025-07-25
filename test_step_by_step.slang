// Let's test if the issue is with the generic application or the final invoke
struct Test {
    void method<int u>(int x) {
        printf("method(int) with u=%d\n", u);
    }
}

extension Test {
    void method<int u>() {
        printf("extension method() with u=%d\n", u);
    }
}

void main()
{
    Test t;
    
    // Test step by step:
    
    // Step 1: This should work (no generic, no extension conflict)
    // t.method(42);  // Would call method(int) if it existed without generics
    
    // Step 2: This should work (generic call with arguments to original method)
    t.method<3>(42);
    
    // Step 3: This fails (generic call without arguments, should go to extension)
    t.method<3>();
}