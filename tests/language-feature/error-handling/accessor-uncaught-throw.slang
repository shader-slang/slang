//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Accessors cannot be declared `throws`, so an error escaping a getter or setter body is
// diagnosed the same way as in any other non-throwing function (#13489).

enum MyError
{
    Failed
}

int f() throws MyError
{
    throw MyError.Failed;
}

struct GetterThrow
{
    property int p
    {
        get { throw MyError.Failed; }
//CHECK:      ^^^^^ uncaught 'throw' in non-throwing function
    }
}

struct SetterThrow
{
    int x;
    property int p
    {
        get { return x; }
        set { throw MyError.Failed; }
//CHECK:      ^^^^^ uncaught 'throw' in non-throwing function
    }
}

struct SubscriptGetterThrow
{
    __subscript(int i) -> int
    {
        get { throw MyError.Failed; }
//CHECK:      ^^^^^ uncaught 'throw' in non-throwing function
    }
}

struct GetterTry
{
    property int p
    {
        get { return try f(); }
//CHECK:             ^^^ uncaught 'try' in non-throwing function
    }
}

struct SetterTry
{
    int x;
    property int p
    {
        get { return x; }
        set { x = try f(); }
//CHECK:          ^^^ uncaught 'try' in non-throwing function
    }
}

struct GetterCatches
{
    property int p
    {
        get
        {
            do
            {
                return try f();
            }
            catch (e : MyError)
            {
                return -1;
            }
        }
    }
}
