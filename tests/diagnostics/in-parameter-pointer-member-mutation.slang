//TEST:SIMPLE:

struct MutableValue
{
    [mutating]
    void setValue(int newValue)
    {
        value = newValue;
    }

    int value;
};

int mutatePointee(MutableValue* pointer, int newValue)
{
    // `pointer` is an ordinary `in` parameter, but `->` crosses into the pointee's storage.
    // Mutating that storage must not be diagnosed as a mutation of the parameter itself.
    pointer->setValue(newValue);
    return pointer->value;
}

int mutateCapturedPointee(MutableValue* pointer, int newValue)
{
    // Lambda capture makes the closure's `pointer` field immutable, but `->` still reaches the
    // separate storage named by `pointer`.
    let mutate = () => pointer->setValue(newValue);
    mutate();
    return pointer->value;
}
