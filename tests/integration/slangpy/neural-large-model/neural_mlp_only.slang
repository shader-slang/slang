// MLP-only image fitting baseline — no spatial encoding.
//
// Architecture: (x,y) -> MLP 2->128->128->128->3 (Sigmoid) -> RGB
// Matches tiny-cuda-nn's FullyFusedMLP test for direct comparison.
//
// Uses PointerAddress (float*) for FFLayer — works on CUDA via device_address.
// Parallel dispatch via spy.grid() — one thread per batch sample.

import slangpy;
import neural;

static const int INPUT_DIM = 2;
static const int HIDDEN_DIM = 128;
static const int OUTPUT_DIM = 3;
static const float ALPHA = 0.01f;

typealias Address = PointerAddress<float>;
typealias InputVec = InlineVector<float, INPUT_DIM>;
typealias HiddenVec = InlineVector<float, HIDDEN_DIM>;
typealias OutputVec = InlineVector<float, OUTPUT_DIM>;

typealias Layer0 = FFLayer<float, InputVec, HiddenVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer1 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer2 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer3 = FFLayer<float, HiddenVec, OutputVec, LinearLayout, Sigmoid<float>, true>;

static const uint TOTAL_PARAMS = Layer0.ParameterCount + Layer1.ParameterCount
                                + Layer2.ParameterCount + Layer3.ParameterCount;

int get_total_params() { return int(TOTAL_PARAMS); }

// ============================================================================
// LCG
// ============================================================================
struct LCG
{
    uint state;
    __init(uint seed) { state = seed; }
    [mutating] uint next_uint() { state = 1664525u * state + 1013904223u; return state; }
    [mutating] float next_float() { return (next_uint() >> 8) * 0x1p-24; }
};

// ============================================================================
// Pre-compute layer addresses outside differentiable scope (getOffset workaround)
// ============================================================================
struct LayerAddr { Address w0, b0, w1, b1, w2, b2, w3, b3; };

LayerAddr make_addr(float* ptr)
{
    Address base = Address(ptr);
    LayerAddr a;
    uint off = 0u;
    a.w0 = base.getOffset(off); a.b0 = base.getOffset(off + INPUT_DIM * HIDDEN_DIM);
    off += Layer0.ParameterCount;
    a.w1 = base.getOffset(off); a.b1 = base.getOffset(off + HIDDEN_DIM * HIDDEN_DIM);
    off += Layer1.ParameterCount;
    a.w2 = base.getOffset(off); a.b2 = base.getOffset(off + HIDDEN_DIM * HIDDEN_DIM);
    off += Layer2.ParameterCount;
    a.w3 = base.getOffset(off); a.b3 = base.getOffset(off + HIDDEN_DIM * OUTPUT_DIM);
    return a;
}

// ============================================================================
// Differentiable MLP forward
// ============================================================================
[Differentiable]
OutputVec mlp_forward(
    Address w0, Address b0, Address w1, Address b1,
    Address w2, Address b2, Address w3, Address b3,
    InputVec input)
{
    let act = LeakyReLU<float>(ALPHA);
    let h0 = Layer0(act).eval<Address>(input, w0, b0);
    let h1 = Layer1(act).eval<Address>(h0, w1, b1);
    let h2 = Layer2(act).eval<Address>(h1, w2, b2);
    return Layer3(Sigmoid<float>()).eval<Address>(h2, w3, b3);
}

// ============================================================================
// Network — just MLP params, no spatial encoding
// ============================================================================
struct Network
{
    float* params;
    float* params_grad;
};

// ============================================================================
// Differentiable loss
// ============================================================================
[Differentiable]
float3 loss(
    Address w0, Address b0, Address w1, Address b1,
    Address w2, Address b2, Address w3, Address b3,
    no_diff int2 pixel, no_diff int2 resolution, no_diff float3 reference)
{
    float2 uv = (float2(pixel) + 0.5f) / float2(resolution);
    InputVec input;
    input[0] = uv.x;
    input[1] = uv.y;
    let out = mlp_forward(w0, b0, w1, b1, w2, b2, w3, b3, input);
    float3 error = float3(out[0], out[1], out[2]) - reference;
    return error * error;
}

// ============================================================================
// Entry points
// ============================================================================

void calculate_grads(uint seed, int2 batch_index, int2 batch_size,
                     Tensor<float3, 2> reference, Network network)
{
    LCG lcg = LCG(seed);
    float2 uv = (batch_index + float2(lcg.next_float(), lcg.next_float())) / float2(batch_size);
    int2 resolution = int2(reference.shape[1], reference.shape[0]);
    int2 pixel = int2(uv * float2(resolution));
    float3 ref_color = reference.getv(pixel);

    LayerAddr la = make_addr(network.params);
    LayerAddr ga = make_addr(network.params_grad);

    bwd_diff(loss)(
        DifferentialPtrPair<Address>(la.w0, ga.w0), DifferentialPtrPair<Address>(la.b0, ga.b0),
        DifferentialPtrPair<Address>(la.w1, ga.w1), DifferentialPtrPair<Address>(la.b1, ga.b1),
        DifferentialPtrPair<Address>(la.w2, ga.w2), DifferentialPtrPair<Address>(la.b2, ga.b2),
        DifferentialPtrPair<Address>(la.w3, ga.w3), DifferentialPtrPair<Address>(la.b3, ga.b3),
        pixel, resolution, ref_color, float3(1.0f));
}

float3 render(int2 pixel, int2 resolution, Network network)
{
    float2 uv = (float2(pixel) + 0.5f) / float2(resolution);
    InputVec input;
    input[0] = uv.x;
    input[1] = uv.y;
    LayerAddr la = make_addr(network.params);
    let out = mlp_forward(la.w0, la.b0, la.w1, la.b1, la.w2, la.b2, la.w3, la.b3, input);
    return float3(out[0], out[1], out[2]);
}

float3 show_loss(int2 pixel, int2 resolution, float3 reference, Network network)
{
    LayerAddr la = make_addr(network.params);
    return loss(la.w0, la.b0, la.w1, la.b1, la.w2, la.b2, la.w3, la.b3,
                pixel, resolution, reference);
}

void optimizer_step(inout float primal, inout float grad,
                    inout float mean, inout float variance,
                    float lr, int iter)
{
    const float BETA1 = 0.9f, BETA2 = 0.999f, EPSILON = 1e-8f;
    mean = lerp(grad, mean, BETA1);
    variance = lerp(grad * grad, variance, BETA2);
    float m_hat = mean / (1.0f - pow(BETA1, iter));
    float v_hat = variance / (1.0f - pow(BETA2, iter));
    primal -= lr * m_hat / (sqrt(v_hat) + EPSILON);
    grad = 0.0f;
}
