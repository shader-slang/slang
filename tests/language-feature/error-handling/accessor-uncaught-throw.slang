//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Accessors cannot be declared `throws`, so an error escaping a `get`, `set` or `ref` accessor
// body is diagnosed the same way as in any other non-throwing function (#13489).

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
//CHECK:      ^^^^^ the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement.
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
//CHECK:      ^^^^^ the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement.
    }
}

struct SubscriptGetterThrow
{
    __subscript(int i) -> int
    {
        get { throw MyError.Failed; }
//CHECK:      ^^^^^ uncaught 'throw' in non-throwing function
//CHECK:      ^^^^^ the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement.
    }
}

struct SubscriptSetterThrow
{
    int x;
    __subscript(int i) -> int
    {
        get { return x; }
        set { throw MyError.Failed; }
//CHECK:      ^^^^^ uncaught 'throw' in non-throwing function
//CHECK:      ^^^^^ the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement.
    }
}

struct SubscriptRefThrow
{
    int x;
    __subscript(int i) -> int
    {
        get { return x; }
        ref { throw MyError.Failed; }
//CHECK:      ^^^^^ uncaught 'throw' in non-throwing function
//CHECK:      ^^^^^ the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement.
    }
}

struct GetterTry
{
    property int p
    {
        get { return try f(); }
//CHECK:             ^^^ uncaught 'try' in non-throwing function
//CHECK:             ^^^ the current function or environment is not declared to throw any errors, but the 'try' clause is not caught
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
//CHECK:          ^^^ the current function or environment is not declared to throw any errors, but the 'try' clause is not caught
    }
}

struct SubscriptGetterTry
{
    __subscript(int i) -> int
    {
        get { return try f(); }
//CHECK:             ^^^ uncaught 'try' in non-throwing function
//CHECK:             ^^^ the current function or environment is not declared to throw any errors, but the 'try' clause is not caught
    }
}

// A `catch` of a different type does not handle `MyError`, so the error still escapes the getter.
struct GetterCatchesOtherType
{
    property int p
    {
        get
        {
            do
            {
                return try f();
//CHECK:               ^^^ uncaught 'try' in non-throwing function
//CHECK:               ^^^ the current function or environment is not declared to throw any errors, but the 'try' clause is not caught
            }
            catch (e : int)
            {
                return -1;
            }
        }
    }
}

// A matching `catch` handles the error inside the getter, so this case reports no diagnostic.
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
