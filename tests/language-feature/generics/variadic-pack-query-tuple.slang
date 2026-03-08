//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    let tuple = makeTuple(10, 20, 30);

    printf("%d\n", __first(tuple));
    // CHECK: 10

    printf("%d\n", __last(tuple));
    // CHECK: 30

    let head = __trimHead(tuple);
    printf("%d\n", head._0 + head._1);
    // CHECK: 50

    let tail = __trimTail(tuple);
    printf("%d\n", tail._0 + tail._1);
    // CHECK: 30
}
