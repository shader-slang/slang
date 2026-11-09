//TEST:INTERPRET(filecheck=CHECK):

// Enums whose cases are built through conversions on an enum that is itself
// mid-check: two that reference each other in both declaration orders, and one
// that references itself. `ensureDecl` sets a declaration's being-checked flag
// before advancing it, so these conversions genuinely arrive while the flag is
// set -- for `E` below, the flag belongs to the very enum being converted.
//
// They are harmless because case initializers are checked at
// `DefinitionChecked`, by which point the enum already satisfies the
// `DeclCheckState::ReadyForLookup` that `SemanticsVisitor::_coerce` asks for
// before reading `EnumDecl::tagType`, so `ensureDecl` returns on its
// already-checked path without reaching its re-entrancy check.
//
// That invariant is what lets the request be unconditional. Were `tagType` ever
// established later than the state a case initializer's conversion needs, the
// same call would land on the re-entrancy path and report a fatal cyclic
// reference, rejecting these declarations; this test is what would catch it.

// B is referenced before it is declared.
enum A
{
    X = int(B(1)),
}

enum B
{
    Y = int(A.X),
}

// The reverse order, with distinct names so both live in one module.
enum C
{
    Z = int(D.W),
}

enum D
{
    W = int(C(1)),
}

// The sharpest case: an enum whose own case is built by converting to and from
// itself, so both conversions run with this very enum's being-checked flag set.
enum E
{
    V = int(E(1)),
}

void main()
{
    // CHECK: A.X=1
    printf("A.X=%d\n", int(A.X));
    // CHECK: B.Y=1
    printf("B.Y=%d\n", int(B.Y));
    // CHECK: C.Z=1
    printf("C.Z=%d\n", int(C.Z));
    // CHECK: D.W=1
    printf("D.W=%d\n", int(D.W));
    // CHECK: E.V=1
    printf("E.V=%d\n", int(E.V));
}
