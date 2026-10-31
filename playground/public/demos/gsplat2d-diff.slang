/* 
 * 2D Gaussian Splatting Example in Slang
 *
 * This example demonstrates the use of Slang's differentiable programming capabilities to implement 
 * a 2D Gaussian splatting algorithm that can be trained within the browser using the Slang Playground.
 * 
 * This algorithm represents a simplified version of the 3D Gaussian Splatting algorithm detailed in 
 * this paper (https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/). 
 * This 2D demonstration does not have the 3D->2D projection step & assumes that the Gaussian blobs 
 * are presented in order of depth (higher index = farther away). Further, this implementation does 
 * not perform adaptive density control to add or remove blobs.
 * 
 * See the `computeDerivativesMain()` kernel and the `splatBlobs()` function for the bulk of the key
 * pieces of the code. 
 *
 * Key Slang features used in this example include the autodiff operator `bwd_diff(fn)`, the 
 * `[Differentiable]` attribute, and custom derivatives for a few specific components via 
 * the `[BackwardDerivative(fn)]` attribute.
 * 
 * For a full 3D Gaussian Splatting implementation written in Slang, see this repository: 
 * https://github.com/google/slang-gaussian-rasterization
 *
 */

import playground;

// ----- Constants and definitions --------

static const int BLOB_BUFFER_SIZE = 184320;
static const int NUM_FIELDS = 9;

static const int GAUSSIANS_PER_BLOCK = 256;
static const int WG_X = 16;
static const int WG_Y = 16;

static const float ADAM_ETA = 0.002;
static const float ADAM_BETA_1 = 0.9;
static const float ADAM_BETA_2 = 0.999;
static const float ADAM_EPSILON = 1e-8;

// ------ Global buffers and textures --------

// The playground attributes instruct slang-playground to allocate and initialize the buffers 
// with the appropriate data. 
//
// When using this sample code locally, your own engine is responsible for allocating, 
// initializing & binding these buffers.
//

[playground::RAND(BLOB_BUFFER_SIZE)]
RWStructuredBuffer<float> blobsBuffer;

[playground::ZEROS(BLOB_BUFFER_SIZE)]
RWStructuredBuffer<Atomic<uint>> derivBuffer;

[playground::ZEROS(BLOB_BUFFER_SIZE)]
RWStructuredBuffer<float> adamFirstMoment;

[playground::ZEROS(BLOB_BUFFER_SIZE)]
RWStructuredBuffer<float> adamSecondMoment;

[playground::URL("static/jeep.jpg")]
[format("rgba8")]
Texture2D<float4> targetTexture;

// ----- Shared memory declarations --------

// Note: In Slang, the 'groupshared' identifier is used to define
// workgroup-level shared memory. This is equivalent to '__shared__' in CUDA

// blobCountAT is used when storing blob IDs into the blobs buffer. It needs to be atomic 
// since multiple threads will be in contention to increment it.
//
// Atomic<T> is the most portable way to express atomic operations. Slang supports basic 
// operations like +, -, ++, etc.. on Atomic<T> types.
// 
groupshared Atomic<uint> blobCountAT;

// This is used after the coarse rasterization step as a non-atomic 
// location to store the blob count, since atomics are not necessary after the coarse
// rasterization step.
//
groupshared uint blobCount;

// The blobs buffer is used to store the indices of the blobs that intersect 
// with the current tile.
//
groupshared uint blobs[GAUSSIANS_PER_BLOCK];

// The maxCount and finalVal buffers are used to store the final PixelState objects
// after the forward pass. This data is read-back for the backwards pass.
// 
groupshared uint maxCount[WG_X * WG_Y];
groupshared float4 finalVal[WG_X * WG_Y];

// The reductionBuffer is used for the binary reduction in the loadFloat_bwd() function.
groupshared float reductionBuffer[WG_X * WG_Y];

// -----------------------------------------

// Some types to hold state info on the 'blobs' buffer.
// This makes it easy to make sure we're not accidentally using the buffer
// in the wrong state.
//
// The actual data is in the 'blobs' object.
//
struct InitializedShortList { int _dummy = 0; };
struct FilledShortList { int _dummy = 0; };
struct PaddedShortList { int _dummy = 0; };
struct SortedShortList { int _dummy = 0; };

/* 
* Oriented bounding box (OBB) data-structure
*
* Can be used to represent the bounds of an anisotropic Gaussian blob.
* The bounding box can be extracted by taking a canonical box 
* formed by (-1,-1), (1,-1), (1,1), (-1,1), then translating, rotating, and scaling it.
*/
struct OBB
{
    float2 center;
    float2x2 rotation;
    float2 scale;

    /* 
    * intersects() returns true if the OBB intersects with another OBB.
    * 
    * The implementation is based on the separating axis theorem (see 
    * https://dyn4j.org/2010/01/sat/#sat-algo for a detailed explanation). 
    * At a high level, the SAT algorithm checks if the projections of the 
    * points of the two OBBs are disjoint along the normals of all of the 
    * faces of each OBB.
    */
    bool intersects(OBB other)
    {
        float2 canonicalPts[4] = float2[4](float2(-1, -1), float2(1, -1), float2(1, 1), float2(-1, 1));

        float2x2 invRotation = inverse(rotation);
        float2x2 otherInvRotation = inverse(other.rotation);
        float2 pts[4];
        for (int i = 0; i < 4; i++)
            pts[i] = center + float2(
                                dot(invRotation[0], (canonicalPts[i] * scale)),
                                dot(invRotation[1], (canonicalPts[i] * scale)));
    
        float2 otherPts[4];
        for (int i = 0; i < 4; i++)
            otherPts[i] = other.center + float2(
                dot(otherInvRotation[0], (canonicalPts[i] * other.scale)),
                dot(otherInvRotation[1], (canonicalPts[i] * other.scale)));

        return !(arePtsSeparatedAlongAxes(pts, otherPts, rotation) ||
                arePtsSeparatedAlongAxes(pts, otherPts, other.rotation));
    }

    static bool arePtsSeparatedAlongAxes(float2[4] pts, float2[4] otherPts, float2x2 axes)
    {
        // If any set of points are entirely on one side of the other, they are separated.
        //
        for (int i = 0; i < 2; i++)
        {
            float2 axis = axes[i];
            float2 proj = float2(dot(pts[0], axis), dot(pts[0], axis));
            float2 otherProj = float2(dot(otherPts[0], axis), dot(otherPts[0], axis));

            for (int j = 1; j < 4; j++)
            {
                proj.x = min(proj.x, dot(pts[j], axis));
                proj.y = max(proj.y, dot(pts[j], axis));

                otherProj.x = min(otherProj.x, dot(otherPts[j], axis));
                otherProj.y = max(otherProj.y, dot(otherPts[j], axis));
            }

            if (proj.y < otherProj.x || otherProj.y < proj.x)
                return true;
        }

        return false;
    }

    // In Slang, constructors are defined through special methods named `__init`.
    // Several constructors can be defined, and overload resolution will pick the right one.
    //
    __init(float2 center, float2x2 rotation, float2 scale)
    {
        this.center = center;
        this.rotation = rotation;
        this.scale = scale;
    }
};

/*
* smoothStep maps a value from the range [minval, maxval] to the range [0, 1] using a smooth function.

* The Hermite interpolation function makes sure the derivative is 0 at the ends of the range. 
* This is helpful for representing optimizable parameters since it prevents the parameters from exceeding 
* the valid range and losing gradients.
* 
* Note that this function is marked `[Differentiable]`, which allows it to be used in other differentiable functions
* and will be differentiated automatically by the compiler whenever necessary.
*/
[Differentiable]
vector<float, N> smoothStep<let N : int>(vector<float, N> x, vector<float, N> minval, vector<float, N> maxval)
{
    vector<float, N> y = clamp((x - minval) / (maxval - minval), 0.f, 1.f);
    return y * y * (3.f - 2.f * y);
}

// Scalar variant of the above function.
[Differentiable]
float smoothStep(float x, float minval, float maxval)
{
    float y = clamp((x - minval) / (maxval - minval), 0.f, 1.f);
    return y * y * (3.f - 2.f * y);
}

/*
* A utility function to premultiply the color by the alpha value. 
* This is a key part of the alpha blending routine used in the 
* Gaussian splatting algorithm.
*/
[Differentiable]
float4 preMult(float4 pixel)
{
    return float4(pixel.rgb * pixel.a, pixel.a);
}

/*
* alphaBlend() implements the standard alpha blending algorithm.
* 
* Takes the current pixel value 'pixel' & blends it with a 
* contribution 'gval' from a new Gaussian.
*/
[Differentiable]
float4 alphaBlend(float4 pixel, float4 gval)
{
    gval = preMult(gval);

    return float4(
        pixel.rgb + gval.rgb * pixel.a,
        pixel.a * (1 - gval.a));
}

/*
* undoAlphaBlend() implements the reverse of the alpha blending algorithm.
* 
* Takes a pixel value 'pixel' and the same 'gval' contribution & 
* computes the previous pixel value.
* 
* This is a critical piece of the backwards pass.
*/
float4 undoAlphaBlend(float4 pixel, float4 gval)
{
    gval = preMult(gval);

    var oldPixelAlpha = pixel.a / (1 - gval.a);
    return float4(
        pixel.rgb - gval.rgb * oldPixelAlpha,
        oldPixelAlpha);
}

/*
* PixelState encapsulates all the info for a pixel as it is being rasterized
* through the sorted list of blobs.
*/
struct PixelState : IDifferentiable
{
    float4 value;
    uint finalCount;
};

/* 
* transformPixelState() applies the alpha blending operation to the pixel state & 
* updates the counter accordingly. 
* 
* This state transition also stops further blending once the pixel is effectively opaque. 
* This is important to avoid the alpha becoming too low (or even 0), at which point
* the blending is not reversible.
*
*/
[Differentiable]
PixelState transformPixelState(PixelState pixel, float4 gval)
{
    var newState = alphaBlend(pixel.value, gval);

    if (pixel.value.a < 1.f / 255.f)
        return { pixel.value, pixel.finalCount };

    return { newState, pixel.finalCount + 1 };
}

/* 
* undoPixelState() reverses the alpha blending operation and restores the previous pixel 
* state.
*/
PixelState undoPixelState(PixelState nextState, uint index, float4 gval)
{
    if (index > nextState.finalCount)
        return { nextState.value, nextState.finalCount };
    
    return { undoAlphaBlend(nextState.value, gval), nextState.finalCount - 1 };
}

/*
* loadFloat() is a helper method that loads a float from the buffer in a *differentiable* manner.
*
* The function itself is fairly straightforward, but the key part is the `[BAckwardDerivative]` attribute.
*
* loadFloat_bwd() is the corresponding user-defined backwards function that is responsible for writing
* back the gradient associated with the loaded float.
*
* Using the [BackwardDerivative] attributes instructs the auto-diff pass to call the provided function to
* backpropagate the gradient (rather than trying to differentiate the function body automatically).
*
* This system is the primary approach to dealing with memory loads & stores since there are many approaches to
* accumulating gradients of memory accesses.
*/
[BackwardDerivative(loadFloat_bwd)]
float loadFloat(uint idx, uint localDispatchIdx)
{
    return blobsBuffer[idx];
}

/* 
* loadFloat_bwd() is the user-defined derivative for loadFloat()
*
* Since loadFloat() is always used to load values that are uniform across the workgroup,
* the differentials must be accumulated before writing back, since each thread will 
* have a different derivative value.
* 
* The function uses a workgroup-level binary reduction to add up the gradients across the workgroup.
* Then the first thread in the workgroup atomically accumulates the gradient to the global derivative buffer.
*/
void loadFloat_bwd(uint idx, uint localDispatchIdx, float dOut)
{
    // Clamp the gradients to avoid any weird problems with the optimization.
    if (abs(dOut) < 10.f)
        reductionBuffer[localDispatchIdx] = dOut;
    else
        reductionBuffer[localDispatchIdx] = 10.f * sign(dOut);
    
    GroupMemoryBarrierWithGroupSync();
    
    // Binary reduction
    for (uint stride = (WG_X * WG_Y) / 2; stride > 0; stride /= 2)
    {
        if (localDispatchIdx < stride)
            reductionBuffer[localDispatchIdx] += reductionBuffer[localDispatchIdx + stride];

        GroupMemoryBarrierWithGroupSync();
    }

    if (localDispatchIdx == 0)
        atomicAccumulate(reductionBuffer[0], idx);
}

/*
* atomicAccumulate() is a helper method that atomically accumulates a float value to the global derivative buffer.
*
* Unfortunately, WGSL does not have floating-point atomics, so this method uses a compare-and-swap (i.e. compareExchange()) loop to perform
* this operation. This is a common pattern for implementing floating-point atomics on platforms that do not support them.
*
* This function makes use of 'bitcasting' which is a way of reinterpreting the bits of one type as another type. Note that this is
* different from type-casting, which changes the value of the data. 
* In Slang, this can be done via type-specific methods such as `asfloat()` or `asuint()` or more generally via `bit_cast<T, U>()`
*
*/
void atomicAccumulate(float val, uint idx)
{
    // No need to accumulate zeros.
    if (val == 0.f)
        return; 

    // Loop for as long as the compareExchange() fails, which means another thread 
    // is trying to write to the same location.
    //
    for (;;)
    {
        uint oldInt = derivBuffer[idx].load();
        float oldFloat = asfloat(oldInt);

        float newFloat = oldFloat + val;

        uint newInt = asuint(newFloat);

        // compareExchange() returns the value at the location before the operation.
        // If it's changed, we have contention between threads & need to try again.
        //
        if (derivBuffer[idx].compareExchange(oldInt, newInt) == oldInt)
            break;
    }
}

[Differentiable]
float2x2 inverse(float2x2 mat)
{
    float2x2 output;

    float det = determinant(mat);
    output[0][0] = mat[1][1] / det;
    output[0][1] = -mat[0][1] / det;
    output[1][0] = -mat[1][0] / det;
    output[1][1] = mat[0][0] / det;

    return output;
}

struct Gaussian2D : IDifferentiable
{
    float2 center;
    float2x2 sigma;
    float3 color;
    float opacity;

    [Differentiable]
    static Gaussian2D load(uint idx, uint localIdx)
    {
        uint total = Gaussian2D.count();
        Gaussian2D gaussian;

        gaussian.center = smoothStep(
            float2(
                loadFloat(total * 0 + idx, localIdx),
                loadFloat(total * 1 + idx, localIdx)),
            float2(0, 0),
            float2(1, 1));
        
        // Add a small padding value to avoid singularities or unstable Gaussians.
        gaussian.sigma[0][0] = smoothStep(
            loadFloat(total * 2 + idx, localIdx) * 0.8f, 0.f, 1.f) + 0.005f; 
        gaussian.sigma[1][1] = smoothStep(
            loadFloat(total * 3 + idx, localIdx) * 0.8f, 0.f, 1.f) + 0.005f; 

        float aniso = (smoothStep(
            loadFloat(total * 4 + idx, localIdx) * 0.6f, 0.f, 1.f) - 0.5f) * 1.65f;
        
        gaussian.sigma[0][1] = sqrt(gaussian.sigma[0][0] * gaussian.sigma[1][1]) * aniso;
        gaussian.sigma[1][0] = sqrt(gaussian.sigma[0][0] * gaussian.sigma[1][1]) * aniso;

        
        gaussian.color = smoothStep(
            float3(
                loadFloat(total * 5 + idx, localIdx) * 0.8f,
                loadFloat(total * 6 + idx, localIdx) * 0.8f,
                loadFloat(total * 7 + idx, localIdx) * 0.8f),
            float3(0, 0, 0),
            float3(1, 1, 1));

        gaussian.opacity = smoothStep(
            loadFloat(total * 8 + idx, localIdx) * 0.9f + 0.1f, 0, 1);

        // Scale the sigma so the blobs aren't too large
        gaussian.sigma *= 0.0001;

        return gaussian;
    }

    // Simple helper method to get the number of elements in the buffer
    static uint count()
    {
        uint elementCount = (uint)BLOB_BUFFER_SIZE;
        return elementCount / NUM_FIELDS;
    }

    /*
    * eval() calculates the color and weight of the Gaussian at a given UV coordinate.
    * 
    * This method calculates an alpha by applying the standard multi-variate Gaussian formula 
    * to calculate the power which is then scaled by an opacity value. The color components 
    * are represented by additional fields.
    */
    [Differentiable]
    float4 eval(float2 uv)
    {
        float2x2 invCov = inverse(sigma);
        float2 diff = uv - center;
        float power = -0.5f * ((diff.x * diff.x * invCov[0][0]) +
                            (diff.y * diff.y * invCov[1][1]) +
                            (diff.x * diff.y * invCov[0][1]) +
                            (diff.y * diff.x * invCov[1][0]));
        
        float weight = min(.99f, opacity * exp(power));
        return float4(color, weight);
    }

    OBB bounds()
    {
        // Calculate eigenvectors for the 2x2 matrix.
        float2x2 cov = sigma;

        float a = cov[0][0];
        float b = cov[0][1];
        float c = cov[1][0];
        float d = cov[1][1];

        float n_stddev = 4.f;

        if (abs(b) < 1e-6 || abs(c) < 1e-6)
        {
            // The covariance matrix is diagonal (or close enough..), so the eigenvectors are the x and y axes.
            float2x2 eigenvectors = float2x2(float2(1, 0), float2(0, 1));
            float2 scale = float2(sqrt(a), sqrt(d));

            return OBB(center, eigenvectors, scale * n_stddev);
        }
        else
        {
            float trace = a + d;
            float det = a * d - b * c;

            float lambda1 = 0.5 * (trace + sqrt(trace * trace - 4 * det));
            float lambda2 = 0.5 * (trace - sqrt(trace * trace - 4 * det));

            float2x2 eigenvectors;
            eigenvectors[0] = float2(lambda1 - d, c) / length(float2(lambda1 - d, c));
            eigenvectors[1] = float2(b, lambda2 - a) / length(float2(b, lambda2 - a));

            // Calculate the scale of the OBB
            float2 scale = float2(sqrt(lambda1), sqrt(lambda2));

            return OBB(center, eigenvectors, scale * n_stddev);
        }
    }
};

/*
* padBuffer() is a helper method that fills the unused space in the buffer with a sentinel value (uint::maxValue).
* This is just because bitonicSort requires all elements to have a valid value. padBuffer filles these in with
* maxValue, which are effectively pushed to the end of the list.
*/
PaddedShortList padBuffer(FilledShortList, uint localIdx)
{
    GroupMemoryBarrierWithGroupSync();

    var maxN = blobCount;
    for (uint i = localIdx; i < GAUSSIANS_PER_BLOCK; i += (WG_X * WG_Y))
    {
        if (i >= maxN)
            blobs[i] = uint::maxValue;
    }

    return { 0 };
}

/*
* bitonicSort() implements a workgroup-level parallel sorting algorithm to sort indices in the short-list.
* Requires all elements in the buffer to be valid (invalid elements should be set to infinity, or its equivalent).
*
* Bitonic sorting is an efficient, deterministic, parallel sorting algorithm particularly well-suited for GPUs. 
* At a high-level, it operates by comparing & swapping elements in parallel in (logN)^2 stages.
*
* More info on the bitonic sort algorithm: https://en.wikipedia.org/wiki/Bitonic_sorter
* The code was adapted from the Wikipedia sample pseudocode here: https://en.wikipedia.org/wiki/Bitonic_sorter#Example_code
* 
*/
SortedShortList bitonicSort(PaddedShortList, uint localIdx)
{
    GroupMemoryBarrierWithGroupSync();

    uint maxN = blobCount;
    for (uint k = 2; k <= GAUSSIANS_PER_BLOCK; k *= 2)
    {
        for (uint j = k / 2; j > 0; j /= 2)
        {
            for (uint i = localIdx; i < GAUSSIANS_PER_BLOCK; i += WG_X * WG_Y)
            {
                uint l = i ^ j;
                if (l > i)
                {
                    if ((((i & k) == 0) && (blobs[i] > blobs[l])) ||
                        (((i & k) != 0) && (blobs[i] < blobs[l])))
                    {
                        // Swap
                        var temp = blobs[i];
                        blobs[i] = blobs[l];
                        blobs[l] = temp;
                    }
                }
            }

            GroupMemoryBarrierWithGroupSync();
        }
    }

    return { 0 };
}

/*
* coarseRasterize() calculates a subset of blobs that intersect with the current tile. Expects the blob counters to be reset before calling.
*
* The coarse rasterization step determines a subset of blobs that intersect with the tile.
* Each thread in the workgroup takes a subset of blobs and uses bounding-box intersection tests to determine
* if the tile associated with this workgroup overlaps with the blob's bounds.
* 
* Note: This is a simplistic implementation, so there is a limit to the number of blobs in the short-list (NUM_GAUSSIANS_PER_BLOCK).
* In practice, if the number of blobs per tile exceeds this, NUM_GAUSSIANS_PER_BLOCK must be increased manually. 
* A more sophisticated implementation would perform multiple passes to handle this case.
*
*/
FilledShortList coarseRasterize(InitializedShortList sList, OBB tileBounds, uint localIdx)
{
    GroupMemoryBarrierWithGroupSync();

    Gaussian2D gaussian;
    uint numGaussians = Gaussian2D.count();
    for (uint i = localIdx; i < numGaussians; i += (WG_X * WG_Y))
    {
        gaussian = Gaussian2D.load(i, localIdx);
        OBB bounds = gaussian.bounds();
        if (bounds.intersects(tileBounds))
        {
            blobs[blobCountAT++] = i;
        }
    }

    GroupMemoryBarrierWithGroupSync();

    blobCount = blobCountAT.load();

    return { 0 };
}

[Differentiable]
float4 eval(uint blob_id, no_diff float2 uv, uint localIdx)
{
    Gaussian2D gaussian = Gaussian2D.load(blob_id, localIdx);
    return gaussian.eval(uv);
}

/* 
* fineRasterize() produces the per-pixel final color from a sorted list of blobs that overlap the current tile.
*
* The fine rasterizeration is where the calculation of the per-pixel color happens. 
* This uses the multiplicative alpha blending algorithm laid out in the original GS paper (https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/)
* This is represented as a 'state transition' (transformPixelState) as we go through the blobs in order, so that we can 
* concisely represent the 'state undo' operation in the backwards pass.
* 
* In Slang, custom derivative functions can be defiened using the `[BackwardDerivative(custom_fn)]` attribute.
*/
[BackwardDerivative(fineRasterize_bwd)]
float4 fineRasterize(SortedShortList, uint localIdx, no_diff float2 uv)
{
    GroupMemoryBarrierWithGroupSync();

    PixelState pixelState = PixelState(float4(0, 0, 0, 1), 0);
    uint count = blobCount;
    // The forward rasterization 
    for (uint i = 0; i < count; i++)
        pixelState = transformPixelState(pixelState, eval(blobs[i], uv, localIdx));

    maxCount[localIdx] = pixelState.finalCount;
    finalVal[localIdx] = pixelState.value;
    return pixelState.value;
}

/*
* fineRasterize_bwd() is the user-provided backwards pass for the fine rasterization step.
* 
* This is implemented as a custom derivative function because, while applying auto-diff directly to a function
* with a loop can result in excessive state caching (a necessary part of standard automatic differentiation methods)
*
* For Gaussian splatting, there is a 'state undo' (undoPixelState) operation available. fineRasterize_bwd takes advantage of this 
* to recreate the states at each step of the forward pass instead of letting auto-diff store them.
* 
* While it is important to represent the backwards loop explicitly in this way, the contents of the loop body (loading, evaluation, 
* blending, etc..) can still be differentiated automatically (and it would be tedioush to do so manually). 
*
* The loop body therefore invokes `bwd_diff` to backprop the derivatives via auto-diff.
*/
void fineRasterize_bwd(SortedShortList, uint localIdx, float2 uv, float4 dOut)
{
    GroupMemoryBarrierWithGroupSync();

    PixelState pixelState = { finalVal[localIdx], maxCount[localIdx] };

    PixelState.Differential dColor = { dOut };
    
    // `workgroupUniformLoad` is a WGSL-specific intrinsic that marks a load as uniform across the workgroup.
    // This is necessary to prevent errors from uniformity analysis.
    //
    uint count = workgroupUniformLoad(blobCount);

    // The backwards pass manually performs an 'undo' to reproduce the state at each step.
    // The inner loop body still uses auto-diff, so the bulk of the computation is still
    // handled by the auto-diff engine.
    //
    for (uint _i = count; _i > 0; _i--)
    {
        uint i = _i - 1;
        var blobID = blobs[i];
        var gval = eval(blobID, uv, localIdx);
        var prevState = undoPixelState(pixelState, i+1, gval);

        var dpState = diffPair(prevState);
        var dpGVal = diffPair(gval);
        
        // Once we have the previous state, we can continue with the backpropagation via auto-diff within
        // the loop body. Note that the `bwd_diff` calls writeback the differentials to dpState and dpGVal,
        // and can be obtained via `getDifferential()` (or simply '.d')
        // 
        bwd_diff(transformPixelState)(dpState, dpGVal, dColor);
        bwd_diff(eval)(blobID, uv, localIdx, dpGVal.getDifferential());

        pixelState = prevState;
        dColor = dpState.getDifferential();
    }
}

InitializedShortList initShortList(uint2 dispatchThreadID)
{
    GroupMemoryBarrierWithGroupSync();

    if (dispatchThreadID.x % WG_X == 0 && dispatchThreadID.y % WG_Y == 0)
    {
        blobCount = 0; blobCountAT = 0;
    }

    return { 0 };
}

/* 
* calcUV() computes a 'stretch-free' mapping from the requested render-target dimensions (renderSize) to the
* image in the texture (imageSize)
*/
float2 calcUV(uint2 dispatchThreadID, int2 renderSize, int2 imageSize)
{
    // Easy case.
    if (all(renderSize == imageSize))
        return ((float2)dispatchThreadID) / renderSize;
    
    float aspectRatioRT = ((float) renderSize.x) / renderSize.y;
    float aspectRatioTEX = ((float) imageSize.x) / imageSize.y;

    if (aspectRatioRT > aspectRatioTEX)
    {
        // Render target is wider than the texture. 
        // Match the widths.
        //
        float xCoord = ((float) dispatchThreadID.x) / renderSize.x;
        float yCoord = ((float) dispatchThreadID.y * aspectRatioTEX) / renderSize.x;

        // We'll re-center the y-coord around 0.5.
        float yCoordMax = aspectRatioTEX / aspectRatioRT;
        yCoord = yCoord + (1.0 - yCoordMax) / 2.0f;
        return float2(xCoord, yCoord);
    }
    else
    {
        // Render target is taller than the texture. 
        // Match the heights.
        //
        float yCoord = ((float) dispatchThreadID.y) / renderSize.y;
        float xCoord = ((float) dispatchThreadID.x) / (renderSize.y * aspectRatioTEX);

        // We'll recenter the x-coord around 0.5.
        float xCoordMax = aspectRatioRT / aspectRatioTEX;
        xCoord = xCoord + (1.0 - xCoordMax) / 2.0f;
        return float2(xCoord, yCoord);
    }
}

/* 
* splatBlobs() is the main rendering routine that computes a final color for the pixel.
* 
* It proceeds in 4 stages: 
*  1. Coarse rasterization: Short-list blobs that intersect with the current tile through 
*                           bounding-box intersection tests.
*  2. Padding: Fill the unused space in the buffer with a sentinel value.
*  3. Sorting: Sort the short list of blobs.
*  4. Fine rasterization: Calculate the final color for the pixel.
* 
* Note that only the final stage is differentiable since it is the only stage that produces 
* the final color. 
* The other stages are just optimizations to reduce the blobs under consideration.
*
* The produced derivative function will re-use the same optimizations as-is.
* 
*/
[Differentiable]
float4 splatBlobs(uint2 dispatchThreadID, int2 dispatchSize)
{
    uint globalID = dispatchThreadID.x + dispatchThreadID.y * dispatchSize.x;
    
    int texWidth;
    int texHeight;
    targetTexture.GetDimensions(texWidth, texHeight);
    int2 texSize = int2(texWidth, texHeight);

    // Calculate effective uv coordinate for the current pixel. This is used for 
    // evaluating the 2D Daussians.
    float2 uv = calcUV(dispatchThreadID, dispatchSize, texSize);
    
    //
    // Calculate a bounding box in uv coordinates for the current workgroup.
    //

    uint2 tileCoords = uint2(dispatchThreadID.x / WG_X, dispatchThreadID.y / WG_Y);

    float2 tileLow = calcUV(tileCoords * uint2(WG_X, WG_Y), dispatchSize, texSize);
    float2 tileHigh = calcUV((tileCoords + 1) * uint2(WG_X, WG_Y), dispatchSize, texSize);

    float2 tileCenter = (tileLow + tileHigh) / 2;
    float2x2 tileRotation = float2x2(1, 0, 0, 1);
    float2 tileScale = (tileHigh - tileLow) / 2;

    OBB tileBounds = OBB(tileCenter, tileRotation, tileScale);
    
    // -------------------------------------------------------------------

    // Main rendering steps..

    // Initialize the short list (by resetting counters)
    InitializedShortList sList = initShortList(dispatchThreadID);

    uint2 localID = dispatchThreadID % uint2(WG_X, WG_Y);
    uint localIdx = localID.x + localID.y * WG_X;

    // Short-list blobs that overlap with the local tile.
    FilledShortList filledSList = coarseRasterize(sList, tileBounds, localIdx);

    // Pad the unused space in the buffer
    PaddedShortList paddedSList = padBuffer(filledSList, localIdx);

    // Sort the short list
    SortedShortList sortedList = bitonicSort(paddedSList, localIdx);

    // Perform per-pixel fine rasterization
    float4 color = fineRasterize(sortedList, localIdx, uv);

    // Blend with background
    return float4(color.rgb * (1.0 - color.a) + color.a, 1.0);
}

/*
* loss() implements the standard L2 loss function to quantify the difference between 
* the rendered image and the target texture.
*/
[Differentiable]
float loss(uint2 dispatchThreadID, int2 imageSize)
{
    // Splat the blobs and calculate the color for this pixel.
    float4 color = splatBlobs(dispatchThreadID, imageSize);

    float4 targetColor;
    float weight;
    if (dispatchThreadID.x >= imageSize.x || dispatchThreadID.y >= imageSize.y)
    {
        return 0.f;
    }
    else
    {
        uint2 flippedCoords = uint2(dispatchThreadID.x, imageSize.y - dispatchThreadID.y);
        targetColor = no_diff targetTexture[flippedCoords];
        return dot(color.rgb - targetColor.rgb, color.rgb - targetColor.rgb);
    }

    return 0.f;
}

// Sequence of additional kernel calls to be performed before imageMain
// By default, imageMain is always the last kernel to be dispatched
//
// playground::CALL is a slang-playground directive that helps us queue up additional kernels.
// Note that if this sample code is outside the playground, your engine is responsible for
// dispatching these kernels in this order.
//

/*
* clearDerivativesMain() is a kernel that resets the derivative buffer to all 0s
*/
[playground::CALL(BLOB_BUFFER_SIZE, 1, 1)]
[shader("compute")]
[numthreads(64, 1, 1)]
void clearDerivativesMain(uint2 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= BLOB_BUFFER_SIZE)
        return;
    
    derivBuffer[dispatchThreadID.x].store(asuint(0.f));
}

/*
* computeDerivativesMain() is a kernel that computes the derivatives of the loss function with respect to the blobs.
* 
* It uses Slang's auto-diff capabilities by simply calling `bwd_diff()` on the loss function to generate a new function
* that is the derivative of the loss function.
*/
[playground::CALL::SIZE_OF("targetTexture")]
[shader("compute")]
[numthreads(16, 16, 1)]
void computeDerivativesMain(uint2 dispatchThreadID : SV_DispatchThreadID)
{
    uint dimX;
    uint dimY;
    targetTexture.GetDimensions(dimX, dimY);

    int2 targetImageSize = int2(dimX, dimY);

    // Distribute the 1.f total weight across all pixels
    float perPixelWeight = 1.f / (targetImageSize.x * targetImageSize.y);

    // Backprop (will write derivatives to the derivBuffer)
    bwd_diff(loss)(dispatchThreadID, targetImageSize, perPixelWeight);
}

/* 
* updateBlobsMain() is a kernel that updates the blob parameters using the Adam optimizer.
* 
* Since all the parameters are laid out in a single float buffer, there is no need to re-interpret 
* the buffer into a struct.
* 
* The Adam optimization method (https://arxiv.org/abs/1412.6980) is used to process the gradients before
* applying the update. It acts as a temporal filter on the gradients, and stores per-parameter state that
* persists across iterations to help stabilize the optimization process.
*
*/
[playground::CALL(BLOB_BUFFER_SIZE, 1, 1)]
[shader("compute")]
[numthreads(256, 1, 1)]
void updateBlobsMain(uint2 dispatchThreadID: SV_DispatchThreadID)
{
    var globalID = dispatchThreadID.x;
    if (globalID >= BLOB_BUFFER_SIZE)
        return;

    // Read & reset the derivative
    float g_t = asfloat(derivBuffer[globalID].load());
    derivBuffer[globalID] = asuint(0.f);

    float g_t_2 = g_t * g_t;

    // 
    // Perform a gradient update using Adam optimizer rules for
    // a smoother optimization.
    // 

    float m_t_prev = adamFirstMoment[globalID];
    float v_t_prev = adamSecondMoment[globalID];
    float m_t = ADAM_BETA_1 * m_t_prev + (1 - ADAM_BETA_1) * g_t;
    float v_t = ADAM_BETA_2 * v_t_prev + (1 - ADAM_BETA_2) * g_t_2;

    adamFirstMoment[globalID] = m_t;
    adamSecondMoment[globalID] = v_t;

    float m_t_hat = m_t / (1 - ADAM_BETA_1);
    float v_t_hat = v_t / (1 - ADAM_BETA_2);

    float update = (ADAM_ETA / (sqrt(v_t_hat) + ADAM_EPSILON)) * m_t_hat;

    blobsBuffer[globalID] -= update;
}

float4 imageMain(uint2 dispatchThreadID, uint2 screenSize)
{
    return splatBlobs(dispatchThreadID, screenSize);
}