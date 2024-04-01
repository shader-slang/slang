//TEST:SIMPLE(filecheck=CHECK):

// Test to catch compilation time errors with return values in a constructor / member functions within a class / struct.
struct MyType
{
    float v;

    // A constructor from the callsite's point of view is a function that returns MyType.
    // A constructor from the inside the body is treated as a function that modifies `this`.
    // because we are expected to write:
    // this.v = 5;

    // A test case where the constructor returns a value.
    // This automatically gets resolved as this = t; return this;
    __init()
    {
        MyType t;
        t.v = 5;
        return t;
    }

    // A valid use-case when the constructor returns a void
    __init(int v)
    {
        if (v == 0)
        {
            this.v = 5;
            return this;
        }
        this.v = v;
    }

    // This member function must return a value with the correct type
    MyType createNew() {
        MyType t;
        t.v = 5;
        // CHECK: ([[# @LINE+1]]): error 30006: 'return' should have an expression
        return;
    }
}

void main()
{
    MyType t = MyType();
    MyType t1 = t.createNew();
    MyType t2 = MyType(0);
}
