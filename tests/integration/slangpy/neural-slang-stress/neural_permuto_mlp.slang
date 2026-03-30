// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Permuto Encoder + MLP image fitting test.
//
// InlineVector mode exposes regular slangpy-callable functions.
// WaveTangledVector mode exposes explicit compute entry points selected with
// -DUSE_WAVE_TANGLED=1 from Python.
//
// Half-precision variant (USE_HALF=1) uses mixed-precision training:
//   - MLP weights use PointerAddress<half> (MlpAddress) for forward/backward
//   - Feature table always uses PointerAddress<float> (FeatureAddress) because
//     PermutoEncoder<float,...> requires float-typed addresses
//   - Optimizer: compute_optimizer_step_half for half MLP params (reads half
//     grads → fp32 Adam → copies back to half), compute_optimizer_step for
//     float feature table params
//   - loss_scale is plumbed through for future use but currently 1.0

import slangpy;
import slang.neural;
import neural_test_common;

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
#ifdef USE_HALF
typealias ElemType = half;
#else
typealias ElemType = float;
#endif
typealias ShMemSize = SharedMemorySize<ElemType, TargetEnum.CUDA, ExecutionMode.Training,
                                       SUBGROUP_SIZE, SUBGROUP_COUNT>;
typealias ShMemSizeLayer4 = ShMemSize.OfLayer4<INPUT_DIM, HIDDEN_DIM, HIDDEN_DIM,
                                                HIDDEN_DIM, OUTPUT_DIM>;
typealias MlpAddress = PointerAddress<ElemType>;
typealias FeatureAddress = PointerAddress<float>;
typealias InputVec = WaveTangledVector<ElemType, ShMemSizeLayer4, INPUT_DIM, SUBGROUP_SIZE>;
typealias HiddenVec = WaveTangledVector<ElemType, ShMemSizeLayer4, HIDDEN_DIM, SUBGROUP_SIZE>;
typealias OutputVec = WaveTangledVector<ElemType, ShMemSizeLayer4, OUTPUT_DIM, SUBGROUP_SIZE>;
#else
typealias MlpAddress = PointerAddress<float>;
typealias FeatureAddress = PointerAddress<float>;
typealias InputVec = InlineVector<float, INPUT_DIM>;
typealias HiddenVec = InlineVector<float, HIDDEN_DIM>;
typealias OutputVec = InlineVector<float, OUTPUT_DIM>;
#endif

typealias PosArray = float[POS_DIMS];
typealias FeatArray = float[TOTAL_FEATURES];
typealias Encoder = PermutoEncoder<float, POS_DIMS, FEAT_PER_LEVEL, LOG2_HASHMAP_SIZE,
                                   MAX_LEVELS, PosArray, FeatArray>;
#ifdef USE_WAVE_TANGLED
typealias Layer0 = FFLayer<ElemType, InputVec, HiddenVec, LinearLayout, LeakyReLU<ElemType>>;
typealias Layer1 = FFLayer<ElemType, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<ElemType>>;
typealias Layer2 = FFLayer<ElemType, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<ElemType>>;
typealias Layer3 = FFLayer<ElemType, HiddenVec, OutputVec, LinearLayout, Sigmoid<ElemType>>;
#else
typealias Layer0 = FFLayer<float, InputVec, HiddenVec, LinearLayout, LeakyReLU<float>>;
typealias Layer1 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>>;
typealias Layer2 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, LeakyReLU<float>>;
typealias Layer3 = FFLayer<float, HiddenVec, OutputVec, LinearLayout, Sigmoid<float>>;
#endif

static const uint MLP_PARAMS = Layer0.ParameterCount + Layer1.ParameterCount
                              + Layer2.ParameterCount + Layer3.ParameterCount;
static const uint HASHMAP_SIZE = 1u << LOG2_HASHMAP_SIZE;
static const uint FEATURE_TABLE_SIZE = MAX_LEVELS * HASHMAP_SIZE * FEAT_PER_LEVEL;

int get_mlp_params() { return int(MLP_PARAMS); }
int get_feature_table_size() { return int(FEATURE_TABLE_SIZE); }


[Differentiable]
FeatArray permuto_encode(no_diff PosArray position, FeatureAddress featureAddr)
{
    FeatArray encoded = {};
    Encoder.Params levelParams;
    levelParams.maxLevel = MAX_LEVEL_RATIO * float(MAX_LEVELS);

    [MaxIters(8)]
    for (uint level = 0; level < MAX_LEVELS; ++level)
    {
        Encoder.prepareLevelInfo(level, BASE_SCALE, LOG2_PER_LEVEL_SCALE, ENCODER_SEED, levelParams.levelInfo);
        Encoder.encodePerLevel<FeatureAddress>(levelParams, position, featureAddr, encoded);
    }
    return encoded;
}

#ifdef USE_WAVE_TANGLED
[Differentiable]
OutputVec forward_wave(
    FeatureAddress featureAddr,
    MlpAddress paramAddr,
    no_diff PosArray position)
{
    FeatArray encoded = permuto_encode(position, featureAddr);
    InputVec input;
    [ForceUnroll]
    for (int i = 0; i < INPUT_DIM; ++i)
        input[i] = encoded[i];

    let act = LeakyReLU<ElemType>(ElemType(ALPHA));
    let h0 = Layer0(act).eval<MlpAddress>(input, paramAddr);
    let a1 = paramAddr.getOffset(Layer0.ParameterCount);
    let h1 = Layer1(act).eval<MlpAddress>(h0, a1);
    let a2 = a1.getOffset(Layer1.ParameterCount);
    let h2 = Layer2(act).eval<MlpAddress>(h1, a2);
    let a3 = a2.getOffset(Layer2.ParameterCount);
    return Layer3(Sigmoid<ElemType>()).eval<MlpAddress>(h2, a3);
}

OutputVec forward_inference(float* params, float* featureTable, PosArray position)
{
    return forward_wave(FeatureAddress(featureTable), MlpAddress(params), position);
}

[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_calculate_grads(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform uint seed,
    uniform int2 batch_size,
    uniform int2 img_resolution,
    uniform float loss_scale,
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
    ElemType doutArr[OUTPUT_DIM];
    doutArr[0] = ElemType(2.0f * (float(out[0]) - ref_color.x) * loss_scale / float(OUTPUT_DIM));
    doutArr[1] = ElemType(2.0f * (float(out[1]) - ref_color.y) * loss_scale / float(OUTPUT_DIM));
    doutArr[2] = ElemType(2.0f * (float(out[2]) - ref_color.z) * loss_scale / float(OUTPUT_DIM));
    OutputVec dOutput = OutputVec(doutArr);

    bwd_diff(forward_wave)(
        DifferentialPtrPair<FeatureAddress>(FeatureAddress(feature_table), FeatureAddress(feature_table_grad)),
        DifferentialPtrPair<MlpAddress>(MlpAddress(params), MlpAddress(params_grad)),
        position, dOutput);
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
    uniform float loss_scale,
    uniform int param_count)
{
    int idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    if (idx >= param_count) return;
    adam_step(primal[idx], grad[idx], mean_buf[idx], variance_buf[idx], lr, iter, loss_scale);
}

#ifdef USE_HALF
[shader("compute")]
[numthreads(WORKGROUP_SIZE, 1, 1)]
void compute_optimizer_step_half(
    uint3 gid : SV_GroupID,
    uint gtid : SV_GroupIndex,
    uniform half* primal,
    uniform half* grad,
    uniform float* primal_master,
    uniform float* mean_buf,
    uniform float* variance_buf,
    uniform float lr,
    uniform int iter,
    uniform float loss_scale,
    uniform int param_count)
{
    int idx = int(gid.x) * WORKGROUP_SZ + int(gtid);
    if (idx >= param_count) return;
    adam_step_mixed(primal[idx], grad[idx], primal_master[idx],
                    mean_buf[idx], variance_buf[idx], lr, iter, loss_scale);
}
#endif
#else
[Differentiable]
OutputVec mlp_forward(MlpAddress paramAddr, InputVec input)
{
    let act = LeakyReLU<float>(ALPHA);
    let h0 = Layer0(act).eval<MlpAddress>(input, paramAddr);
    let a1 = paramAddr.getOffset(Layer0.ParameterCount);
    let h1 = Layer1(act).eval<MlpAddress>(h0, a1);
    let a2 = a1.getOffset(Layer1.ParameterCount);
    let h2 = Layer2(act).eval<MlpAddress>(h1, a2);
    let a3 = a2.getOffset(Layer2.ParameterCount);
    return Layer3(Sigmoid<float>()).eval<MlpAddress>(h2, a3);
}

[Differentiable]
OutputVec forward(
    no_diff PosArray position,
    FeatureAddress featureAddr,
    MlpAddress paramAddr)
{
    FeatArray encoded = permuto_encode(position, featureAddr);
    InputVec input;
    [ForceUnroll]
    for (int i = 0; i < INPUT_DIM; ++i)
        input[i] = encoded[i];
    return mlp_forward(paramAddr, input);
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
    FeatureAddress featureAddr,
    MlpAddress paramAddr,
    no_diff float3 reference)
{
    let out = forward(position, featureAddr, paramAddr);
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

    FeatureAddress ft = FeatureAddress(network.feature_table);
    FeatureAddress ftg = FeatureAddress(network.feature_table_grad);

    bwd_diff(loss)(
        position,
        DifferentialPtrPair<FeatureAddress>(ft, ftg),
        DifferentialPtrPair<MlpAddress>(MlpAddress(network.params), MlpAddress(network.params_grad)),
        ref_color, float3(1.0f));
}

float3 render(int2 pixel, int2 resolution, Network network)
{
    PosArray position;
    position[0] = (float(pixel.x) + 0.5f) / float(resolution.x);
    position[1] = (float(pixel.y) + 0.5f) / float(resolution.y);

    FeatureAddress ft = FeatureAddress(network.feature_table);
    let out = forward(position, ft, MlpAddress(network.params));
    return float3(out[0], out[1], out[2]);
}

float3 show_loss(int2 pixel, int2 resolution, float3 reference, Network network)
{
    PosArray position;
    position[0] = (float(pixel.x) + 0.5f) / float(resolution.x);
    position[1] = (float(pixel.y) + 0.5f) / float(resolution.y);

    FeatureAddress ft = FeatureAddress(network.feature_table);
    return loss(position, ft, MlpAddress(network.params), reference);
}

void optimizer_step(inout float primal, inout float grad,
                    inout float mean, inout float variance,
                    float lr, int iter)
{
    adam_step(primal, grad, mean, variance, lr, iter, 1.0f /* no loss scaling */);
}
#endif
