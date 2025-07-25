struct Test {
    void method<int u>(int x) {
        printf("method(int) with u=%d\n", u);
    }
    
    void method<int u>() {
        printf("method() with u=%d\n", u);
    }
}

void main()
{
    Test t;
    t.method<3>(); // Should call the parameterless version
}