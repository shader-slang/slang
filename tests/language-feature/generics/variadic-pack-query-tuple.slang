//TEST:INTERPRET(filecheck=CHECK):
#lang slang 2026

struct IntConst<let N : int>
{
    static int get() { return N; }
}

void main()
{
    let tuple = makeTuple(10, 20, 30);

    printf("%d\n", __first(tuple));
    // CHECK: 10

    printf("%d\n", __last(tuple));
    // CHECK: 30

    let head = __trimFirst(tuple);
    printf("%d\n", head._0 + head._1);
    // CHECK: 50

    let tail = __trimLast(tuple);
    printf("%d\n", tail._0 + tail._1);
    // CHECK: 30

    // Tuple-expression syntax should also fold through `__first` / `__last`
    // when the result is required in an integer-constant context.
    printf("%d\n", IntConst<__first((40, 50))>.get());
    // CHECK: 40

    printf("%d\n", IntConst<__last((60, 70))>.get());
    // CHECK: 70
}
