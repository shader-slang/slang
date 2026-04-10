//TEST:INTERPRET(filecheck=CHECK):

int first(int x) { return x + 10; }
int last(int x) { return x + 20; }
int trimFirst(int x) { return x + 30; }
int trimLast(int x) { return x + 40; }

struct TypeHolder<T>
{
    T value;
}

struct ShadowedFirstTypeQuery<each T> where nonempty(T)
{
    TypeHolder<__first(T)> value;
}

struct ShadowedTrimFirstTypeQuery<each T>
{
    Tuple<expand each __trimFirst(T)> value;
}

void main()
{
    printf("%d\n", first(1));
    // CHECK: 11

    printf("%d\n", last(2));
    // CHECK: 22

    printf("%d\n", trimFirst(3));
    // CHECK: 33

    printf("%d\n", trimLast(4));
    // CHECK: 44

    let first = 100;
    let trimFirst = 200;

    printf("%d\n", first);
    // CHECK: 100

    printf("%d\n", trimFirst);
    // CHECK: 200

    ShadowedFirstTypeQuery<int, float, bool> q = {{3}};
    printf("%d\n", q.value.value);
    // CHECK: 3

    ShadowedTrimFirstTypeQuery<int, int, int> q2 = {makeTuple(7, 8)};
    printf("%d\n", q2.value._0 + q2.value._1);
    // CHECK: 15
}
