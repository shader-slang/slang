//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// The core module converts a scalar to a vector with `__builtin_cast`, which never calls a
// user-defined conversion. A user conversion into the element type therefore does not make
// a vector, struct, or pointer source implicitly convertible to the vector type, and the
// mismatch is reported at the source instead of as a nested vector type such as
// `vector<vector<float,3>,4>` (shader-slang/slang#14100).

struct S
{
    float x;
}

extension float
{
    __implicit_conversion(500)
    __init(float3 v) { this = v.x; }

    __implicit_conversion(500)
    __init(S v) { this = v.x; }
}

float4 fromFloat3(float3 p)
{
    return p;
//CHECK:   ^ type mismatch in expression
//CHECK:   ^ expected an expression of type 'vector<float,4>', got 'vector<float,3>'
}

float4 fromStruct(S s)
{
    float4 r = s;
//CHECK:       ^ type mismatch in expression
//CHECK:       ^ expected an expression of type 'vector<float,4>', got 'S'
    return r;
}

// The core module's own `bool.__init(Ptr)` does not make a pointer splat either, because a
// pointer is not a valid vector element.
bool4 fromPointer(int* p)
{
    return p;
//CHECK:   ^ type mismatch in expression
//CHECK:   ^ expected an expression of type 'vector<bool,4>', got 'Ptr<int, Access.ReadWrite, AddressSpace.Device, DefaultDataLayout>'
}

// A one-element vector splats implicitly only into a vector of the same element type. An
// explicit `float4(double1(x))` still converts the element and then splats.
double4 fromFloat1(float1 v)
{
    return v;
//CHECK:   ^ type mismatch in expression
//CHECK:   ^ expected an expression of type 'vector<double,4>', got 'vector<float,1>'
}

float4 fromInt1(int1 v)
{
    return v;
//CHECK:   ^ type mismatch in expression
//CHECK:   ^ expected an expression of type 'vector<float,4>', got 'vector<int,1>'
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}
