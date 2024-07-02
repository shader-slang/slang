int use_undefined_value(int k)
{
    int x;
    x += k;
    return x;
}

__generic<T>
T generic_undefined_return()
{
    T x;
    return x;
}
