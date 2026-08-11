//TEST:INTERPRET(filecheck=CHECK):

// DECLARATION ORDER IS LOAD-BEARING -- DO NOT REORDER THE DECLARATIONS BELOW.
//
// No signature-position compile-time-constant use of `Size` may come before the
// first bound below. Resolving `Size` in such a position -- a member reference
// like `Size.Large` in a bound, or a generic value argument -- drives the enum
// to `DeclCheckState::ReadyForLookup`, which assigns `EnumDecl::tagType` as a
// side effect and makes the case here compile even without the fix. A use that
// is checked later (a scalar initializer, or anything in a function body) does
// not, and a bare `Size`-typed declaration resolves no value at all. So
// inserting `static int warm[int(Size.Large)];` or `static S<int(Size.Large)> warm;`
// above the first bound would silently retire the regression test.
//
// The opposite conversion -- an enum value back to its tag type -- lives in
// enum-to-tag-in-array-bound.slang. One cold `Size` cannot require both
// conversion sites: `int(Size(6))` constructs first and populates the tag type,
// leaving the enum-to-tag conversion with nothing to fix. Separate files keep
// each direction failing on its own when only one of the two sites is fixed.

enum Size
{
    Small = 2,
    Large = 6,
}

// Global bound: enum construction from a literal.
static int gGlobal[int(Size(6))];

// Struct field bound.
struct Holder
{
    int values[int(Size(6))];
}

// Function parameter bound over a generic value parameter, as originally
// reported.
int sumParam<let N : int>(int a[(Size(N) == Size.Large) ? 3 : 1])
{
    int total = 0;
    for (int i = 0; i < ((Size(N) == Size.Large) ? 3 : 1); ++i)
        total += a[i];
    return total;
}

void main()
{
    // CHECK: gGlobal=6
    printf("gGlobal=%d\n", gGlobal.getCount());

    Holder h;
    // CHECK: field=6
    printf("field=%d\n", h.values.getCount());

    int three[3] = { 1, 2, 3 };
    // CHECK: large=6
    printf("large=%d\n", sumParam<6>(three));

    int one[1] = { 9 };
    // CHECK: small=9
    printf("small=%d\n", sumParam<2>(one));
}
