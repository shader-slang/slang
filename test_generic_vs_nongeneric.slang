// Let's test if extension methods are found at all for generic methods
struct Test {
    void nonGenericMethod(int x) {
        printf("original nonGenericMethod(int) x=%d\n", x);
    }
    
    void genericMethod<int u>(int x) {
        printf("original genericMethod(int) u=%d, x=%d\n", u, x);
    }
}

extension Test {
    void nonGenericMethod(float y) {
        printf("extension nonGenericMethod(float) y=%f\n", y);
    }
    
    void genericMethod<int u>(float y) {
        printf("extension genericMethod(float) u=%d, y=%f\n", u, y);
    }
}

void main()
{
    Test t;
    
    // Non-generic methods - these should work fine
    t.nonGenericMethod(42);      // original
    t.nonGenericMethod(42.0f);   // extension
    
    // Generic methods - do these work?
    t.genericMethod<3>(42);      // original
    t.genericMethod<3>(42.0f);   // extension (this is likely to fail)
}