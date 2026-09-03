// spectrum_features.slang
//
// Compute shader: per-frame spectral feature extraction.
//
// The shader reads a 1-D time-domain signal from a structured buffer and
// processes it frame-by-frame.  Each compute thread is responsible for one
// frame of kFrameLen samples.  Within a frame the pipeline is:
//
//   1.  Load the raw samples.
//   2.  Remove the DC offset (subtract the frame mean).
//   3.  Apply a Hann window to reduce spectral leakage.
//   4.  Compute the DFT magnitude spectrum (DC through Nyquist inclusive).
//   5.  Apply spectral whitening (divide each bin by a local smoothed average).
//   6.  Locate the dominant spectral peak.
//   7.  Refine the peak position to sub-bin precision via quadratic interpolation.
//   8.  Derive secondary features: RMS energy, spectral centroid, bandwidth,
//       zero-crossing rate, and spectral flatness.
//   9.  Write all features to the output buffer.
//
// Thread dispatch: (kFrameCount, 1, 1) — one thread per frame.
//
// Buffer layout
//   set 0, binding 0  g_signal   : float[kSignalLen]   input time-domain signal
//   set 0, binding 1  g_features : float[kFrameCount * kFeaturesPerFrame]
//                                  output feature matrix, row-major

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static const int   kFrameLen        = 32;
static const int   kFrameCount      = 16;
static const int   kSignalLen       = kFrameLen * kFrameCount;   // 512

// DFT: for a real-valued frame of length N we compute N/2+1 unique bins
// (DC at index 0 through Nyquist at index N/2).
static const int   kDFTBins         = kFrameLen / 2 + 1;        // 17

// Spectral whitening window half-width (bins).
static const int   kWhitenRadius    = 2;
static const float kWhitenFloor     = 1e-4f;

// Number of features written per frame.
static const int   kFeaturesPerFrame = 10;
// Feature indices within each frame row:
//   0  refined_peak_bin   sub-bin dominant frequency (bin units)
//   1  peak_magnitude     whitened magnitude of the dominant bin
//   2  peak_width         half-power width of the dominant spectral peak (bins)
//   3  spectral_balance   fraction of energy below the dominant bin
//   4  rms_energy         root-mean-square of the windowed frame
//   5  spectral_centroid  amplitude-weighted mean frequency (bin units)
//   6  spectral_bandwidth amplitude-weighted spread around the centroid
//   7  spectral_flatness  geometric-to-arithmetic mean ratio of the spectrum
//   8  zero_crossing_rate fraction of samples with a sign change from the previous
//   9  spectral_rolloff   bin at which 85% of the total spectral energy is reached

static const float kPI              = 3.14159265358979f;
static const float kPreemphAlpha    = 0.95f;
static const float kQuadEps         = 1e-7f;
static const float kRolloffThresh   = 0.85f;

// ---------------------------------------------------------------------------
// Descriptor bindings
// ---------------------------------------------------------------------------

RWStructuredBuffer<float> g_signal;    // [set 0, binding 0]
RWStructuredBuffer<float> g_features;  // [set 0, binding 1]

// ---------------------------------------------------------------------------
// Stage 1 – frame loading
// ---------------------------------------------------------------------------

void load_frame(int frame_id, out float s[kFrameLen])
{
    int base = frame_id * kFrameLen;
    for (int i = 0; i < kFrameLen; ++i)
        s[i] = g_signal[base + i];
}

// ---------------------------------------------------------------------------
// Stage 2 – DC removal
// ---------------------------------------------------------------------------

void remove_dc(inout float s[kFrameLen])
{
    float sum = 0.0f;
    for (int i = 0; i < kFrameLen; ++i)
        sum += s[i];
    float mean = sum / float(kFrameLen);
    for (int i = 0; i < kFrameLen; ++i)
        s[i] -= mean;
}

// ---------------------------------------------------------------------------
// Stage 2b – first-order preemphasis (optional high-frequency boost)
// ---------------------------------------------------------------------------

void apply_preemphasis(inout float s[kFrameLen])
{
    // Process back-to-front so we don't clobber the previous sample in-place.
    for (int i = kFrameLen - 1; i >= 1; --i)
        s[i] -= kPreemphAlpha * s[i - 1];
    // s[0] left unchanged.
}

// ---------------------------------------------------------------------------
// Stage 3 – Hann windowing
// ---------------------------------------------------------------------------

float hann_coeff(int n, int N)
{
    // w[n] = 0.5 * (1 - cos(2*pi*n / (N - 1)))
    return 0.5f * (1.0f - cos(2.0f * kPI * float(n) / float(N - 1)));
}

void apply_hann_window(inout float s[kFrameLen])
{
    for (int n = 0; n < kFrameLen; ++n)
        s[n] *= hann_coeff(n, kFrameLen);
}

// ---------------------------------------------------------------------------
// Stage 4 – DFT magnitude spectrum
// ---------------------------------------------------------------------------
//
// Computes |X[k]| for k = 0 .. kDFTBins-1 using the definition:
//
//   X[k] = sum_{n=0}^{N-1} x[n] * e^{-j 2pi k n / N}
//
// For real-valued x the spectrum is conjugate-symmetric so only the first
// N/2+1 bins are unique.

void compute_dft(float s[kFrameLen], out float mag[kDFTBins])
{
    float two_pi_over_N = 2.0f * kPI / float(kFrameLen);
    for (int k = 0; k < kDFTBins; ++k) {
        float re = 0.0f;
        float im = 0.0f;
        for (int n = 0; n < kFrameLen; ++n) {
            float phase = two_pi_over_N * float(k) * float(n);
            re += s[n] * cos(phase);
            im -= s[n] * sin(phase);
        }
        mag[k] = sqrt(re * re + im * im);
    }
}

// ---------------------------------------------------------------------------
// Stage 5 – spectral whitening
// ---------------------------------------------------------------------------
//
// Divides each bin by a locally-smoothed version of the spectrum so that
// narrow peaks stand out regardless of the overall spectral envelope.

void compute_whitening_envelope(float mag[kDFTBins], out float env[kDFTBins])
{
    for (int k = 0; k < kDFTBins; ++k) {
        float acc = 0.0f;
        int   cnt = 0;
        int   lo  = max(k - kWhitenRadius, 0);
        int   hi  = min(k + kWhitenRadius, kDFTBins - 1);
        for (int j = lo; j <= hi; ++j) {
            acc += mag[j];
            ++cnt;
        }
        env[k] = acc / float(cnt);
    }
}

void apply_whitening(float mag[kDFTBins], float env[kDFTBins],
                     out float white[kDFTBins])
{
    for (int k = 0; k < kDFTBins; ++k)
        white[k] = mag[k] / (env[k] + kWhitenFloor);
}

// ---------------------------------------------------------------------------
// Stage 6 – dominant peak detection
// ---------------------------------------------------------------------------

int find_peak_bin(float white[kDFTBins])
{
    int   peak_bin = 0;
    float peak_val = white[0];
    for (int k = 1; k < kDFTBins; ++k) {
        if (white[k] > peak_val) {
            peak_val = white[k];
            peak_bin = k;
        }
    }
    return peak_bin;
}

// ---------------------------------------------------------------------------
// Stage 7 – sub-bin quadratic interpolation
// ---------------------------------------------------------------------------
//
// Given the whitened spectrum and the index of the dominant bin, fits a
// parabola through the bin and its immediate left and right neighbours to
// estimate the true peak location with sub-bin precision.
//
// Standard formula (Smith, "Spectral Audio Signal Processing", 2011):
//
//   delta = 0.5 * (y[-1] - y[+1]) / (y[-1] - 2*y[0] + y[+1])
//
// Returns the refined peak position in bin units.

[noinline]
float refine_peak_position(float white[kDFTBins], int peak_bin)
{
    // Guard against the left boundary: bin 0 has no left neighbour.
    if (peak_bin == 0)
        return 0.0f;

    float ym1   = white[peak_bin - 1];
    float y0    = white[peak_bin    ];
    float y1    = white[peak_bin + 1];

    float denom = ym1 - 2.0f * y0 + y1;
    if (abs(denom) < kQuadEps)
        return float(peak_bin);

    float delta = 0.5f * (ym1 - y1) / denom;
    return float(peak_bin) + clamp(delta, -0.5f, 0.5f);
}

// ---------------------------------------------------------------------------
// Stage 7b – spectral peak width estimation
// ---------------------------------------------------------------------------
//
// Measures how many bins around the dominant peak remain above a fraction of
// the peak's whitened magnitude.  A narrow peak indicates a clean tonal signal;
// a wide peak indicates a broad spectral lobe or noisy excitation.

[noinline]
float estimate_peak_width(float white[kDFTBins], int peak_bin, float threshold_ratio)
{
    float threshold = white[peak_bin] * threshold_ratio;
    if (threshold < kWhitenFloor)
        return 0.0f;

    // Walk left from peak until we drop below threshold.
    int left = peak_bin;
    while (left > 0 && white[left - 1] >= threshold)
        --left;

    // Walk right from peak until we drop below threshold.
    int right = peak_bin;
    while (right < kDFTBins - 1 && white[right + 1] >= threshold)
        ++right;

    return float(right - left);
}

// ---------------------------------------------------------------------------
// Stage 7c – inter-bin energy ratio
// ---------------------------------------------------------------------------
//
// Computes the ratio of spectral energy below the peak bin versus above it.
// A ratio near 1 indicates a balanced spectrum; a high value indicates that
// most energy is in the low-frequency bins; a low value indicates dominance
// of high frequencies.

float compute_spectral_balance(float mag[kDFTBins], int split_bin)
{
    float low_energy  = 0.0f;
    float high_energy = 0.0f;

    for (int k = 0; k < kDFTBins; ++k) {
        float pw = mag[k] * mag[k];
        if (k < split_bin)
            low_energy  += pw;
        else if (k > split_bin)
            high_energy += pw;
    }

    float denom = low_energy + high_energy + kWhitenFloor * kWhitenFloor;
    return low_energy / denom;
}

// ---------------------------------------------------------------------------
// Secondary features
// ---------------------------------------------------------------------------

// RMS energy of the windowed frame.
float compute_rms(float s[kFrameLen])
{
    float sum_sq = 0.0f;
    for (int i = 0; i < kFrameLen; ++i)
        sum_sq += s[i] * s[i];
    return sqrt(sum_sq / float(kFrameLen));
}

// Amplitude-weighted mean frequency of the magnitude spectrum.
// Returns the centroid in bin units.
float compute_spectral_centroid(float mag[kDFTBins])
{
    float num = 0.0f;
    float den = 0.0f;
    for (int k = 0; k < kDFTBins; ++k) {
        num += float(k) * mag[k];
        den += mag[k];
    }
    if (den < kWhitenFloor)
        return 0.0f;
    return num / den;
}

// Amplitude-weighted standard deviation around the centroid.
float compute_spectral_bandwidth(float mag[kDFTBins], float centroid)
{
    float num = 0.0f;
    float den = 0.0f;
    for (int k = 0; k < kDFTBins; ++k) {
        float diff = float(k) - centroid;
        num += diff * diff * mag[k];
        den += mag[k];
    }
    if (den < kWhitenFloor)
        return 0.0f;
    return sqrt(num / den);
}

// Spectral flatness: ratio of geometric mean to arithmetic mean.
// A value near 1 indicates noise-like content; near 0 indicates a tonal signal.
float compute_spectral_flatness(float mag[kDFTBins])
{
    float log_sum = 0.0f;
    float lin_sum = 0.0f;
    for (int k = 0; k < kDFTBins; ++k) {
        float v = mag[k] + kWhitenFloor;
        log_sum += log(v);
        lin_sum += v;
    }
    float geom = exp(log_sum / float(kDFTBins));
    float arith = lin_sum / float(kDFTBins);
    return geom / arith;
}

// Zero-crossing rate: fraction of adjacent sample pairs with opposite signs.
float compute_zcr(float s[kFrameLen])
{
    int crossings = 0;
    for (int i = 1; i < kFrameLen; ++i) {
        if (s[i - 1] * s[i] < 0.0f)
            ++crossings;
    }
    return float(crossings) / float(kFrameLen - 1);
}

// Spectral rolloff: lowest bin index at which the cumulative spectral energy
// reaches kRolloffThresh (85%) of the total energy.
// Returns the rolloff bin as a float (integer part = bin index).
float compute_spectral_rolloff(float mag[kDFTBins])
{
    // Compute total power.
    float total = 0.0f;
    for (int k = 0; k < kDFTBins; ++k)
        total += mag[k] * mag[k];

    if (total < kWhitenFloor * kWhitenFloor)
        return 0.0f;

    float target    = kRolloffThresh * total;
    float cumulative = 0.0f;
    for (int k = 0; k < kDFTBins; ++k) {
        cumulative += mag[k] * mag[k];
        if (cumulative >= target)
            return float(k);
    }
    return float(kDFTBins - 1);
}

// ---------------------------------------------------------------------------
// Frame processing
// ---------------------------------------------------------------------------

// Write the ten features for frame `frame_id` into g_features.
[noinline]
void write_features(int    frame_id,
                    float  refined_peak,
                    float  peak_mag,
                    float  peak_width,
                    float  balance,
                    float  rms,
                    float  centroid,
                    float  bandwidth,
                    float  flatness,
                    float  zcr,
                    float  rolloff)
{
    int base = frame_id * kFeaturesPerFrame;
    g_features[base + 0] = refined_peak;
    g_features[base + 1] = peak_mag;
    g_features[base + 2] = peak_width;
    g_features[base + 3] = balance;
    g_features[base + 4] = rms;
    g_features[base + 5] = centroid;
    g_features[base + 6] = bandwidth;
    g_features[base + 7] = flatness;
    g_features[base + 8] = zcr;
    g_features[base + 9] = rolloff;
}

// Per-frame processing entry point.
[noinline]
void process_frame(int frame_id)
{
    // --- Stage 1: load ---------------------------------------------------------
    float samples[kFrameLen];
    load_frame(frame_id, samples);

    // --- Stage 2: preprocess ---------------------------------------------------
    remove_dc(samples);
    apply_preemphasis(samples);

    // Early-exit for silent frames (avoids division by zero in later stages).
    float pre_rms = compute_rms(samples);
    if (pre_rms < 1e-5f) {
        write_features(frame_id, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }

    // --- Stage 3: window -------------------------------------------------------
    float windowed[kFrameLen];
    for (int i = 0; i < kFrameLen; ++i)
        windowed[i] = samples[i];
    apply_hann_window(windowed);

    // --- Stage 4: DFT ----------------------------------------------------------
    float mag[kDFTBins];
    compute_dft(windowed, mag);

    // --- Stage 5: whitening ----------------------------------------------------
    float env  [kDFTBins];
    float white[kDFTBins];
    compute_whitening_envelope(mag, env);
    apply_whitening(mag, env, white);

    // --- Stage 6: peak detection -----------------------------------------------
    int peak_bin = find_peak_bin(white);

    // --- Stage 7: sub-bin refinement -------------------------------------------
    float refined = refine_peak_position(white, peak_bin);

    // --- Stage 7b-c: peak shape ------------------------------------------------
    float peak_width = estimate_peak_width(white, peak_bin, 0.5f);
    float balance    = compute_spectral_balance(mag, peak_bin);

    // --- Secondary features ----------------------------------------------------
    float rms       = compute_rms(windowed);
    float centroid  = compute_spectral_centroid(mag);
    float bandwidth = compute_spectral_bandwidth(mag, centroid);
    float flatness  = compute_spectral_flatness(mag);
    float zcr       = compute_zcr(samples);
    float rolloff   = compute_spectral_rolloff(mag);

    // --- Write output ----------------------------------------------------------
    write_features(frame_id, refined, white[peak_bin],
                   peak_width, balance,
                   rms, centroid, bandwidth, flatness, zcr, rolloff);
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int frame_id = int(tid.x);
    if (frame_id >= kFrameCount)
        return;
    process_frame(frame_id);
}
