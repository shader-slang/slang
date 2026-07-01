
//TEST:SIMPLE(filecheck=CHECK):-target spirv -O0

float ddx_or(float val, float defaultVal)
{
    __stage_switch
    {
        case fragment:
            return ddx(val);
        default:
            return defaultVal;
    }
}

float intermediate(float val)
{
    return ddx_or(val, 1.0);
}

RWStructuredBuffer<float> output;

[numthreads(1,1,1)]
void computeMain()
{
    // `intermediate`/`ddx_or` are specialized per reachable stage, so the
    // compute-reachable specialization of `ddx_or` is checked directly: it
    // must take the `default` branch (no `ddx` call) and simply return the
    // `defaultVal` parameter that `intermediate` passed `1.0` into.
    // CHECK-LABEL: %computeMain = OpFunction
    // CHECK: OpFunctionCall %float %intermediate %float_2
    // CHECK: OpFunctionEnd
    // CHECK-LABEL: %intermediate = OpFunction
    // CHECK: OpFunctionCall %float %ddx_or %val %float_1
    // CHECK: OpFunctionEnd
    // CHECK-LABEL: %ddx_or = OpFunction
    // CHECK-NOT: OpDPdx
    // CHECK: OpReturnValue %defaultVal
    // CHECK: OpFunctionEnd
    output[0] = intermediate(2.0);
}

[shader("fragment")]
float4 fragmentMain(float vin) : SV_Target
{
    // CHECK-LABEL: %fragmentMain = OpFunction 
    // CHECK: OpDPdx
    // CHECK: OpFunctionEnd
    return intermediate(vin);
}