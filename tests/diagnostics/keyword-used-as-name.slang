//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

// Using a type-introducing keyword as the name of a variable, parameter, or
// field is problematic: the parser treats the name as the start of a type
// specifier, so e.g. `struct = ...;` cannot be parsed as an assignment. We warn
// about exactly those keywords (struct, class, enum, typealias, typedef).
//
// Almost all other keywords are contextual and remain usable as ordinary
// identifiers (e.g. `triangle`, `func`, `let`, `interface`), so they must NOT
// warn here.

RWStructuredBuffer<int> outputBuffer;

// Global variable declaration site.
static int enum = 0;
//CHECK:   ^^^^ keyword used as a name
//CHECK:   ^^^^ 'enum' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

// Field declaration site.
struct S
{
    int class;
//CHECK:^^^^^ keyword used as a name
//CHECK:^^^^^ 'class' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
}

// `typedef` alias-name site (reached via the shared declarator machinery).
typedef int class;
//CHECK:    ^^^^^ keyword used as a name
//CHECK:    ^^^^^ 'class' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

// `typealias` alias-name site (reads the name directly; must warn symmetrically).
typealias struct = int;
//CHECK:  ^^^^^^ keyword used as a name
//CHECK:  ^^^^^^ 'struct' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

// Parameter declaration site (traditional declarator).
int useParam(int struct)
//CHECK:         ^^^^^^ keyword used as a name
//CHECK:         ^^^^^^ 'struct' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
{
    return 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int typealias = 0;
//CHECK:^^^^^^^^^ keyword used as a name
//CHECK:^^^^^^^^^ 'typealias' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts
    int typedef = 0;
//CHECK:^^^^^^^ keyword used as a name
//CHECK:^^^^^^^ 'typedef' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

    // Modern (`let`/`var`) declaration syntax is also covered.
    let struct = 0;
//CHECK:^^^^^^ keyword used as a name
//CHECK:^^^^^^ 'struct' is a type keyword; using it as a name may make the name ambiguous or impossible to reference in some contexts

    // Contextual keywords must remain usable as names without a warning.
    int triangle = 0;
    int func = 0;
    int letVar = 0;
    int interface = 0;

    outputBuffer[0] = triangle + func + letVar + interface;
}
