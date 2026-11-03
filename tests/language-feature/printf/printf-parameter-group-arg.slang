// A `ConstantBuffer` or `ParameterBlock` argument to `printf` is passed by value: the compiler
// dereferences it the same way it does for `uint v = cb;`, and every argument of a mixed call
// reaches the target print call.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain

ConstantBuffer<uint> cb;
ParameterBlock<uint> pb;
ConstantBuffer<float4> cbv;

// The source-language targets emit this function before `computeMain`.
// HLSL: printf("param=%u\n", cb_0);
// GLSL: debugPrintfEXT("param=%u\n", cb_0._data);
// METAL: os_log_default.log("param=%u\n", *{{.*}}cb_0);
// CUDA: [[PARAM:_S[0-9]+]] = __ldg(globalParams_0->cb_0);
// CUDA: printf("param=%u\n", [[PARAM]]);
// CPP: printf("param=%u\n", *{{.*}}->cb_0);
void printFromParam(ConstantBuffer<uint> p)
{
    printf("param=%u\n", p);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // HLSL: printf("cb=%u\n", cb_0);
    // GLSL: debugPrintfEXT("cb=%u\n", cb_0._data);
    // METAL: os_log_default.log("cb=%u\n", *{{.*}}cb_{{[0-9]+}});
    // CUDA: [[CB:_S[0-9]+]] = __ldg(globalParams_0->cb_0);
    // CUDA: printf("cb=%u\n", [[CB]]);
    // CPP: printf("cb=%u\n", *{{.*}}->cb_0);
    // SPIRV: %[[CB:[0-9]+]] = OpLoad %uint
    // SPIRV: DebugPrintf %{{[0-9]+}} %[[CB]]{{$}}
    printf("cb=%u\n", cb);

    // HLSL: printf("pb=%u\n", pb_0);
    // GLSL: debugPrintfEXT("pb=%u\n", pb_0._data);
    // METAL: os_log_default.log("pb=%u\n", *{{.*}}pb_0);
    // CUDA: [[PB:_S[0-9]+]] = __ldg(globalParams_0->pb_0);
    // CUDA: printf("pb=%u\n", [[PB]]);
    // CPP: printf("pb=%u\n", *{{.*}}->pb_0);
    // SPIRV: %[[PB:[0-9]+]] = OpLoad %uint
    // SPIRV: DebugPrintf %{{[0-9]+}} %[[PB]]{{$}}
    printf("pb=%u\n", pb);

    // HLSL: printf("cbv=%v4f\n", cbv_0);
    // GLSL: debugPrintfEXT("cbv=%v4f\n", cbv_0._data);
    // METAL: os_log_default.log("cbv=%v4f\n", *{{.*}}cbv_0);
    // CUDA: [[CBV:_S[0-9]+]] = __ldg(globalParams_0->cbv_0);
    // CUDA: printf("cbv=%v4f\n", [[CBV]]);
    // CPP: printf("cbv=%v4f\n", *{{.*}}->cbv_0);
    // SPIRV: %[[CBV:[0-9]+]] = OpLoad %v4float
    // SPIRV: DebugPrintf %{{[0-9]+}} %[[CBV]]{{$}}
    printf("cbv=%v4f\n", cbv);

    // HLSL: printf("a=%u cb=%u b=%u\n", 7U, {{[_a-zA-Z0-9]+}}, 9U);
    // GLSL: debugPrintfEXT("a=%u cb=%u b=%u\n", 7U, {{[_a-zA-Z0-9.]+}}, 9U);
    // METAL: os_log_default.log("a=%u cb=%u b=%u\n", 7U, {{[_a-zA-Z0-9]+}}, 9U);
    // CUDA: [[MIXED:_S[0-9]+]] = __ldg(globalParams_0->cb_0);
    // CUDA: printf("a=%u cb=%u b=%u\n", 7U, [[MIXED]], 9U);
    // CPP: printf("a=%u cb=%u b=%u\n", 7U, *{{.*}}->cb_0, 9U);
    // SPIRV: DebugPrintf %{{[0-9]+}} %uint_7 %{{[0-9]+}} %uint_9{{$}}
    printf("a=%u cb=%u b=%u\n", 7u, cb, 9u);

    // SPIR-V emits `printFromParam` after `computeMain`.
    // SPIRV: %[[PARAM:[0-9]+]] = OpLoad %uint
    // SPIRV: DebugPrintf %{{[0-9]+}} %[[PARAM]]{{$}}
    printFromParam(cb);
}
