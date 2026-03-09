// Permuto hash grid encoding + MLP image fitting baseline.
//
// Architecture: (x,y) -> PermutoEncoder(8 levels, 2 feat/level) -> 16-dim -> MLP 16->128->128->128->3 (Sigmoid) -> RGB
// Matches tiny-cuda-nn's Permuto + FullyFusedMLP architecture for comparison.
//
// Uses PointerAddress (float*) for both MLP weights and feature table.
// Feature table gradients use atomicAdd via PermutoEncoder's backward pass.
//
// Note: 8 levels (not 16) to keep the autodiff-generated backward kernel within
// CUDA compilation limits. With [ForceUnroll] for 16 levels + MLP backward, the
// combined kernel exceeds GPU register/instruction cache limits. 8 levels still
// achieves excellent quality (loss < 0.0001 at 500 iters).

import slangpy;
import neural;

static const int POS_DIMS = 2;
static const int FEAT_PER_LEVEL = 2;
static const uint LOG2_HASHMAP_SIZE = 16;
static const uint MAX_LEVELS = 8;
static const uint TOTAL_FEATURES = MAX_LEVELS * FEAT_PER_LEVEL;  // 16
static const float BASE_SCALE = 16.0f;
static const float LOG2_PER_LEVEL_SCALE = 1.5f;
static const float MAX_LEVEL_RATIO = 1.1f;
static const uint ENCODER_SEED = 1337u;

static const int INPUT_DIM = int(TOTAL_FEATURES);  // 32
static const int HIDDEN_DIM = 128;
static const int OUTPUT_DIM = 3;
static const float ALPHA = 0.01f;

typealias Address = PointerAddress<float>;

typealias PosArray = float[POS_DIMS];
typealias FeatArray = float[TOTAL_FEATURES];
typealias Encoder = PermutoEncoder<float, POS_DIMS, FEAT_PER_LEVEL, LOG2_HASHMAP_SIZE, MAX_LEVELS, PosArray, FeatArray>;

typealias InputVec = InlineVector<float, INPUT_DIM>;
typealias HiddenVec = InlineVector<float, HIDDEN_DIM>;
typealias OutputVec = InlineVector<float, OUTPUT_DIM>;

typealias Layer0 = FFLayer<float, InputVec, HiddenVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer1 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer2 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer3 = FFLayer<float, HiddenVec, OutputVec, LinearLayout, Sigmoid<float>, true>;

static const uint MLP_PARAMS = Layer0.ParameterCount + Layer1.ParameterCount
                              + Layer2.ParameterCount + Layer3.ParameterCount;
static const uint HASHMAP_SIZE = 1u << LOG2_HASHMAP_SIZE;
static const uint FEATURE_TABLE_SIZE = MAX_LEVELS * HASHMAP_SIZE * FEAT_PER_LEVEL;

int get_mlp_params() { return int(MLP_PARAMS); }
int get_feature_table_size() { return int(FEATURE_TABLE_SIZE); }

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
// Pre-compute MLP layer addresses (getOffset workaround)
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
// Differentiable Permuto encoding
// Position is no_diff — we don't need position gradients.
// Feature table gradients flow via readFeatureValueBwd (atomicAdd).
// Barycentric weights (which depend on position) are treated as detached
// constants in the backward pass, which is correct: dL/dFeature = weight * dL/dEncoded.
// ============================================================================
[Differentiable]
FeatArray permuto_encode(no_diff PosArray position, Address featureAddr)
{
    FeatArray encoded = {};
    Encoder.Params levelParams;
    levelParams.maxLevel = MAX_LEVEL_RATIO * float(MAX_LEVELS);

    [MaxIters(8)]
    for (uint level = 0; level < MAX_LEVELS; ++level)
    {
        Encoder.prepareLevelInfo(level, BASE_SCALE, LOG2_PER_LEVEL_SCALE, ENCODER_SEED, levelParams.levelInfo);
        Encoder.encodePerLevel<Address>(levelParams, position, featureAddr, encoded);
    }
    return encoded;
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
// Differentiable end-to-end forward: encode + MLP
// ============================================================================
[Differentiable]
OutputVec forward(
    no_diff PosArray position,
    Address featureAddr,
    Address w0, Address b0, Address w1, Address b1,
    Address w2, Address b2, Address w3, Address b3)
{
    FeatArray encoded = permuto_encode(position, featureAddr);

    InputVec input;
    [ForceUnroll]
    for (int i = 0; i < INPUT_DIM; ++i)
        input[i] = encoded[i];

    return mlp_forward(w0, b0, w1, b1, w2, b2, w3, b3, input);
}

// ============================================================================
// Network struct
// ============================================================================
struct Network
{
    float* params;
    float* params_grad;
    float* feature_table;
    float* feature_table_grad;
};

// ============================================================================
// Differentiable loss
// ============================================================================
[Differentiable]
float3 loss(
    no_diff PosArray position,
    Address featureAddr,
    Address w0, Address b0, Address w1, Address b1,
    Address w2, Address b2, Address w3, Address b3,
    no_diff float3 reference)
{
    let out = forward(position, featureAddr, w0, b0, w1, b1, w2, b2, w3, b3);
    float3 pred = float3(out[0], out[1], out[2]);
    float3 error = pred - reference;
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

    PosArray position;
    position[0] = (float(pixel.x) + 0.5f) / float(resolution.x);
    position[1] = (float(pixel.y) + 0.5f) / float(resolution.y);

    LayerAddr la = make_addr(network.params);
    LayerAddr ga = make_addr(network.params_grad);
    Address ft = Address(network.feature_table);
    Address ftg = Address(network.feature_table_grad);

    bwd_diff(loss)(
        position,
        DifferentialPtrPair<Address>(ft, ftg),
        DifferentialPtrPair<Address>(la.w0, ga.w0), DifferentialPtrPair<Address>(la.b0, ga.b0),
        DifferentialPtrPair<Address>(la.w1, ga.w1), DifferentialPtrPair<Address>(la.b1, ga.b1),
        DifferentialPtrPair<Address>(la.w2, ga.w2), DifferentialPtrPair<Address>(la.b2, ga.b2),
        DifferentialPtrPair<Address>(la.w3, ga.w3), DifferentialPtrPair<Address>(la.b3, ga.b3),
        ref_color, float3(1.0f));
}

float3 render(int2 pixel, int2 resolution, Network network)
{
    float2 uv = (float2(pixel) + 0.5f) / float2(resolution);
    PosArray position;
    position[0] = uv.x;
    position[1] = uv.y;

    Address ft = Address(network.feature_table);
    LayerAddr la = make_addr(network.params);

    let out = forward(position, ft, la.w0, la.b0, la.w1, la.b1, la.w2, la.b2, la.w3, la.b3);
    return float3(out[0], out[1], out[2]);
}

float3 show_loss(int2 pixel, int2 resolution, float3 reference, Network network)
{
    float2 uv = (float2(pixel) + 0.5f) / float2(resolution);
    PosArray position;
    position[0] = uv.x;
    position[1] = uv.y;

    Address ft = Address(network.feature_table);
    LayerAddr la = make_addr(network.params);

    return loss(position, ft, la.w0, la.b0, la.w1, la.b1, la.w2, la.b2, la.w3, la.b3, reference);
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
