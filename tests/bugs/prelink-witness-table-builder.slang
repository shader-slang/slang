//TEST:SIMPLE(filecheck=CHECK):-target spirv -profile spirv_1_6 -emit-spirv-directly -stage compute -entry cs_main

// This calls the core module's [__unsafeForceInlineEarly] vector<T, N> float
// constructor. Since that constructor is imported, lowering queues its definition
// for pre-link cloning. The generic T/N context exercises cloning witness-table
// dependencies while building generic full types and bodies.

vector<T, N> f<T, int N>(vector<T, N> v) where T : __BuiltinFloatingPointType
{
    return vector<T, N>(0.0f);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cs_main()
{
}

// CHECK: OpEntryPoint
