//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

interface IFoo {
    void test();
};

struct Foo<let GenericValue : uint> : IFoo {
    void test() {
        printf("GenericValue=%d, value=%d\n", GenericValue, value);
    }

    uint value;
};

void testFoo(IFoo foo) {
    foo.test();
}

void main() {
    Foo<0> foo0 = {0};
    testFoo(foo0);
    Foo<1> foo1 = {1};
    testFoo(foo1);
    Foo<2> foo2 = {2};
    testFoo(foo2);
    
    //CHECK: GenericValue=0, value=0
    //CHECK: GenericValue=1, value=1
    //CHECK: GenericValue=2, value=2
}