//TEST:EXECUTABLE:
[mutating]
public void f(Ptr<int> arr, inout int size)
{
    for (int i = 0; i < 1; ++i)
        arr[i] = 0;

    int end = 0;
    if (end >= size)
        end = size;

    for (int i = 0; i < 1; ++i)
        arr[i] = 0;

    size -= end;
}

export __extern_cpp int main()
{
    // Success is not crashing the compiler.
    int size = 0;
    int arr = 0;
    f(&arr, size);

    return 0;
}
