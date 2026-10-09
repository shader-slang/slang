// A member that shares its name with an interface method requirement but is not a method is
// reported as E38105: a field, a static const, a property, a nested type, an array field, a
// field of a type whose conformance is declared in an extension, and an inherited field.
// Witness synthesis diagnoses these single-candidate mismatches without an internal error
// (gh-13349).

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

interface INestedType
{
    int foo();
//CHECK:^^^ see interface requirement declaration of 'foo'
}

struct NestedType : INestedType
{
    struct foo { int x; }
//CHECK:   ^^^ E38105
//CHECK:   ^^^ member 'foo' does not match interface requirement.
}

interface IFooTakesInt
{
    int foo(int x);
//CHECK:^^^ see interface requirement declaration of 'foo'
}

struct FieldForFooTakesInt : IFooTakesInt
{
    int foo;
//CHECK:^^^ E38105
//CHECK:^^^ member 'foo' does not match interface requirement.
}

interface IArrayFooTakesInt
{
    int foo(int x);
//CHECK:^^^ see interface requirement declaration of 'foo'
}

struct ArrayFieldForFooTakesInt : IArrayFooTakesInt
{
    int foo[4];
//CHECK:^^^ E38105
//CHECK:^^^ member 'foo' does not match interface requirement.
}

interface IExtension
{
    int foo();
//CHECK:^^^ see interface requirement declaration of 'foo'
}

struct ExtensionField
{
    int foo;
//CHECK:^^^ E38105
//CHECK:^^^ member 'foo' does not match interface requirement.
}

extension ExtensionField : IExtension {}

interface IInherited
{
    int foo();
//CHECK:^^^ see interface requirement declaration of 'foo'
}

struct InheritedBase
{
    int foo;
//CHECK:^^^ E38105
//CHECK:^^^ member 'foo' does not match interface requirement.
}

struct InheritedField : InheritedBase, IInherited {}
//CHECK:                ^^^^^^^^^^^^^ warning E30816
