// Permuto Encoder + MLP image fitting test.
//
// InlineVector mode exposes regular slangpy-callable functions.
// WaveTangledVector mode exposes explicit compute entry points selected with
// -DUSE_WAVE_TANGLED=1 from Python.

import slangpy;
import slang.neural;

static const int POS_DIMS = 2;
static const int FEAT_PER_LEVEL = 2;
static const uint LOG2_HASHMAP_SIZE = 16;
static const uint MAX_LEVELS = 8;
static const uint TOTAL_FEATURES = MAX_LEVELS * FEAT_PER_LEVEL;
static const float BASE_SCALE = 16.0f;
static const float LOG2_PER_LEVEL_SCALE = 1.5f;
static const float MAX_LEVEL_RATIO = 1.1f;
static const uint ENCODER_SEED = 1337u;
static const int INPUT_DIM = int(TOTAL_FEATURES);
static const int HIDDEN_DIM = 128;
static const int OUTPUT_DIM = 3;
static const float ALPHA = 0.01f;

#ifdef USE_WAVE_TANGLED
static const int SUBGROUP_SIZE = 32;
#ifndef WORKGROUP_SIZE
#define WORKGROUP_SIZE 64
#endif
static const int WORKGROUP_SZ = WORKGROUP_SIZE;
static const int SUBGROUP_COUNT = WORKGROUP_SZ / SUBGROUP_SIZE;
typealias ElemType = float;
typealias ShMemSize = SharedMemorySize<ElemType, TargetEnum.CUDA, ExecutionMode.Training,
                                       SUBGROUP_SIZE, SUBGROUP_COUNT>;
typealias ShMemSizeLayer4 = ShMemSize.OfLayer4<INPUT_DIM, HIDDEN_DIM, HIDDEN_DIM,
                                                HIDDEN_DIM, OUTPUT_DIM>;
typealias Address = PointerAddress<ElemType>;
typealias InputVec = WaveTangledVector<ElemType, ShMemSizeLayer4, INPUT_DIM, SUBGROUP_SIZE>;
typealias HiddenVec = WaveTangledVector<ElemType, ShMemSizeLayer4, HIDDEN_DIM, SUBGROUP_SIZE>;
typealias OutputVec = WaveTangledVector<ElemType, ShMemSizeLayer4, OUTPUT_DIM, SUBGROUP_SIZE>;
#else
typealias Address = PointerAddress<float>;
typealias InputVec = InlineVector<float, INPUT_DIM>;
typealias HiddenVec = InlineVector<float, HIDDEN_DIM>;
typealias OutputVec = InlineVector<float, OUTPUT_DIM>;
#endif

typealias PosArray = float[POS_DIMS];
typealias FeatArray = float[TOTAL_FEATURES];
typealias Encoder = PermutoEncoder<float, POS_DIMS, FEAT_PER_LEVEL, LOG2_HASHMAP_SIZE,
                                   MAX_LEVELS, PosArray, FeatArray>;
typealias Layer0 = FFLayer<float, InputVec, HiddenVec, LinearLayout, LeakyReLU<float>>;
typealias Layer1 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>>;
typealias Layer2 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>>;
typealias Layer3 = FFLayer<float, HiddenVec, OutputVec, LinearLayout, Sigmoid<float>>;

static const uint MLP_PARAMS = Layer0.ParameterCount + Layer1.ParameterCount
                              + Layer2.ParameterCount + Layer3.ParameterCount;
static const uint HASHMAP_SIZE = 1u << LOG2_HASHMAP_SIZE;
static const uint FEATURE_TABLE_SIZE = MAX_LEVELS * HASHMAP_SIZE * FEAT_PER_LEVEL;

int get_mlp_params() { return int(MLP_PARAMS); }
int get_feature_table_size() { return int(FEATURE_TABLE_SIZE); }

struct LCG
{
    uint state;
    __init(uint seed) { state = seed; }
    [mutating] uint next_uint() { state = 1664525u * state + 1013904223u; return state; }
    [mutating] float next_float() { return (next_uint() >> 8) * 0x1p-24; }
};

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

#ifdef USE_WAVE_TANGLED
[Differentiable]
OutputVec forward_wave(
    Address featureAddr,
    Address w0, Address b0, Address w1, Address b1,
    Address w2, Address b2, Address w3, Address b3,
    no_diff PosArray position)
{
    FeatArray encoded = permuto_encode(position, featureAddr);
    InputVec input;
    [ForceUnroll]
    for (int i = 0; i < INPUT_DIM; ++i)
        input[i] = encoded[i];

    let act = LeakyReLU<float>(ALPHA);
    let h0 = Layer0(act).eval<Address>(input, w0, b0);
    let h1 = Layer1(act).eval<Address>(h0, w1, b1);
    let h2 = Layer2(act).eval<Address>(h1, w2, b2);
    return Layer3(Sigmoid<float>()).eval<Address>(h2, w3, b3);
}

OutputVec forward_inference(float* params, float* featureTable, PosArray position)
{
    FeatArray encoded = permuto_encode(position, Address(featureTable));
    InputVec input;
    [ForceUnroll]
    for (int i = 0; i < INPUT_DIM; ++i)
        input[i] = encoded[i];

    LayerAddr la = make_addr(params);
    let act = LeakyReLU<float>(ALPHA);
    let h0 = Layer0(act).eval<Address>(input, la.w0, la.b0);
    let h1 = Layer1(act).eval<Address>(h0, la.w1, la.b1);
    let h2 = Layer2(act).eval<Address>(h1, la.w2, la.b2);
    return Layer3(Sigmoid<float>()).eval<Address>(h2, la.w3, la.b3);
}

[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_calculate_grads(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform uint seed,
    uniform int2 batch_size,
    uniform int2 img_resolution,
    uniform float* ref_image,
    uniform float* params,
    uniform float* params_grad,
    uniform float* feature_table,
    uniform float* feature_table_grad)
{
    int sample_idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    int batch_count = batch_size.x * batch_size.y;
    if (sample_idx >= batch_count) return;

    LCG lcg = LCG(seed + uint(sample_idx));
    int2 batch_index = int2(sample_idx % batch_size.x, sample_idx / batch_size.x);
    float2 uv = (float2(batch_index) + float2(lcg.next_float(), lcg.next_float()))
                / float2(batch_size);
    int2 pixel = clamp(int2(uv * float2(img_resolution)), int2(0), img_resolution - 1);

    int ref_idx = (pixel.y * img_resolution.x + pixel.x) * 3;
    float3 ref_color = float3(ref_image[ref_idx], ref_image[ref_idx + 1], ref_image[ref_idx + 2]);

    PosArray position;
    position[0] = (float(pixel.x) + 0.5f) / float(img_resolution.x);
    position[1] = (float(pixel.y) + 0.5f) / float(img_resolution.y);

    OutputVec out = forward_inference(params, feature_table, position);
    float doutArr[OUTPUT_DIM];
    doutArr[0] = 2.0f * (float(out[0]) - ref_color.x) / float(OUTPUT_DIM);
    doutArr[1] = 2.0f * (float(out[1]) - ref_color.y) / float(OUTPUT_DIM);
    doutArr[2] = 2.0f * (float(out[2]) - ref_color.z) / float(OUTPUT_DIM);
    OutputVec dOutput = OutputVec(doutArr);

    LayerAddr la = make_addr(params);
    LayerAddr ga = make_addr(params_grad);
    bwd_diff(forward_wave)(
        DifferentialPtrPair<Address>(Address(feature_table), Address(feature_table_grad)),
        DifferentialPtrPair<Address>(la.w0, ga.w0), DifferentialPtrPair<Address>(la.b0, ga.b0),
        DifferentialPtrPair<Address>(la.w1, ga.w1), DifferentialPtrPair<Address>(la.b1, ga.b1),
        DifferentialPtrPair<Address>(la.w2, ga.w2), DifferentialPtrPair<Address>(la.b2, ga.b2),
        DifferentialPtrPair<Address>(la.w3, ga.w3), DifferentialPtrPair<Address>(la.b3, ga.b3),
        position,
        dOutput);
}

[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_render(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform int2 img_resolution,
    uniform float* params,
    uniform float* feature_table,
    uniform float* output_image)
{
    int pixel_idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    int total_pixels = img_resolution.x * img_resolution.y;
    if (pixel_idx >= total_pixels) return;

    int2 pixel = int2(pixel_idx % img_resolution.x, pixel_idx / img_resolution.x);
    PosArray position;
    position[0] = (float(pixel.x) + 0.5f) / float(img_resolution.x);
    position[1] = (float(pixel.y) + 0.5f) / float(img_resolution.y);
    OutputVec out = forward_inference(params, feature_table, position);

    int out_idx = pixel_idx * 3;
    output_image[out_idx + 0] = float(out[0]);
    output_image[out_idx + 1] = float(out[1]);
    output_image[out_idx + 2] = float(out[2]);
}

[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_show_loss(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform int2 img_resolution,
    uniform float* params,
    uniform float* feature_table,
    uniform float* ref_image,
    uniform float* output_loss)
{
    int pixel_idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    int total_pixels = img_resolution.x * img_resolution.y;
    if (pixel_idx >= total_pixels) return;

    int2 pixel = int2(pixel_idx % img_resolution.x, pixel_idx / img_resolution.x);
    PosArray position;
    position[0] = (float(pixel.x) + 0.5f) / float(img_resolution.x);
    position[1] = (float(pixel.y) + 0.5f) / float(img_resolution.y);
    OutputVec out = forward_inference(params, feature_table, position);

    int ref_idx = pixel_idx * 3;
    float3 ref_color = float3(ref_image[ref_idx], ref_image[ref_idx + 1], ref_image[ref_idx + 2]);
    output_loss[ref_idx + 0] = pow(float(out[0]) - ref_color.x, 2);
    output_loss[ref_idx + 1] = pow(float(out[1]) - ref_color.y, 2);
    output_loss[ref_idx + 2] = pow(float(out[2]) - ref_color.z, 2);
}

[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_optimizer_step(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform float* primal,
    uniform float* grad,
    uniform float* mean_buf,
    uniform float* variance_buf,
    uniform float lr,
    uniform int iter,
    uniform int param_count)
{
    int idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    if (idx >= param_count) return;

    const float BETA1 = 0.9f, BETA2 = 0.999f, EPSILON = 1e-8f;
    float g = grad[idx];
    float m = lerp(g, mean_buf[idx], BETA1);
    float v = lerp(g * g, variance_buf[idx], BETA2);
    float m_hat = m / (1.0f - pow(BETA1, iter));
    float v_hat = v / (1.0f - pow(BETA2, iter));
    primal[idx] -= lr * m_hat / (sqrt(v_hat) + EPSILON);
    grad[idx] = 0.0f;
    mean_buf[idx] = m;
    variance_buf[idx] = v;
}
#else
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

struct Network
{
    float* params;
    float* params_grad;
    float* feature_table;
    float* feature_table_grad;
};

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

void calculate_grads(uint seed, int2 batch_index, int2 batch_size,
                     Tensor<float3, 2> reference, Network network)
{
    LCG lcg = LCG(seed);
    float2 uv = (batch_index + float2(lcg.next_float(), lcg.next_float())) / float2(batch_size);
    int2 resolution = int2(reference.shape[1], reference.shape[0]);
    int2 pixel = int2(uv * float2(resolution));
    float3 ref_color = reference[pixel];

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
    PosArray position;
    position[0] = (float(pixel.x) + 0.5f) / float(resolution.x);
    position[1] = (float(pixel.y) + 0.5f) / float(resolution.y);

    Address ft = Address(network.feature_table);
    LayerAddr la = make_addr(network.params);
    let out = forward(position, ft, la.w0, la.b0, la.w1, la.b1, la.w2, la.b2, la.w3, la.b3);
    return float3(out[0], out[1], out[2]);
}

float3 show_loss(int2 pixel, int2 resolution, float3 reference, Network network)
{
    PosArray position;
    position[0] = (float(pixel.x) + 0.5f) / float(resolution.x);
    position[1] = (float(pixel.y) + 0.5f) / float(resolution.y);

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
#endif
