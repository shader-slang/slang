// A conforming type whose only member named like an interface method requirement is not a
// method must fail conformance with E38105. Witness synthesis tries to call that member, and
// the non-callable single candidate used to be reported as an ambiguous reference, which read
// an empty overload list and aborted the compiler (gh-13349).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IField
{
    int foo();
//CHECK:^^^ see interface requirement declaration of 'foo'
}

struct Field : IField
{
    int foo;
//CHECK:^^^ E38105
//CHECK:^^^ member 'foo' does not match interface requirement.
}

interface IStaticConst
{
    int foo();
//CHECK:^^^ see interface requirement declaration of 'foo'
}

struct StaticConst : IStaticConst
{
    static const int foo = 3;
//CHECK:             ^^^ E38105
//CHECK:             ^^^ member 'foo' does not match interface requirement.
}

interface IProperty
{
    int foo();
//CHECK:^^^ see interface requirement declaration of 'foo'
}

struct Property : IProperty
{
    property int foo
//CHECK:         ^^^ E38105
//CHECK:         ^^^ member 'foo' does not match interface requirement.
    {
        get { return 1; }
    }
}

interface IFieldWithParam
{
    int foo(int x);
//CHECK:^^^ see interface requirement declaration of 'foo'
}

struct FieldWithParam : IFieldWithParam
{
    int foo;
//CHECK:^^^ E38105
//CHECK:^^^ member 'foo' does not match interface requirement.
}
