// A `printf` argument that is, or contains, a resource or opaque type is rejected with a
// diagnostic on every target, including a `ConstantBuffer` whose element type holds a resource.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute

struct Mixed
{
    uint x;
    Texture2D t;
}

Texture2D tex;
StructuredBuffer<uint> sb;
RWStructuredBuffer<uint> rwsb;
ConstantBuffer<Mixed> cbMixed;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    printf("tex\n", tex);
/*CHECK:
                    ^^^ 'printf' argument cannot be a resource or opaque type
                    ^^^ argument of type 'Texture2D<vector<float,4>>' is or contains a resource or opaque type, which 'printf' cannot print.
*/
    printf("sb\n", sb);
/*CHECK:
                   ^^ 'printf' argument cannot be a resource or opaque type
                   ^^ argument of type 'StructuredBuffer<uint, DefaultDataLayout>' is or contains a resource or opaque type, which 'printf' cannot print.
*/
    printf("rwsb\n", rwsb);
/*CHECK:
                     ^^^^ 'printf' argument cannot be a resource or opaque type
                     ^^^^ argument of type 'RWStructuredBuffer<uint, DefaultDataLayout>' is or contains a resource or opaque type, which 'printf' cannot print.
*/
    printf("cbMixed\n", cbMixed);
/*CHECK:
                        ^^^^^^^ 'printf' argument cannot be a resource or opaque type
                        ^^^^^^^ argument of type 'Mixed' is or contains a resource or opaque type, which 'printf' cannot print.
*/
    printf("a=%u tex b=%u\n", 7u, tex, 9u);
/*CHECK:
                                  ^^^ 'printf' argument cannot be a resource or opaque type
                                  ^^^ argument of type 'Texture2D<vector<float,4>>' is or contains a resource or opaque type, which 'printf' cannot print.
*/
}
