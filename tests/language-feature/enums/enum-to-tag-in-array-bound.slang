//TEST:INTERPRET(filecheck=CHECK):

// Companion to enum-construction-in-array-bound.slang, kept in a SEPARATE file
// on purpose. This one covers the enum-to-tag-type conversion, and the other
// covers construction of an enum from an integer. Both conversions read
// `EnumDecl::tagType`, and the first one to run drives the enum to
// `DeclCheckState::ReadyForLookup` and populates that field for everything
// after it. So one cold enum cannot require both conversion sites: in a file
// that also constructs (`int(Size(6))`), the construction populates the field
// first and leaves the enum-to-tag conversion nothing to fix. Keeping the two
// directions in separate files is what makes each one fail on its own when only
// one of the two conversion sites is fixed.
//
// For the same reason, do not add any use of `Size(...)` -- or any other
// compile-time-constant use of `Size` in a declaration's signature -- to this
// file ahead of the bound below.

enum Size
{
    Small = 2,
    Large = 6,
}

// The bound converts an enum-typed generic value parameter to its tag type.
// Nothing here constructs a `Size` from an integer.
int sumEnumParam<let S : Size>(int a[int(S)])
{
    int total = 0;
    for (int i = 0; i < int(S); ++i)
        total += a[i];
    return total;
}

void main()
{
    int six[6] = { 1, 2, 3, 4, 5, 6 };
    // CHECK: enumParam=21
    printf("enumParam=%d\n", sumEnumParam<Size.Large>(six));
}
