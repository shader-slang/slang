//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

// Subclasses of `StructDecl` must enforce both parent and child declaration nesting rules.
// The diagnostic should name the HLSL spelling `class`, not the internal AST node kind.
interface IContainer
{
    class InvalidChild
//CHECK:  ^^^^^^^^^^^^ declaration not allowed here
//CHECK:  ^^^^^^^^^^^^ class is not allowed here.
    {
        int value;
    };
};

class InvalidParent
{
    namespace InvalidMember {}
//CHECK:      ^^^^^^^^^^^^^ declaration not allowed here
//CHECK:      ^^^^^^^^^^^^^ namespace is not allowed here.
};
