// `debugPrintfEXT` is the GLSL spelling of `printf` and takes its arguments the same way: a
// `ConstantBuffer` argument prints the buffer contents, and a resource argument is reported at
// the call site.

//TEST:SIMPLE(filecheck=CHECK): -target glsl -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG): -target glsl -stage compute -entry computeMain -DUSE_RESOURCE

import glsl;

ConstantBuffer<uint> cb;
Texture2D tex;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: debugPrintfEXT("cb=%u\n", cb_0._data);
    debugPrintfEXT("cb=%u\n", cb);
#ifdef USE_RESOURCE
    debugPrintfEXT("tex\n", tex);
/*DIAG:
                            ^^^ 'printf' argument cannot be a resource or opaque type
                            ^^^ argument of type 'Texture2D<vector<float,4>>' is or contains a resource or opaque type, which 'printf' cannot print.
*/
#endif
}
