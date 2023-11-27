//TEST:SIMPLE(filecheck=CHECK): -stage fragment -target spirv -emit-spirv-directly -entry main

RWStructuredBuffer<float> output;

void main()
{
    {
        // CHECK: OpFwidth
        float w = 1.0;
        float b = fwidth(w);
        output[0] = b;

        // CHECK: OpDPdx
        float b1 = ddx(w);
        output[1] = b1;
        // CHECK: OpDPdy
        float b2 = ddy(w);
        output[2] = b2;

        // CHECK: OpDPdxCoarse
        float b3 = ddx_coarse(w);
        output[3] = b3;

        // CHECK: OpDPdyCoarse
        float b4 = ddy_coarse(w);
        output[4] = b4;

        // CHECK: OpDPdxFine
        float b5 = ddx_fine(w);
        output[5] = b5;

        // CHECK: OpDPdyFine
        float b6 = ddy_fine(w);
        output[6] = b6;
    }

    {
        // CHECK: OpFwidth
        float3 w = 1.0;
        float3 b = fwidth(w);
        output[7] = b.x;

        // CHECK: OpDPdx
        float3 b1 = ddx(w);
        output[8] = b1.x;
        // CHECK: OpDPdy
        float3 b2 = ddy(w);
        output[9] = b2.x;

        // CHECK: OpDPdxCoarse
        float3 b3 = ddx_coarse(w);
        output[10] = b3.x;

        // CHECK: OpDPdyCoarse
        float3 b4 = ddy_coarse(w);
        output[11] = b4.x;

        // CHECK: OpDPdxFine
        float3 b5 = ddx_fine(w);
        output[12] = b5.x;

        // CHECK: OpDPdyFine
        float3 b6 = ddy_fine(w);
        output[13] = b6.x;
    }
}
