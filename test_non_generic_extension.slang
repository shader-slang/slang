struct Test {
    void method(int x) {
        printf("method(int)\n");
    }
}

extension Test {
    void method() {
        printf("extension method()\n");
    }
}

void main()
{
    Test t;
    t.method(); // Should call the extension parameterless version
}