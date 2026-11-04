//TEST:INTERPRET(filecheck=CHECK):

// Pack queries on a value pack can be the operand of `each` inside an `expand`
// pattern, both as a generic value argument and in an expression.

struct Layer<int InSize, int OutSize>
{
}

void add(inout int a, int b) { a += b; }

int sum<let each D : int>()
{
    int s = 0;
    expand add(s, each D);
    return s;
}

int layerCount<let each S : int>()
{
    return countof(expand Layer<each __trimLast(S), each __trimFirst(S)>);
}

int pairCount<let each S : int>()
{
    return countof(expand (each __trimLast(S)) + (each __trimFirst(S)));
}

int pairSum<let each S : int>()
{
    return sum<expand (each __trimLast(S)) + (each __trimFirst(S))>();
}

int innerSum<let each S : int>()
{
    return sum<expand each __trimFirst(__trimLast(S))>();
}

int runtimeTrimFirstSum<let each S : int>()
{
    int s = 0;
    expand add(s, each __trimFirst(S));
    return s;
}

void main()
{
    printf("layerCount=%d\n", layerCount<4, 8, 16, 32>());
    // CHECK: layerCount=3

    printf("layerCount=%d\n", layerCount<8>());
    // CHECK-NEXT: layerCount=0

    printf("pairCount=%d\n", pairCount<4, 8, 16, 32>());
    // CHECK-NEXT: pairCount=3

    printf("pairCount=%d\n", pairCount<8>());
    // CHECK-NEXT: pairCount=0

    printf("pairSum=%d\n", pairSum<4, 8, 16, 32>());
    // CHECK-NEXT: pairSum=84

    printf("pairSum=%d\n", pairSum<8>());
    // CHECK-NEXT: pairSum=0

    printf("innerSum=%d\n", innerSum<4, 8, 16, 32>());
    // CHECK-NEXT: innerSum=24

    printf("runtimeTrimFirstSum=%d\n", runtimeTrimFirstSum<4, 8, 16, 32>());
    // CHECK-NEXT: runtimeTrimFirstSum=56
}
