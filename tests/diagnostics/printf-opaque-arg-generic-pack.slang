// A resource that reaches `printf` through a generic is only known after specialization. The
// specialized module is checked on every target, so the argument is reported instead of crashing
// type legalization, being dropped from the argument list, or being printed as a handle.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute

ConstantBuffer<uint> cb;
Texture2D tex;

void logPack<each T>(expand each T args)
{
    printf("%u\n", expand each args);
/*CHECK:
          ^ unsupported printf argument type
          ^ a 'printf' argument is or contains 'cbuffer<uint>', a resource or opaque type, which 'printf' cannot print.
*/
}

void logOne<T>(T arg)
{
    printf("%u %u %u\n", 7u, arg, 9u);
/*CHECK:
          ^ unsupported printf argument type
          ^ a 'printf' argument is or contains 'Texture2D', a resource or opaque type, which 'printf' cannot print.
*/
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    logPack(cb);
    logOne(tex);
}
