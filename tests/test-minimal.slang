//TEST:INTERPRET(filecheck=CHECK):

// Test for multiple generic value parameters through interface

interface ITest {
    int getValue();
};

struct Test<let N : int> : ITest {
    int getValue() {
        return N;
    }
};

void test(ITest it) {
    printf("Value: %d\n", it.getValue());
}

void main() {
    Test<10> t1;
    test(t1);
    Test<20> t2;
    test(t2);
    Test<30> t3;
    test(t3);
    
    // CHECK: Value: 10
    // CHECK: Value: 20  
    // CHECK: Value: 30
}