//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

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
    // ERROR: public method in private struct
    public void publicMethod() {} // ERROR.
//CHECK:        ^^^^^^^^^^^^ visibility higher than parent
//CHECK:        ^^^^^^^^^^^^ 'publicMethod' cannot have a higher visibility than 'MyType'.
}

void test()
{
    MyType t; // should leave uninitialized.
    t.func1(); // OK.
    // ERROR: calling private method
    t.func(); // ERROR.
/*CHECK:
      ^^^^ declaration not accessible
      ^^^^ 'func' is not accessible from the current context.
*/
    // ERROR: using private subscript
    t[0] = 1; // ERROR.
/*CHECK:
     ^ declaration not accessible
     ^ 'operator[]' is not accessible from the current context.
*/

    // ERROR: accessing private member
    t.member = 2;
/*CHECK:
      ^^^^^^ declaration not accessible
      ^^^^^^ 'member' is not accessible from the current context.
*/
}
