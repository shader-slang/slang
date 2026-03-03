// GPU MLP stress test for neural.slang — follows PR #42 architecture.
//
// Architecture: UV -> bilinear sample 32x32x4 latent grid -> MLP 4->128->128->128->3 -> RGB
//
// Uses PointerAddress (float*) for FFLayer — works on CUDA via device_address.
// Uses slangpy Tensor/AtomicTensor for latent grid (PR #42 pattern).
// Parallel dispatch via spy.grid() — one thread per batch sample.

import slangpy;
import neural;

static const int INPUT_DIM = 4;
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
typealias Layer3 = FFLayer<float, HiddenVec, OutputVec, LinearLayout, IdentityActivation<float>, true>;

static const uint TOTAL_PARAMS = Layer0.ParameterCount + Layer1.ParameterCount
                                + Layer2.ParameterCount + Layer3.ParameterCount;

int get_total_params() { return int(TOTAL_PARAMS); }
int get_grid_params() { return 32 * 32 * INPUT_DIM; }

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
// Differentiable MLP forward (addresses pre-computed)
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
    return Layer3().eval<Address>(h2, w3, b3);
}

// ============================================================================
// Latent texture (PR #42 pattern)
// ============================================================================
struct LatentTexture
{
    Tensor<float, 3> texture;
    AtomicTensor<float, 3> texture_grads;

    [Differentiable]
    float getLatent(int x, int y, int idx) { return texture[y, x, idx]; }

    [BackwardDerivativeOf(getLatent)]
    void getLatentBwd(int x, int y, int idx, float grad) { texture_grads.set({y, x, idx}, grad); }

    [Differentiable]
    InputVec sample(no_diff inout float2 uv)
    {
        int2 size = int2(texture.shape[1], texture.shape[0]);
        uv *= float2(size - 1);
        int2 c0 = clamp(int2(uv), int2(0), size - 1);
        int2 c1 = min(c0 + 1, size - 1);
        float2 f = saturate(uv - float2(c0));

        InputVec result;
        [ForceUnroll]
        for (int i = 0; i < INPUT_DIM; ++i)
            result[i] = lerp(lerp(getLatent(c0.x, c0.y, i), getLatent(c1.x, c0.y, i), f.x),
                lerp(getLatent(c0.x, c1.y, i), getLatent(c1.x, c1.y, i), f.x), f.y);
        return result;
    }
};

// ============================================================================
// Network (PR #42 pattern, using raw pointers for CUDA PointerAddress)
// ============================================================================
struct Network
{
    LatentTexture latent_texture;
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
    LatentTexture tex,
    no_diff int2 pixel, no_diff int2 resolution, no_diff float3 reference)
{
    float2 uv = (float2(pixel) + 0.5f) / float2(resolution);
    let out = mlp_forward(w0, b0, w1, b1, w2, b2, w3, b3, tex.sample(uv));
    float3 error = float3(out[0], out[1], out[2]) - reference;
    return error * error;
}

// ============================================================================
// Entry points — regular functions, spy.grid() dispatches in parallel
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
        network.latent_texture,
        pixel, resolution, ref_color, float3(1.0f));
}

float3 render(int2 pixel, int2 resolution, Network network)
{
    float2 uv = (float2(pixel) + 0.5f) / float2(resolution);
    LayerAddr la = make_addr(network.params);
    let out = mlp_forward(la.w0, la.b0, la.w1, la.b1, la.w2, la.b2, la.w3, la.b3,
                           network.latent_texture.sample(uv));
    return float3(out[0], out[1], out[2]);
}

float3 show_loss(int2 pixel, int2 resolution, float3 reference, Network network)
{
    LayerAddr la = make_addr(network.params);
    return loss(la.w0, la.b0, la.w1, la.b1, la.w2, la.b2, la.w3, la.b3,
                network.latent_texture, pixel, resolution, reference);
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
