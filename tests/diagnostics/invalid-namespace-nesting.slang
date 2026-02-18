//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct Type
{
    namespace ns {}
//CHECK:      ^^ declaration not allowed here
//CHECK:      ^^ namespace is not allowed here.
}
