module kernels;

import common;
import mlp;
import network;
import adam;

[numthreads(256, 1, 1)]
[require(spvGroupNonUniformBallot, spvGroupNonUniformArithmetic, spvCooperativeVectorNV)]
void learnGradient(
    uint32_t tid : SV_DispatchThreadID,
    uniform MyNetwork* network, 
    uniform Atomic<uint32_t>* lossBuffer,
    uniform float2* inputs,
    uniform uint32_t count)
{
    if (tid >= count)
        return;

    var input = (half2)inputs[tid];
    bwd_diff(loss)(network, input.x, input.y, 1.0h);
    let thisLoss = (float)loss(network, input.x, input.y);
    let maxLoss = WaveActiveMax(thisLoss);
    if (WaveIsFirstLane())
    {
        lossBuffer.max(bit_cast<uint32_t>(maxLoss));
    }
}

[numthreads(256, 1, 1)]
void adjustParameters(uint32_t tid : SV_DispatchThreadID, uniform AdamState* states, uniform NFloat* params, uniform NFloat* gradients, uniform uint32_t count)
{
    if (tid >= count)
        return;
    if (isnan(gradients[tid]))
    {
        gradients[tid] = 0.0h;
        return;
    }
    AdamOptimizer::step(states[tid], params[tid], gradients[tid]);
}