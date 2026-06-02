//TEST:INTERPRET(filecheck=CHECK):

interface IGenericThisSizedArray<T>
{
    static const int Size;

    __init(T[This.Size] data);
    T get(int index);
}

struct GenericThisSizedArrayImpl<T, let N : int> : IGenericThisSizedArray<T>
{
    static const int Size = N;
    T data[N];

    __init(T[Size] input)
    {
        for (int i = 0; i < Size; ++i)
            data[i] = input[i];
    }

    T get(int index)
    {
        return data[index];
    }
}

void main()
{
    int input[3] = { 4, 5, 6 };
    GenericThisSizedArrayImpl<int, 3> value = GenericThisSizedArrayImpl<int, 3>(input);
    printf("%d %d\n", value.get(0), value.get(2));
    // CHECK: 4 6
}
