// signal_smooth.slang
//
// Compute pass: Gaussian smoothing of a 1-D measurement buffer.
//
// Each thread is responsible for one output sample.  It reads a neighbourhood
// of radius kRadius around its position, weights the samples with a binomial-
// coefficient kernel, and writes the result to the output buffer.
//
// Dispatch: (kCount, 1, 1) threads — one per sample.

// ---- constants --------------------------------------------------------------

static const int   kCount       = 16;          // number of samples
static const int   kRadius      = 2;           // kernel half-width
static const float kTotalWeight = 16.0f;       // sum of kWeights

// Binomial-coefficient weights for offsets -2 .. +2  (Pascal's triangle row 4)
static const float kWeights[2 * kRadius + 1] = {
    1.0f, 4.0f, 6.0f, 4.0f, 1.0f
};

// ---- bindings ---------------------------------------------------------------

RWStructuredBuffer<float> g_signal;    // [set 0, binding 0]  raw input
RWStructuredBuffer<float> g_smoothed;  // [set 0, binding 1]  smoothed output

// ---- helpers ----------------------------------------------------------------

// Return the input sample at `pos`, clamping to the nearest valid index so
// that threads near the buffer edges see a repeated edge value rather than
// out-of-bounds garbage.
float fetch_clamped(int pos)
{
    int safe = clamp(pos, 0, kCount);   // BUG: upper bound should be kCount - 1
    return g_signal[safe];
}

// Evaluate the Gaussian kernel centred on `centre` and return the weighted sum.
float apply_kernel(int centre)
{
    float acc = 0.0f;
    for (int k = -kRadius; k <= kRadius; k++)
        acc += fetch_clamped(centre + k) * kWeights[k + kRadius];
    return acc / kTotalWeight;
}

// ---- entry point ------------------------------------------------------------

[shader("compute")]
[numthreads(kCount, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int i = (int)tid.x;
    g_smoothed[i] = apply_kernel(i);
}
