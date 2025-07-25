// Test to see if both candidates are being found
struct Test {
    void method<int u>(int x) {
        printf("Test::method(int) with u=%d\n", u);
    }
}

extension Test {
    void method<int u>() {
        printf("Test extension::method() with u=%d\n", u);
    }
    
    // Let's also try a different name to see if that works
    void otherMethod<int u>() {
        printf("Test extension::otherMethod() with u=%d\n", u);
    }
}

void main()
{
    Test t;
    // This fails
    t.method<3>();
    
    // But this should work since there's no conflict
    t.otherMethod<3>();
}