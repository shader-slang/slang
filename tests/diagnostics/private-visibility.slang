//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

struct MyType
{
    private int member;
    private int func()
    {
        return member;
    }
    struct SubType
    {
        int member2;
        int func2(MyType m)
        {
            return m.member + member2; // OK.
        }
    }
    int func1() { return member; }

    private __init() { member = 0; }
    
    private __subscript(int i)->int
    {
        get { return member; }
        set { member = newValue; }
    }
    // CHECK:{{.*}}([[# @LINE+1]]): error 30601:
    public void publicMethod() {} // ERROR.
}

void test()
{
    MyType t; // should leave uninitialized.
    // CHECK-NOT:{{.*}}error
    t.func1(); // OK.
    // CHECK:{{.*}}([[# @LINE+1]]): error 30600:
    t.func(); // ERROR.
    // CHECK:{{.*}}([[# @LINE+1]]): error 30600:
    t[0] = 1; // ERROR.

    // CHECK:{{.*}}([[# @LINE+1]]): error 30600:
    t.member = 2;
}
