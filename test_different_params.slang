// Let's see if we can determine which methods are being found
struct Test {
    void method<int u>(int x) {
        printf("method(int) with u=%d, x=%d\n", u, x);
    }
}

extension Test {
    void method<int u>(float y) {  // Different parameter type to distinguish
        printf("extension method(float) with u=%d, y=%f\n", u, y);
    }
}

void main()
{
    Test t;
    
    // Both of these should work since they have different signatures
    t.method<3>(42);      // Should call method(int)
    t.method<3>(42.0f);   // Should call extension method(float)
}