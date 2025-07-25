// Test to understand where the issue occurs
struct Test {
    void method<int u>(int x) {
        printf("Original method(int) with u=%d\n", u);
    }
}

extension Test {
    void method<int u>() {
        printf("Extension method() with u=%d\n", u);
    }
}

void main()
{
    Test t;
    
    // This works
    t.method<3>(42);
    
    // This doesn't work 
    t.method<3>();
}