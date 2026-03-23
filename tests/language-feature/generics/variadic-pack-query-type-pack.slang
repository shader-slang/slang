//TEST:INTERPRET(filecheck=CHECK):

struct Wrapper<T>
{
    T value;
}

struct CountHolder<let N : int>
{
    static int get() { return N; }
}

int countTrimFirstTypePack<each T>()
{
    return countof(__trimFirst(T));
}

int countTrimLastTypePack<each T>()
{
    return countof(__trimLast(T));
}

struct FirstHolder<each T> where nonempty(T)
{
    __first(T) value;
}

struct LastHolder<each T> where nonempty(T)
{
    __last(T) value;
}

int countTrimFirstExpandedTypePack<each T>()
{
    return countof(__trimFirst(expand Wrapper<each T>));
}

int countReconstructedTuple<each T>() where nonempty(T)
{
    return CountHolder<countof(Tuple<__first(T), expand each __trimFirst(T)>)>.get();
}

void main()
{
    printf("%d\n", countTrimFirstTypePack<int, int, int>());
    // CHECK: 2

    printf("%d\n", countTrimLastTypePack<int, int, int>());
    // CHECK: 2

    printf("%d\n", countTrimFirstTypePack<>());
    // CHECK: 0

    FirstHolder<int, int, int> firstHolder = {11};
    printf("%d\n", firstHolder.value);
    // CHECK: 11

    LastHolder<int, int, int> lastHolder = {22};
    printf("%d\n", lastHolder.value);
    // CHECK: 22

    printf("%d\n", countTrimFirstExpandedTypePack<int, float, bool>());
    // CHECK: 2

    printf("%d\n", countReconstructedTuple<int, float, bool>());
    // CHECK: 3

    printf("%d\n", countReconstructedTuple<int>());
    // CHECK: 1
}
