// countof(EnumType) folds at compile time to the number of declared enum cases,
// independent of the assigned tag values (see shader-slang/slang#12549).
//TEST:INTERPRET(filecheck=CHECK):

// Plain enum, contiguous default tags, trailing comma.
enum DataType
{
    A,
    B,
    C,
}

// enum class, contiguous default tags.
enum class Color
{
    Red,
    Green,
    Blue,
}

// Explicit, non-contiguous tags: the count is still the number of cases, not max-tag+1.
enum E2
{
    X = 5,
    Y,
    Z,
}

// A five-case uint enum: its case count (5) differs from its alignment (4), so a wrong
// value-materialization path that lowered the count as the enum's alignment would show 4.
enum E5 : uint
{
    P0,
    P1,
    P2,
    P3,
    P4,
}

// Use the folded count in a context that requires a compile-time constant, so the
// test fails to compile if countof does not fold.
static const int gDataTypeCount = countof(DataType);

void main()
{
    // Materialized-value use (the issue's acceptance criterion): a static const whose
    // value is stored/printed at runtime must be the case count, not the enum's alignment.
    printf("%d\n", gDataTypeCount);
    // CHECK: 3

    printf("%d\n", countof(Color));
    // CHECK: 3

    // Array extent: exercises the compile-time constant-fold path.
    int colors[countof(Color)];
    printf("%d\n", (int)(sizeof(colors) / sizeof(colors[0])));
    // CHECK: 3

    printf("%d\n", countof(E2));
    // CHECK: 3

    // Five cases, uint-backed: distinguishes case count (5) from alignment (4).
    printf("%d\n", countof(E5));
    // CHECK: 5
}
