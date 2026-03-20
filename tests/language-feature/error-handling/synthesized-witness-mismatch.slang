//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum MyError
{
    Fail = 0x10
};

interface IFoo
{
    int dosomething();
//CHECK:^^^^^^^^^^^ see interface requirement declaration of 'dosomething'
}

struct MyFoo: IFoo
{
    int dosomething() throws MyError
//CHECK:^^^^^^^^^^^ interface requirement mismatch
//CHECK:^^^^^^^^^^^ member 'dosomething' does not match interface requirement.
    {
        return 1;
    }
}

