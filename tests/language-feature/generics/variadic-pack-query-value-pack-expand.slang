//TEST:INTERPRET(filecheck=CHECK):

// Pack queries on a value pack can be the operand of `each` inside an `expand`
// pattern, both as a generic value argument and in an expression. `layerCount`,
// `pairCount`, `pairSum`, `innerSum`, `pairDigits` and `swapDigits` fold the `each`
// operand at compile time; `runtime*` capture it for a run-time `expand`; `local*`
// capture a `let` local that holds a pack, which does not fold.

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

void push(inout int a, int b) { a = a * 100 + b; }

// `digits` depends on the order of the pack elements, so it also checks that each element
// of `__trimLast(S)` is paired with the next element of `S`.
int digits<let each D : int>()
{
    int s = 0;
    expand push(s, each D);
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

int pairDigits<let each S : int>()
{
    return digits<expand (each __trimLast(S)) * 10 + (each __trimFirst(S))>();
}

int runtimePairDigits<let each S : int>()
{
    int s = 0;
    expand push(s, (each __trimLast(S)) * 10 + (each __trimFirst(S)));
    return s;
}

int swapDigits<let each S : int>()
{
    return digits<expand each __shapeSwap(S, 0, 2)>();
}

int runtimeSwapDigits<let each S : int>()
{
    int s = 0;
    expand push(s, each __shapeSwap(S, 0, 2));
    return s;
}

struct Outer<let each D : int>
{
    static int pairDigits()
    {
        return digits<expand (each __trimLast(D)) * 10 + (each __trimFirst(D))>();
    }
}

int localPackSum<let each S : int>()
{
    let p = S;
    int s = 0;
    expand add(s, each p);
    return s;
}

int localTrimFirstCount<let each S : int>()
{
    let p = __trimFirst(S);
    return countof(expand each p);
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

    printf("runtimeTrimFirstSum=%d\n", runtimeTrimFirstSum<8>());
    // CHECK-NEXT: runtimeTrimFirstSum=0

    printf("pairDigits=%d\n", pairDigits<1, 2, 3, 4>());
    // CHECK-NEXT: pairDigits=122334

    printf("runtimePairDigits=%d\n", runtimePairDigits<1, 2, 3, 4>());
    // CHECK-NEXT: runtimePairDigits=122334

    printf("swapDigits=%d\n", swapDigits<1, 2, 3>());
    // CHECK-NEXT: swapDigits=30201

    printf("runtimeSwapDigits=%d\n", runtimeSwapDigits<1, 2, 3>());
    // CHECK-NEXT: runtimeSwapDigits=30201

    printf("outerPairDigits=%d\n", Outer<5, 6, 7>.pairDigits());
    // CHECK-NEXT: outerPairDigits=5667

    printf("localPackSum=%d\n", localPackSum<1, 2, 3>());
    // CHECK-NEXT: localPackSum=6

    printf("localTrimFirstCount=%d\n", localTrimFirstCount<1, 2, 3>());
    // CHECK-NEXT: localTrimFirstCount=2
}
