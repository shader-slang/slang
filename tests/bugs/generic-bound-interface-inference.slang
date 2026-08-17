//TEST:INTERPRET(filecheck=CHECK):

// Regression test for inferring a missing generic parameter from the interface
// arguments written on another generic parameter's where-clause.

interface IArrayAccessor<T>
{
    T getElement();
}

struct IntArray : IArrayAccessor<int>
{
    int getElement()
    {
        return 42;
    }
}

__generic<U, InArray : IArrayAccessor<U>>
U readElement(InArray array)
{
    return array.getElement();
}

void main()
{
    IntArray array;
    printf("%d\n", readElement(array));
    // CHECK: 42
}
