// SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

// This isolated corpus workload uses its CUDA descriptor-handle branch.
#define __TARGET_CUDA__ 1

// Fully isolated Falcor2 MaterialX Tiled Brass (complex_mix).
// No imports, includes, SlangPy vectorization, or unused render systems are embedded.
namespace utils {

/// Return true if value contains any bit from flag.
[ForceInline]
public bool is_set<T : __EnumType>(T value, T flag)
{
    return value & flag;
}

/// Toggle flag in value.
public void flip_bit<T : __EnumType>(inout T value, T flag)
{
    value = is_set(value, flag) ? (value & ~flag) : (value | flag);
}

/// Utility for packing and unpacking bitfields within an integer type.
/// @tparam T Integer type used as the storage.
/// @tparam BitOffset Bit position of the field's least significant bit.
/// @tparam BitSize Number of bits in the field.
public __generic<T : __BuiltinIntegerType, let BitOffset : uint, let BitSize : uint> struct BitField
{
    // static_assert(BitSize + BitOffset < sizeof(T) * 8, "Size and offset exceed base type size.");

    /// Maximum value representable by this field.
    public static const T MAX_VALUE = (T(1) << BitSize) - T(1);
    /// Bitmask for this field, shifted to its position.
    public static const T MASK = ((T(1) << BitSize) - T(1)) << BitOffset;

    /// Insert a value into the field, preserving other bits.
    /// @param base The integer to modify.
    /// @param field The value to insert (must not exceed MAX_VALUE).
    public static void insert(inout T base, T field) { base = (base & ~MASK) | (field << BitOffset); }

    /// Extract the field value from an integer.
    /// @param base The integer to extract from.
    /// @return The extracted field value.
    public static T extract(T base) { return (base & MASK) >> BitOffset; }
};

public interface IMathConstants
{
    public static const This E;
    public static const This LOG2_E;
    public static const This LOG10_E;
    public static const This LN_TWO;
    public static const This LN_TEN;
    public static const This PI;
    public static const This TWO_PI;
    public static const This FOUR_PI;
    public static const This INV_PI;
    public static const This INV_TWO_PI;
    public static const This INV_FOUR_PI;
    public static const This PI_OVER_TWO;
    public static const This PI_OVER_FOUR;
    public static const This TWO_OVER_PI;
    public static const This FOUR_OVER_PI;
    public static const This SQRT_TWO;
    public static const This INV_SQRT_TWO;
    public static const This SQRT_THREE;
    public static const This SQRT_PI;
    public static const This INV_SQRT_PI;
    public static const This TWO_OVER_SQRT_PI;

    // TODO these are not math constants, so maybe they should be moved
    public static const This EPSILON;  // smallest such that 1.0+EPSILON != 1.0
    public static const This MAX;      // max value
    public static const This MIN;      // min positive value (normalized)
    public static const This TRUE_MIN; // min positive value
    public static const This INFINITY; // positive infinity
    public static const This NAN;      // not a number
};

#define concat(a, b) a##b

// clang-format off
#define DEFINE_MATH_CONSTANTS(type, suffix)                                                                     \
    public static const type E                  = concat(2.71828182845904523536, suffix);   /* e            */  \
    public static const type LOG2_E             = concat(1.44269504088896340736, suffix);   /* log2(e)      */  \
    public static const type LOG10_E            = concat(0.434294481903251827651, suffix);  /* log10(e)     */  \
    public static const type LN_TWO             = concat(0.693147180559945309417, suffix);  /* ln(2)        */  \
    public static const type LN_TEN             = concat(2.30258509299404568402, suffix);   /* ln(10)       */  \
    public static const type PI                 = concat(3.14159265358979323846, suffix);   /* pi           */  \
    public static const type TWO_PI             = concat(6.28318530717958647693, suffix);   /* 2pi          */  \
    public static const type FOUR_PI            = concat(12.5663706143591729539, suffix);   /* 4pi          */  \
    public static const type INV_PI             = concat(0.318309886183790671538, suffix);  /* 1/pi         */  \
    public static const type INV_TWO_PI         = concat(0.159154943091895335769, suffix);  /* 1/2pi        */  \
    public static const type INV_FOUR_PI        = concat(0.079577471545947667884, suffix);  /* 1/4pi        */  \
    public static const type PI_OVER_TWO        = concat(1.57079632679489661923, suffix);   /* pi/2         */  \
    public static const type PI_OVER_FOUR       = concat(0.785398163397448309616, suffix);  /* pi/4         */  \
    public static const type TWO_OVER_PI        = concat(0.636619772367581343076, suffix);  /* 2/pi         */  \
    public static const type FOUR_OVER_PI       = concat(1.27323954473516268615, suffix);   /* 4/pi         */  \
    public static const type SQRT_TWO           = concat(1.41421356237309504880, suffix);   /* sqrt(2)      */  \
    public static const type INV_SQRT_TWO       = concat(0.707106781186547524401, suffix);  /* 1/sqrt(2)    */  \
    public static const type SQRT_THREE         = concat(1.732050807568877293527, suffix);  /* sqrt(3)      */  \
    public static const type SQRT_PI            = concat(1.77245385090551602730, suffix);   /* sqrt(pi)     */  \
    public static const type INV_SQRT_PI        = concat(0.564189583547756279280, suffix);  /* 1/sqrt(pi)   */  \
    public static const type TWO_OVER_SQRT_PI   = concat(1.12837916709551257390, suffix);   /* 2/sqrt(pi)   */

// clang-format on

public extension half : IMathConstants
{
    DEFINE_MATH_CONSTANTS(half, h);

    public static const half EPSILON = 0.00097656h;
    public static const half MAX = 65504.0h;
    public static const half MIN = 6.10351562e-05h;
    public static const half TRUE_MIN = 5.96046448e-08h;
    public static const half INFINITY = 1.h / 0.h;
    public static const half NAN = 0.h / 0.h;
};

public extension float : IMathConstants
{
    DEFINE_MATH_CONSTANTS(float, f);

    public static const float EPSILON = 1.192092896e-07f;
    public static const float MAX = 3.402823466e+38f;
    public static const float MIN = 1.175494351e-38f;
    public static const float TRUE_MIN = 1.401298464e-45f;
    public static const float INFINITY = 1.f / 0.f;
    public static const float NAN = 0.f / 0.f;
};

public extension double : IMathConstants
{
    DEFINE_MATH_CONSTANTS(double, l);

    public static const double EPSILON = 2.2204460492503131e-016l;
    public static const double MAX = 1.7976931348623158e+308l;
    public static const double MIN = 2.2250738585072014e-308l;
    public static const double TRUE_MIN = 4.9406564584124654e-324l;
    public static const double INFINITY = 1.l / 0.l;
    public static const double NAN = 0.l / 0.l;
};

/// Returns the square of the input value.
[Differentiable]
public T sqr<T : __BuiltinFloatingPointType>(T x)
{
    return x * x;
}

/// Returns the norm of the input vector.
[Differentiable]
public T norm<T : __BuiltinFloatingPointType, int N>(vector<T, N> x)
{
    return length(x);
}

/// Returns the squared norm of the input vector.
[Differentiable]
public T squared_norm<T : __BuiltinFloatingPointType, int N>(vector<T, N> x)
{
    return dot(x, x);
}

/// Returns the square root of the input value, or 0 if the input is negative.
[Differentiable]
public T safe_sqrt<T : __BuiltinFloatingPointType>(T x)
{
    return sqrt(max(x, T(0)));
}

/// Multiplies 2 values together
[Differentiable]
public T multiply<T : IArithmetic>(T x, T y)
{
    return x * y;
}

/// Adds 2 values together
[Differentiable]
public T add<T : IArithmetic>(T x, T y)
{
    return x + y;
}

/// Subtracts 2 values
[Differentiable]
public T subtract<T : IArithmetic>(T x, T y)
{
    return x - y;
}

/// Divides 2 values
[Differentiable]
public T divide<T : IArithmetic>(T x, T y)
{
    return x / y;
}

/// Error function.
[Differentiable]
public T erf<T : __BuiltinFloatingPointType>(T x)
{
    // From "Numerical Recipes in C, The Art of Scientific Computing"
    // (Second Edition) by Press et al. 1992. Page 221.
    // Maxiumum error: 1.2 x 10^-7.
    T t = T(1.0) / (T(1.0) + T(0.5) * abs(x));
    T p = T(0.17087277);
    p = T(-0.82215223) + p * t;
    p = T(1.48851587) + p * t;
    p = T(-1.13520398) + p * t;
    p = T(0.27886807) + p * t;
    p = T(-0.18628806) + p * t;
    p = T(0.09678418) + p * t;
    p = T(0.37409196) + p * t;
    p = T(1.00002368) + p * t;
    p = T(-1.26551223) + p * t;
    T tau = t * exp(-x * x + p);
    return x >= T(0.0) ? T(1.0) - tau : tau - T(1.0);
}

/// Inverse error function.
[Differentiable]
public T erfinv<T : __BuiltinFloatingPointType>(T x)
{
    // From "Approximating the erfinv function" by Mike Giles 2012.
    // Maximum error: 7 x 10^-7.
    if (x <= T(-1.0)) {
        return T(-1.0) / T(0.0);
    } else if (x >= T(1.0)) {
        return T(1.0) / T(0.0);
    }

    T w = -log((T(1.0) - x) * (T(1.0) + x));
    T p;
    if (w < T(5.0)) {
        w = w - T(2.5);
        p = T(2.81022636e-08);
        p = T(3.43273939e-07) + p * w;
        p = T(-3.5233877e-06) + p * w;
        p = T(-4.39150654e-06) + p * w;
        p = T(0.00021858087) + p * w;
        p = T(-0.00125372503) + p * w;
        p = T(-0.00417768164) + p * w;
        p = T(0.246640727) + p * w;
        p = T(1.50140941) + p * w;
    } else {
        w = sqrt(w) - T(3.0);
        p = T(-0.000200214257);
        p = T(0.000100950558) + p * w;
        p = T(0.00134934322) + p * w;
        p = T(-0.00367342844) + p * w;
        p = T(0.00573950773) + p * w;
        p = T(-0.0076224613) + p * w;
        p = T(0.00943887047) + p * w;
        p = T(1.00167406) + p * w;
        p = T(2.83297682) + p * w;
    }
    return p * x;
}

/// Logarithm of the Gamma function.
[Differentiable]
public T lgamma<T : __BuiltinFloatingPointType>(T x_)
{
    // Lanczos approximation from
    // "Numerical Recipes in C, The Art of Scientific Computing"
    // (Second Edition) by Press et al. 1992. Page 214.
    // Maxiumum error: 2 x 10^-10.
    const T g = T(5.0);
    const T coeffs[7] = {
        T(1.000000000190015),
        T(76.18009172947146),
        T(-86.50532032941677),
        T(24.01409824083091),
        T(-1.231739572450155),
        T(0.1208650973866179e-2),
        T(-0.5395239384953e-5),
    };

    bool reflect = x_ < T(0.5);
    T x = reflect ? -x_ : x_ - T(1.0);
    T base = x + g + T(0.5);

    T s = T(0.0);
    for (int i = 6; i >= 1; --i) {
        s += coeffs[i] / (x + T(i));
    }
    s += coeffs[0];

    T res = ((log(sqrt(T(2.0) * T.getPi())) + log(s)) - base) + log(base) * (x + T(0.5));
    if (reflect) {
        // Apply the Gamma reflection formula (in log-space).
        res = log(abs(T.getPi() / sin(T.getPi() * x_))) - res;
    }
    return res;
}

/// Gamma function.
[Differentiable]
public T gamma<T : __BuiltinFloatingPointType>(T x)
{
    return exp(lgamma(x));
}

/// Beta function.
[Differentiable]
public T beta<T : __BuiltinFloatingPointType>(T x, T y)
{
    return gamma(x) * gamma(y) / gamma(x + y);
}

/// Luminance of an RGB in Rec.709.
[Differentiable]
public float luminance(float3 rgb)
{
    return dot(rgb, float3(0.2126, 0.7152, 0.0722));
}

/// Non-negative finite luminance of an RGB value in Rec.709.
///
/// Non-finite and negative channels are treated as zero. If the weighted sum
/// overflows, the result is also zero. This is intended for statistics and
/// importance calculations where invalid radiance must not poison an
/// accumulation.
public float safe_luminance(float3 rgb)
{
    float3 safe_rgb = float3(
        isfinite(rgb.x) ? max(rgb.x, 0.f) : 0.f,
        isfinite(rgb.y) ? max(rgb.y, 0.f) : 0.f,
        isfinite(rgb.z) ? max(rgb.z, 0.f) : 0.f
    );
    float result = luminance(safe_rgb);
    return isfinite(result) ? result : 0.f;
}

/// Transform RGB in Rec.709 to CIE XYZ.
[Differentiable]
public float3 rgb_to_xyz_rec709(float3 c)
{
    static const float3x3 M = {
        // clang-format off
        0.4123907992659595, 0.3575843393838780, 0.1804807884018343, // row 0
        0.2126390058715104, 0.7151686787677559, 0.0721923153607337, // row 1
        0.0193308187155918, 0.1191947797946259, 0.9505321522496608, // row 2
        // clang-format off
    };
    return mul(M, c);
}

/// Transform CIE XYZ to RGB in Rec.709.
[Differentiable]
public float3 xyz_to_rgb_rec709(float3 c)
{
    static const float3x3 M = {
        // clang-format off
        3.240969941904522,   -1.537383177570094,  -0.4986107602930032, // row 0
        -0.9692436362808803, 1.875967501507721,   0.04155505740717569, // row 1
        0.05563007969699373, -0.2039769588889765, 1.056971514242878,   // row 2
        // clang-format on
    };
    return mul(M, c);
}

/// Transform RGB to YCgCo space.
[Differentiable]
public float3 rgb_to_ycgco(float3 rgb)
{
    float Y = dot(rgb, float3(0.25, 0.50, 0.25));
    float Cg = dot(rgb, float3(-0.25, 0.50, -0.25));
    float Co = dot(rgb, float3(0.50, 0.00, -0.50));
    return float3(Y, Cg, Co);
}

/// Transform YCgCo to RGB space.
public float3 ycgco_to_rgb(float3 ycgco)
{
    float tmp = ycgco.x - ycgco.y;
    float r = tmp + ycgco.z;
    float g = ycgco.x + ycgco.y;
    float b = tmp - ycgco.z;
    return float3(r, g, b);
}

/// Transform RGB to YUV (Rec.709).
[Differentiable]
public float3 rgb_to_yuv(float3 rgb)
{
    float3 ret;
    ret.x = dot(rgb, float3(0.2126, 0.7152, 0.0722));
    ret.y = dot(rgb, float3(-0.09991, -0.33609, 0.436));
    ret.z = dot(rgb, float3(0.615, -0.55861, -0.05639));
    return ret;
}

/// Transform YUV to RGB (Rec.709).
[Differentiable]
public float3 yuv_to_rgb(float3 yuv)
{
    float3 ret;
    ret.x = dot(yuv, float3(1.0, 0.0, 1.28033));
    ret.y = dot(yuv, float3(1.0, -0.21482, -0.38059));
    ret.z = dot(yuv, float3(1.0, 2.12798, 0.0));
    return ret;
}

/// Transform sRGB to linear RGB.
[Differentiable]
public float srgb_to_linear(float srgb)
{
    if (srgb <= 0.04045)
        return srgb * (1.0 / 12.92);
    else
        return pow((srgb + 0.055) * (1.0 / 1.055), 2.4);
}

/// Transform sRGB to linear RGB.
[Differentiable]
public float3 srgb_to_linear(float3 srgb)
{
    return float3(srgb_to_linear(srgb.x), srgb_to_linear(srgb.y), srgb_to_linear(srgb.z));
}

/// Transform linear RGB to sRGB.
[Differentiable]
public float linear_to_srgb(float linear)
{
    if (linear <= 0.0031308)
        return linear * 12.92;
    else
        return pow(linear, (1.0 / 2.4)) * (1.055) - 0.055;
}

/// Transform linear RGB to sRGB.
[Differentiable]
public float3 linear_to_srgb(float3 linear)
{
    return float3(linear_to_srgb(linear.x), linear_to_srgb(linear.y), linear_to_srgb(linear.z));
}


public static const float3 D65_REFERENCE_ILLUMINANT = float3(0.950428545, 1.000000000, 1.088900371);
public static const float3 D65_REFERENCE_ILLUMINANT_INV = float3(1.052156925, 1.000000000, 0.918357670);


/// Transform linear RGB to XYZ space (D65 illuminant).
[Differentiable]
public float3 linear_rgb_to_xyz(float3 rgb)
{
    // Source:
    // https://www.image-engineering.de/library/technotes/958-how-to-convert-between-srgb-and-ciexyz
    // Assumes D65 standard illuminant.
    const float a11 = 10135552.0 / 24577794.0;
    const float a12 = 8788810.0 / 24577794.0;
    const float a13 = 4435075.0 / 24577794.0;
    const float a21 = 2613072.0 / 12288897.0;
    const float a22 = 8788810.0 / 12288897.0;
    const float a23 = 887015.0 / 12288897.0;
    const float a31 = 1425312.0 / 73733382.0;
    const float a32 = 8788810.0 / 73733382.0;
    const float a33 = 70074185.0 / 73733382.0;

    float3 xyz;
    xyz.x = a11 * rgb.r + a12 * rgb.g + a13 * rgb.b;
    xyz.y = a21 * rgb.r + a22 * rgb.g + a23 * rgb.b;
    xyz.z = a31 * rgb.r + a32 * rgb.g + a33 * rgb.b;

    return xyz;
}

/// Transform XYZ to linear RGB space (D65 illuminant).
[Differentiable]
public float3 xyz_to_linear_rgb(float3 xyz)
{
    // Return values in linear RGB, assuming D65 standard illuminant.
    const float a11 = 3.241003275;
    const float a12 = -1.537398934;
    const float a13 = -0.498615861;
    const float a21 = -0.969224334;
    const float a22 = 1.875930071;
    const float a23 = 0.041554224;
    const float a31 = 0.055639423;
    const float a32 = -0.204011202;
    const float a33 = 1.057148933;

    float3 rgb;
    rgb.r = a11 * xyz.x + a12 * xyz.y + a13 * xyz.z;
    rgb.g = a21 * xyz.x + a22 * xyz.y + a23 * xyz.z;
    rgb.b = a31 * xyz.x + a32 * xyz.y + a33 * xyz.z;

    return rgb;
}

[Differentiable]
float actual_pow(float x, float y)
{
    if (x >= 0.0)
        return pow(x, y);
    else
        return -pow(-x, y);
}

[Differentiable]
float3 actual_pow(float3 in, float y)
{
    return float3(actual_pow(in.x, y), actual_pow(in.y, y), actual_pow(in.z, y));
}

[Differentiable]
public float3 xyz_to_oklab(float3 xyz)
{
    static const float3x3 M1 = {
        // clang-format off
        0.8189330101, 0.3618667424, -0.1288597137, // row 0
        0.0329845436, 0.9293118715, 0.0361456387,  // row 1
        0.0482003018, 0.2643662691, 0.6338517070   // row 2
        // clang-format on
    };
    float3 lms = mul(M1, xyz);
    lms = actual_pow(lms, 1.0 / 3.0);
    static const float3x3 M2 = {
        // clang-format off
        0.2104542553, 0.7936177850, -0.0040720468, // row 0
        1.9779984951, -2.4285922050, 0.4505937099, // row 1
        0.0259040371, 0.7827717662, -0.8086757660  // row 2
        // clang-format on
    };
    return mul(M2, lms);
}

[Differentiable]
public float3 oklab_to_xyz(float3 oklab)
{
    static const float3x3 M2_INV = float3x3(
        // clang-format off
        1.00000000, 0.39633779,  0.21580376,  // row 0
        1.00000001, -0.10556134, -0.06385417, // row 1
        1.00000005, -0.08948418, -1.29148554  // row 2
        // clang-format off
    );
    float3 lms = mul(M2_INV, oklab);
    lms = lms * lms * lms;
    static const float3x3 M1_INV = float3x3(
        // clang-format off
        1.22701385,  -0.55779998, 0.28125615,  // row 0
        -0.04058018, 1.11225687,  -0.07167668, // row 1
        -0.07638128, -0.42148198, 1.58616322   // row 2
        // clang-format on
    );
    return mul(M1_INV, lms);
}

[Differentiable]
public float3 oklab_to_hcl(float3 oklab)
{
    float L = oklab.x;
    float a = oklab.y;
    float b = oklab.z;

    float H = atan2(b, a);
    float C = sqrt(a * a + b * b);
    return float3(H, C, L);
}

[Differentiable]
public float3 hcl_to_oklab(float3 hcl)
{
    float H = hcl.x;
    float C = hcl.y;
    float L = hcl.z;

    float a = C * cos(H);
    float b = C * sin(H);
    return float3(L, a, b);
}

[Differentiable]
public float3 rgb_to_oklab(float3 rgb)
{
    return xyz_to_oklab(linear_rgb_to_xyz(rgb));
}

[Differentiable]
public float3 oklab_to_rgb(float3 oklab)
{
    return xyz_to_linear_rgb(oklab_to_xyz(oklab));
}

[Differentiable]
public float3 xyz_to_cie_lab(float3 xyz, const float3 inv_reference_illuminant = D65_REFERENCE_ILLUMINANT_INV)
{
    // The default illuminant is D65.
    float3 tmp = xyz * inv_reference_illuminant;

    float delta = 6.0 / 29.0;
    float delta2 = delta * delta;
    float delta3 = delta * delta2;
    float factor = 1.0 / (3.0 * delta2);
    float term = 4.0 / 29.0;

    tmp.r = (tmp.r > delta3 ? pow(tmp.r, 1.0 / 3.0) : factor * tmp.r + term);
    tmp.g = (tmp.g > delta3 ? pow(tmp.g, 1.0 / 3.0) : factor * tmp.g + term);
    tmp.b = (tmp.b > delta3 ? pow(tmp.b, 1.0 / 3.0) : factor * tmp.b + term);

    float3 cie_lab;
    cie_lab.r = 116.0f * tmp.g - 16.0f;
    cie_lab.g = 500.0f * (tmp.r - tmp.g);
    cie_lab.b = 200.0f * (tmp.g - tmp.b);

    return cie_lab;
}

[Differentiable]
public float3 cie_lab_to_xyz(float3 cie_lab, const float3 reference_illuminant = D65_REFERENCE_ILLUMINANT)
{
    // The default illuminant is D65.
    float Y = (cie_lab.r + 16.0) / 116.0;
    float X = cie_lab.g / 500.0 + Y;
    float Z = Y - cie_lab.b / 200.0;

    float delta = 6.0 / 29.0;
    float factor = 3.0 * delta * delta;
    float term = 4.0 / 29.0;
    X = ((X > delta) ? X * X * X : (X - term) * factor);
    Y = ((Y > delta) ? Y * Y * Y : (Y - term) * factor);
    Z = ((Z > delta) ? Z * Z * Z : (Z - term) * factor);

    return float3(X, Y, Z) * reference_illuminant;
}

[Differentiable]
public float3 xyz_to_ycxcz(float3 xyz, const float3 inv_reference_illuminant = D65_REFERENCE_ILLUMINANT_INV)
{
    // The default illuminant is D65.
    float3 tmp = xyz * inv_reference_illuminant;

    float3 ycxcz;
    ycxcz.x = 116.0 * tmp.g - 16.0;
    ycxcz.y = 500.0 * (tmp.r - tmp.g);
    ycxcz.z = 200.0 * (tmp.g - tmp.b);

    return ycxcz;
}

[Differentiable]
public float3 ycxcz_to_xyz(float3 ycxcz, const float3 reference_illuminant = D65_REFERENCE_ILLUMINANT)
{
    // The default illuminant is D65.
    float Y = (ycxcz.r + 16.0) / 116.0;
    float X = ycxcz.g / 500.0 + Y;
    float Z = Y - ycxcz.b / 200.0;

    return float3(X, Y, Z) * reference_illuminant;
}

[Differentiable]
public float3 linear_rgb_to_cie_lab(float3 rgb)
{
    return xyz_to_cie_lab(linear_rgb_to_xyz(rgb));
}

[Differentiable]
public float3 ycxcz_to_linear_rgb(float3 ycxcz)

{
    return xyz_to_linear_rgb(ycxcz_to_xyz(ycxcz));
}

[Differentiable]
public float3 linear_rgb_to_ycxcz(float3 rgb)
{
    return xyz_to_ycxcz(linear_rgb_to_xyz(rgb));
}


/// Evaluates the 1931 CIE XYZ color matching curves.
/// This uses a multi-lobe piecewise Gaussian fit described in:
/// Wyman et al., "Simple Analytic Approximations to the CIE XYZ Color
/// Matching Functions", JCGT, vol 2(2), 2013.
/// @param lambda Wavelength in nm.
/// @return XYZ tristimulus values.
public float3 wavelength_to_xyz(float lambda)
{
    float x = 0.362 * G(lambda, 442.0, 0.0624, 0.0374) + 1.056 * G(lambda, 599.8, 0.0264, 0.0323)
        - 0.065 * G(lambda, 501.1, 0.0490, 0.0382);
    float y = 0.821 * G(lambda, 568.8, 0.0213, 0.0247) + 0.286 * G(lambda, 530.9, 0.0613, 0.0322);
    float z = 1.217 * G(lambda, 437.0, 0.0845, 0.0278) + 0.681 * G(lambda, 459.0, 0.0385, 0.0725);
    return float3(x, y, z);
}

float G(float x, float lambda, float inv_sigma_1, float inv_sigma_2)
{
    float t = (x - lambda) * (x < lambda ? inv_sigma_1 : inv_sigma_2);
    return exp(-0.5 * t * t);
}


/// Evaluates the Fresnel term using Schlick's approximation.
/// Introduced in http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
///
/// The Fresnel term equals f0 at normal incidence, and approaches f90=1.0 at 90 degrees.
/// The formulation below is generalized to allow both f0 and f90 to be specified.
///
/// @param f0 Specular reflectance at normal incidence (0 degrees).
/// @param f90 Reflectance at orthogonal incidence (90 degrees), which should be 1.0 for specular surface reflection.
/// @param cos_theta Cosine of angle between microfacet normal and incident direction.
/// @return Fresnel term.
[Differentiable]
public float3 eval_fresnel_schlick(float3 f0, float3 f90, float cos_theta)
{
    const float x = max(1 - cos_theta, 0); // Preserve the endpoint if cos_theta = 1+epsilon.
    const float x2 = x * x;
    const float x5 = x2 * x2 * x;
    return f0 + (f90 - f0) * x5;
}

[Differentiable]
public float eval_fresnel_schlick(float f0, float f90, float cos_theta)
{
    const float x = max(1 - cos_theta, 0); // Preserve the endpoint if cos_theta = 1+epsilon.
    const float x2 = x * x;
    const float x5 = x2 * x2 * x;
    return f0 + (f90 - f0) * x5;
}

public struct Frame : IDifferentiable
{
    // Frame uses the common shading-space convention where the surface normal is local +Z.
    // This is independent of the engine's world-space up axis, which may be Y-up.
    public float3 normal = {};
    public float3 tangent = {};
    public float3 bitangent = {};

    public enum Component {
        normal,
        tangent,
    }

    public __init() { }

    public static Frame identity()
    {
        Frame frame = {};
        frame.normal = float3(0.0, 0.0, 1.0);
        frame.tangent = float3(1.0, 0.0, 0.0);
        frame.bitangent = float3(0.0, 1.0, 0.0);
        return frame;
    }

    [Differentiable]
    public __init(float3 normal)
    {
        // Using branchlessONB from
        // https://graphics.pixar.com/library/OrthonormalB/paper.pdf
        this.normal = normal;
        branchless_ONB(this.normal, this.tangent, this.bitangent);
    }

    // normal and tangent are both assumed to be normalized, but not necessarily orthonormal.
    // handedness is with respect to the UV tangent space, not the geometry orientation.
    [Differentiable]
    public __init(
        float3 normal,
        float3 tangent,
        float handedness = 1.0,
        Component preserved_component = Component::normal
    )
    {
        // Only useful for extreme bump cases where tangent and normal can become almost colinear.
        // if (abs(dot(normal, tangent)) >= 0.999) {
        //     this.normal = normal;
        //     branchless_ONB(this.normal, this.tangent, this.bitangent);
        //     return;
        // }
        if (preserved_component == Component::normal) {
            this.normal = normal;
            this.bitangent = normalize(cross(normal, tangent));
            this.tangent = cross(bitangent, normal);
        } else // preserve tangent, used for curves
        {
            this.tangent = tangent;
            this.bitangent = normalize(cross(normal, tangent));
            this.normal = cross(tangent, bitangent);
        }
        this.bitangent *= handedness;
    }

    [Differentiable]
    public float3 to_local(float3 w) { return float3(dot(tangent, w), dot(bitangent, w), dot(normal, w)); }

    [Differentiable]
    public float3 to_global(float3 w) { return w.x * tangent + w.y * bitangent + w.z * normal; }

    // Returns sign that gets applied to such that bitangent = cross(normal, tangent) * handedness_sign();
    // Returns +1 for right handed, and -1 for left handed frames.
    public float handedness_sign() { return dot(cross(normal, tangent), bitangent) >= 0.f ? 1.f : -1.f; }
};

/// Given v1 and v2 where v1 is normalized and v1 and v2 define a plane
/// (v2 is not zero length and not colinear to v1), this method adjusts v2,
/// such that it is orthonormal to v1, lying in the plane defined by the two vectors.
public void orthonormalize(const float3 v1, inout float3 v2)
{
    v2 = normalize(v2 - dot(v1, v2) * v1);
}

public void branchless_ONB(float3 normal, out float3 tangent, out float3 bitangent)
{
    // Using branchlessONB from
    // https://graphics.pixar.com/library/OrthonormalB/paper.pdf

    // Workaround because slang's copysign is currently not differentiable
    // https://github.com/shader-slang/slang/issues/7549
    // float sign = copysign(1.0, normal.z);
    float sign = select(normal.z < 0.0, -1.0, 1.0);
    const float a = -1.0f / (sign + normal.z);
    const float b = normal.x * normal.y * a;
    tangent = float3(1.0f + sign * normal.x * normal.x * a, sign * b, -sign * normal.x);
    bitangent = float3(b, sign + normal.y * normal.y * a, -normal.y);
}

float3 adjust_shading_normal_impl(
    const float3 shading_normal,
    const float3 geometric_normal,
    const float3 view_dir,
    out bool was_adjusted
)
{
    was_adjusted = false;

    // The shading normal Ns may lie on either side depending on whether we're shading the front or back.
    // We orient Ns to lie on the same side as the view direction and Ng for the computations below.
    // The final adjusted normal is oriented to lie on the same side as the original shading normal.
    float3 Ng = geometric_normal;
    float sign_N = dot(shading_normal, Ng) >= 0.f ? 1.f : -1.f;
    float3 Ns = sign_N * shading_normal;

#if 1 // FALCOR_INTERNAL
    // Algorithm from Appendix A.3 of https://arxiv.org/abs/1705.01263
    // The specular reflection of the view direction is computed.
    // If the reflection vector lies under the horizon, the shading normal is adjusted.
    // This ensures that the majority of the reflection lobe lies on the same side,
    // but it does not guarantee that all directions are valid in the general case.

    // Specular reflection in shading normal
    float3 R = reflect(-view_dir, Ns);
    float a = dot(Ng, R);
    if (a < 0) // Perturb normal
    {
        float b = max(0.001, dot(Ns, Ng));
        was_adjusted = true;
        return normalize(sign_N * (view_dir + normalize(R - Ns * a / b)));
    }
#else  // FALCOR_INTERNAL
    // Blend the shading normal towards the geometric normal at grazing angles.
    // This is to avoid the view vector from becoming back-facing.
    const float COS_THETA_THRESHOLD = 0.1f;
    float cos_theta = dot(view_dir, Ns);
    if (cos_theta <= COS_THETA_THRESHOLD) {
        float t = saturate(cos_theta * (1.f / COS_THETA_THRESHOLD));
        was_adjusted = true;
        return normalize(sign_N * lerp(Ng, Ns, t));
    }
#endif // FALCOR_INTERNAL

    return shading_normal;
}

/// Adjusts a unit-length shading normal to avoid invalid reflection directions.
/// The geometric normal and view direction must be unit length and lie in the same hemisphere.
/// The result remains on the shading normal's authored side of the geometric normal.
public float3 adjust_shading_normal(const float3 shading_normal, const float3 geometric_normal, const float3 view_dir)
{
    bool was_adjusted;
    return adjust_shading_normal_impl(shading_normal, geometric_normal, view_dir, was_adjusted);
}

public void adjust_shading_normal(inout Frame sf, const float3 geometric_normal, const float3 view_dir)
{
    bool was_adjusted;
    const float3 adjusted_normal = adjust_shading_normal_impl(sf.normal, geometric_normal, view_dir, was_adjusted);
    if (was_adjusted)
        sf = Frame(adjusted_normal, sf.tangent, sf.handedness_sign());
}

public interface ISampleGenerator
{
    /// Initializes the sample generator from a given seed value.
    /// @param seed Seed value.
    public __init(uint seed);

    /// Initializes the sample generator for a given pixel and sample index.
    /// @param pixel Pixel id.
    /// @param sample_index Sample index.
    public __init(uint2 pixel, uint sample_index);

    /// Returns the next sample value.
    /// @return Sample value in [0,2^32).
    [mutating]
    public uint next_uint();
}

/// Return the next sample value as a float in [0,1).
/// @param[inout] sg Sample generator.
/// @return A float in [0,1).
public float next_float<SG : ISampleGenerator>(inout SG sg)
{
    // Use upper 24 bits and divide by 2^24 to get a number u in [0,1).
    // In floating-point precision this also ensures that 1.0-u != 0.0.
    return (sg.next_uint() >> 8) * 0x1p-24;
}

/// Return the next 2 sample values as a float2 in [0,1).
/// @param[inout] sg Sample generator.
/// @return A float2 in [0,1).
public float2 next_float2<SG : ISampleGenerator>(inout SG sg)
{
    float2 sample;
    // Don't use the float2 initializer to ensure consistent order of evaluation.
    sample.x = next_float(sg);
    sample.y = next_float(sg);
    return sample;
}

/// Return the next 3 sample values as a float3 in [0,1).
/// @param[inout] sg Sample generator.
/// @return A float3 in [0,1).
public float3 next_float3<SG : ISampleGenerator>(inout SG sg)
{
    float3 sample;
    // Don't use the float3 initializer to ensure consistent order of evaluation.
    sample.x = next_float(sg);
    sample.y = next_float(sg);
    sample.z = next_float(sg);
    return sample;
}

/// Return the next 4 sample values as a float4 in [0,1).
/// @param[inout] sg Sample generator.
/// @return A float4 in [0,1).
public float4 next_float4<SG : ISampleGenerator>(inout SG sg)
{
    float4 sample;
    // Don't use the float4 initializer to ensure consistent order of evaluation.
    sample.x = next_float(sg);
    sample.y = next_float(sg);
    sample.z = next_float(sg);
    sample.w = next_float(sg);
    return sample;
}
public struct TinyUniformSampleGenerator : ISampleGenerator
{
    uint state;
    public __init(uint seed) { state = seed; }
    public __init(uint2 pixel, uint sample_index) { state = pixel.x ^ (pixel.y * 0x9e3779b9u) ^ sample_index; }
    [mutating]
    public uint next_uint()
    {
        state = 1664525u * state + 1013904223u;
        return state;
    }
}
public float2 square_to_uniform_disk_concentric(float2 sample)
{
    float2 t = 2.0 * sample - 1.0;
    bool is_zero = all(t == 0.0);
    bool swap = abs(t.x) < abs(t.y);
    float r = select(swap, t.y, t.x);
    float rp = select(swap, t.x, t.y);
    float phi = 0.25 * float.PI * rp / r;
    phi = select(swap, 0.5 * float.PI - phi, phi);
    phi = select(is_zero, 0.0, phi);
    float s, c;
    sincos(phi, s, c);
    return float2(c, s) * r;
}
}
using namespace utils;
namespace render {
using namespace utils;


/** Low-level interface for BSDF functions.

    Conventions:
    - All operations are done in a local coordinate frame.
    - The local frame has normal N=(0,0,1), tangent T=(1,0,0) and bitangent B=(0,1,0).
    - The incident and outgoing direction point away from the shading location.
    - For surface BSDFs, the incident direction (wi) is always in the positive hemisphere.
    - Curve BSDFs, such as hair scattering in a curve frame, may evaluate and sample over the full sphere.
    - The outgoing direction (wo) is sampled.
    - Surface BSDF eval() returns f(wi, wo) * abs(dot(wo, n)). Curve BSDF eval() returns the model
      scattering response and does not apply an extra surface foreshortening term. Curve-specific measure
      terms and Jacobians belong to the BSDF and its pdf
      (see https://pbr-book.org/4ed/Reflection_Models/Scattering_from_Hair).
*/

/// Minimum cos(theta) for the incident and outgoing vectors.
/// Some BSDF functions are not robust for cos(theta) == 0.0,
/// so using a small epsilon for consistency.
public static constexpr float BSDF_MIN_COS_THETA = 1e-6;

public enum BSDFFlags : uint {
    none = 0x00,

    diffuse_reflection = 0x01,
    glossy_reflection = 0x02,
    delta_reflection = 0x04,

    diffuse_transmission = 0x10,
    glossy_transmission = 0x20,
    delta_transmission = 0x40,

    glossy_curve = 0x80,

    diffuse = diffuse_reflection | diffuse_transmission,
    glossy = glossy_reflection | glossy_transmission,
    delta = delta_reflection | delta_transmission,
    curve = glossy_curve,

    reflection = diffuse_reflection | glossy_reflection | delta_reflection,
    transmission = diffuse_transmission | glossy_transmission | delta_transmission,

    non_delta = diffuse | glossy | curve,
    non_delta_reflection = diffuse_reflection | glossy_reflection,
    non_delta_transmission = diffuse_transmission | glossy_transmission,

    all = reflection | transmission | curve,
};

/// Result of a BSDF sample.
public struct BSDFSample : IDifferentiable
{
    /// Outgoing direction.
    public float3 wo;
    /// pdf with respect to solid angle for sampling outgoing direction wo (0 for delta events).
    public float pdf;
    /// Sample weight eval(wi, wo) / pdf(wo).
    public float3 weight;
    /// Flags describing the sampled scattering event (see BSDFFlags).
    public BSDFFlags flags;

    /// Return true if the sample contains any of the specified flags.
    public bool has_flag(const BSDFFlags flags) { return is_set(this.flags, flags); }
}

/// Compile-time hints specifying which components eval_with_pdf() must return.
/// Implementations may compute components that were not requested, but callers must not rely on them.
public enum BSDFEvalHints : uint {
    none = 0x0,
    eval = 0x1,
    pdf = 0x2,
    both = eval | pdf,
}

/// Result of evaluating a BSDF and its directional sampling density together.
public struct BSDFEval : IDifferentiable
{
    /// BSDF value using the same convention as IBSDF.eval().
    public float3 value;
    /// Directional pdf using the same convention as IBSDF.eval_pdf().
    public float pdf;

    [Differentiable]
    public __init()
    {
        value = float3(0.0);
        pdf = 0.0;
    }

    [Differentiable]
    public __init(float3 value, float pdf)
    {
        this.value = value;
        this.pdf = pdf;
    }
}

/// Albedo is split into four components. They have to sum up to 1.0.
public struct AlbedoContributions
{
    /// Energy reflected into upper hemisphere.
    public float3 reflection;
    /// Energy absorbed in upper hemisphere.
    public float3 absorption_r;
    /// Energy transmitted into lower hemisphere.
    public float3 transmission;
    /// Energy absorbed in lower hemisphere.
    public float3 absorption_t;

    public __init(float3 reflection, float3 absorption_r, float3 transmission, float3 absorption_t)
    {
        this.reflection = reflection;
        this.absorption_r = absorption_r;
        this.transmission = transmission;
        this.absorption_t = absorption_t;
    }
};

AlbedoContributions operator +(const AlbedoContributions a, const AlbedoContributions b)
{
    return AlbedoContributions(
        a.reflection + b.reflection,
        a.absorption_r + b.absorption_r,
        a.transmission + b.transmission,
        a.absorption_t + b.absorption_t
    );
}

AlbedoContributions operator +=(inout AlbedoContributions a, const AlbedoContributions b)
{
    a.reflection += b.reflection;
    a.absorption_r += b.absorption_r;
    a.transmission += b.transmission;
    a.absorption_t += b.absorption_t;

    return a;
}

AlbedoContributions operator*(const AlbedoContributions a, const AlbedoContributions b)
{
    return AlbedoContributions(
        a.reflection * b.reflection,
        a.absorption_r * b.absorption_r,
        a.transmission * b.transmission,
        a.absorption_t * b.absorption_t
    );
}

AlbedoContributions operator*(const float f, const AlbedoContributions b)
{
    return AlbedoContributions(f * b.reflection, f * b.absorption_r, f * b.transmission, f * b.absorption_t);
}

public struct RoughnessInformation
{
    /// The (mostly meaningless) non-universal roughness of the BSDF.
    public float2 roughness;
    public float3 scratch;

    public __init()
    {
        roughness = float2(0.f);
        scratch = float3(0.f);
    }

    public __init(float2 roughness, float3 scratch)
    {
        this.roughness = roughness;
        this.scratch = scratch;
    }
}

public struct BSDFContext
{
    /// Index of refraction from incidence medium.
    public float ior_i;
    /// Index of refraction from transmission medium.
    public float ior_t;
    /// Soft sampling-efficiency hint from a higher-level material.
    ///
    /// The caller expects sampled lobes outside this mask to be discarded by
    /// surrounding material logic, so a BSDF may avoid sampling them and report
    /// the matching hinted sampling PDF. Ignoring this hint is still correct
    /// BSDF behavior; it only spends samples on events the caller will later
    /// reject.
    public BSDFFlags sample_lobe_types_hint;

    public __init()
    {
        ior_i = 1.0;
        ior_t = 1.0;
        sample_lobe_types_hint = BSDFFlags::all;
    }

    public __init(float ior_i, float ior_t)
    {
        this.ior_i = ior_i;
        this.ior_t = ior_t;
        sample_lobe_types_hint = BSDFFlags::all;
    }

    public __init(float ior_i, float ior_t, BSDFFlags sample_lobe_types_hint)
    {
        this.ior_i = ior_i;
        this.ior_t = ior_t;
        this.sample_lobe_types_hint = sample_lobe_types_hint;
    }
};

/// Deterministic sample generator used by the generator-free eval_pdf() convenience alias.
///
/// Random samples may affect only the value returned by eval_with_pdf(), never its PDF. Changing that
/// contract would invalidate eval_pdf() and must be treated with utmost care.
public struct EvalPdfFallbackSampleGenerator : ISampleGenerator
{
    public __init(uint seed) { }
    public __init(uint2 pixel, uint sample_index) { }

    [mutating]
    // next_float() maps the upper 24 bits to the midpoint sample 0.5.
    public uint next_uint() { return 0x80000000u; }
};

public interface IBSDF
{
    /// Evaluates the BSDF.
    /// @param wi Incident direction.
    /// @param wo Outgoing direction.
    /// @param[inout] sg Sample generator.
    /// @param bc BSDF context.
    /// @return For surface BSDFs, returns f(wi, wo) * abs(dot(wo, n)). Curve BSDFs return their scattering
    /// response without an extra surface foreshortening factor.
    public float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    /// Evaluates the BSDF and its directional pdf together.
    /// @param wi Incident direction.
    /// @param wo Outgoing direction.
    /// @param[inout] sg Sample generator.
    /// @param bc BSDF context.
    /// The sample generator may affect only the value calculation; the PDF must be deterministic and
    /// generator-independent.
    /// @return The BSDF value and solid-angle pdf returned by eval() and eval_pdf(), respectively.
    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc);

    /// Samples the BSDF.
    /// @param wi Incident direction.
    /// @param[out] sample BSDF sample. Only valid if the function returns true.
    /// @param[inout] sg Sample generator.
    /// @param bc BSDF context.
    /// @return Returns true if successful.
    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc);

    /// Evaluates the directional pdf for sampling outgoing direction wo.
    /// @param wi Incident direction.
    /// @param wo Outgoing direction.
    /// @param bc BSDF context.
    /// @return Returns the pdf with respect to solid angle for sampling outgoing direction wo (0 for delta events).
    public float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    /// Evaluate albedos (hemispherical reflectance) of the BSDF. Reflection + transmission hemisphere should be <= 1.0.
    /// Curve BSDFs may report an approximate full-sphere scattering albedo.
    /// @param wi Incident direction.
    /// @return Returns the albedo contributions.
    public AlbedoContributions eval_albedo(const float3 wi);

    /// Information about roughness of the BSDF in (various) forms.
    /// @param wi Incident direction.
    /// @return Returns the roughness.
    public RoughnessInformation eval_roughness(const float3 wi);

    /// Unattenuated reflectance at normal incidence primarily used to get a common-form IOR.
    public property float3 ior_as_reflectance { get; }
};

public interface IInteraction : IDifferentiable
{
    /// Incident direction (i.e. -ray.dir) in world-space.
    public property float3 wi_ws;
    /// Position in world-space.
    public property float3 position_ws;
};

public interface ISurfaceInteraction : IInteraction
{
    /// Geometric normal in world-space.
    public property no_diff float3 normal_ws;
    /// True if surface is seen from the front (outside), false when seen from the back (inside).
    public property bool front_facing;
    /// Interpolated shading-frame in world-space.
    public property Frame shading_frame_ws;
};

public struct SurfaceInteraction : ISurfaceInteraction
{
    /// Incident direction (i.e. -ray.dir) in world-space.
    public float3 wi_ws = {};
    /// Position in world-space.
    public float3 position_ws = {};
    /// Geometric normal in world-space.
    public no_diff float3 normal_ws = {};
    /// True if surface is seen from the front (outside), false when seen from the back (inside).
    public bool front_facing = {};
    /// Interpolated shading-frame in world-space.
    public Frame shading_frame_ws = {};

    /// Texture coordinates.
    public float2 uv = {};
    /// Index of refraction for the medium on the front-facing, exterior side of the material at the hit.
    public float exterior_ior = 1.0;

    public __init() { }

    /// Returns the geometric normal in world-space such that it points towards the viewer.
    public float3 front_normal_ws() { return front_facing ? normal_ws : -normal_ws; }

    [mutating]
    public void adjust_shading_normal() { adjust_shading_normal(shading_frame_ws, normal_ws, wi_ws); }
};

interface IVolumeInteraction : IInteraction
{
};

// Spaces used by MaterialX transform nodes. The default provider treats them as
// identical, but scene integrations can override the extern provider to supply
// object/world/model transforms without changing generated material code.
public enum class GeomPropSpace : uint {
    model = 0,
    object = 1,
    world = 2,
};

static const float4x4 k_geomprop_identity4x4 = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

public float4x4 geomprop_identity4x4()
{
    return k_geomprop_identity4x4;
}

public interface IGeomPropProvider
{
    public static int get_int(SurfaceInteraction si, uint id);
    public static bool get_bool(SurfaceInteraction si, uint id);
    public static float get_float(SurfaceInteraction si, uint id);
    public static float2 get_float2(SurfaceInteraction si, uint id);
    public static float3 get_float3(SurfaceInteraction si, uint id);
    public static float4 get_float4(SurfaceInteraction si, uint id);
    public static float4x4 get_space_transform(SurfaceInteraction si, GeomPropSpace to_space, GeomPropSpace from_space);
};

public struct DummyGeomPropProvider : IGeomPropProvider
{
    public static int get_int(SurfaceInteraction si, uint id) { return 0; }
    public static bool get_bool(SurfaceInteraction si, uint id) { return false; }
    public static float get_float(SurfaceInteraction si, uint id) { return 0.0; }
    public static float2 get_float2(SurfaceInteraction si, uint id) { return si.uv; }
    public static float3 get_float3(SurfaceInteraction si, uint id) { return float3(si.uv, 0.0); }
    public static float4 get_float4(SurfaceInteraction si, uint id) { return float4(si.uv, 0.0, 0.0); }
    public static float4x4 get_space_transform(SurfaceInteraction si, GeomPropSpace to_space, GeomPropSpace from_space)
    {
        return geomprop_identity4x4();
    }
};

public extern struct GeomPropProvider : IGeomPropProvider = DummyGeomPropProvider;
#ifndef FALCOR_SHARED_HOST
#endif

namespace detail {

struct TextureHandleFieldsNonCUDA
{
    typedef BitField<uint, 0, 20> texture;
    typedef BitField<uint, 20, 10> sampler;
    typedef BitField<uint, 31, 1> udim;
};

struct TextureHandleFieldsCUDA
{
    typedef BitField<uint, 0, 30> texture;
    typedef BitField<uint, 0, 0> sampler;
    typedef BitField<uint, 31, 1> udim;
};

#ifndef FALCOR_SHARED_HOST
#ifdef __TARGET_CUDA__
typedef TextureHandleFieldsCUDA TextureHandleFields;
#else
typedef TextureHandleFieldsNonCUDA TextureHandleFields;
#endif
#endif

static constexpr uint INVALID_TEXTURE_HANDLE = 0xffffffff;

} // namespace detail


/// DescriptorHandle cannot be used in a dynamic dispatch context for now.
/// This wrapper is a workaround for that. Once Slang is fixed we can replace all the wrapped
/// handles and remove the `.unwrap()` call to turn them into actual descriptor handles.
/// https://github.com/shader-slang/slang/issues/9651
public struct DescriptorHandleWAR<T : IOpaqueDescriptor>
{
#ifdef __TARGET_CUDA__
    public uint64_t handle;
#else
    public uint2 handle;
#endif
    DescriptorHandle<T> unwrap() { return DescriptorHandle<T>(handle); }
};


public interface ILodSampler
{
    public __generic<T : ITexelElement> T sample(Texture2D<T> texture, SamplerState sampler, float2 uv);
    public __generic<T : ITexelElement> T sample(Texture3D<T> texture, SamplerState sampler, float3 uvw);
}

public struct ImplicitLodSampler : ILodSampler
{
    public __init() { }

    public __generic<T : ITexelElement> T sample(Texture2D<T> texture, SamplerState sampler, float2 uv)
    {
        return texture.Sample(sampler, uv);
    }

    public __generic<T : ITexelElement> T sample(Texture3D<T> texture, SamplerState sampler, float3 uvw)
    {
        return texture.Sample(sampler, uvw);
    }
}

public struct ExplicitLodSampler : ILodSampler
{
    public float lod = 0;

    public __init() { }
    public __init(float lod) { this.lod = lod; }

    public __generic<T : ITexelElement> T sample(Texture2D<T> texture, SamplerState sampler, float2 uv)
    {
        return texture.SampleLevel(sampler, uv, lod);
    }

    public __generic<T : ITexelElement> T sample(Texture3D<T> texture, SamplerState sampler, float3 uvw)
    {
        return texture.SampleLevel(sampler, uvw, lod);
    }
}

public extern static const bool FALCOR_TEXTURE_COORDINATE_ORIGIN_LOWER_LEFT = false;

public enum class TextureHandleFlags : uint {
    none = 0,
    udim = 0x1,
    uv_origin_correction = 0x2,
    all = 0x3,
};

// TODO(TCO-FUT-001): Use TextureHandleFlags directly once SlangPy treats enum-valued
// generic arguments as concrete value arguments rather than unknown type arguments.
// `uint(TextureHandleFlags::all)` is the desired spelling, but it currently fails
// in this generic default before SlangPy binding gets involved.
__generic<T : ITexelElement = float4, uint FlagsAsUint = 0x3 /*TextureHandleFlags::all*/> public struct TextureHandle
{
    public static const TextureHandleFlags Flags = TextureHandleFlags(FlagsAsUint);

    public static const bool ENABLE_UDIM = is_set(Flags, TextureHandleFlags::udim);
    public static const bool FLIP_V_COORDINATE
        = is_set(Flags, TextureHandleFlags::uv_origin_correction) && FALCOR_TEXTURE_COORDINATE_ORIGIN_LOWER_LEFT;

    public uint data = detail::INVALID_TEXTURE_HANDLE;

    public __init() { }

    public __init(uint data) { this.data = data; }

    public __generic<uint OtherFlagsAsUint> __init(TextureHandle<T, OtherFlagsAsUint> other) { this.data = other.data; }

    public property uint sampler_index
    {
        get {
            return detail::TextureHandleFields::sampler::extract(data);
        }
    }

    public property uint texture_index
    {
        get {
            return detail::TextureHandleFields::texture::extract(data);
        }
    }

    public bool is_valid() { return data != detail::INVALID_TEXTURE_HANDLE; }

    public bool is_udim() { return ENABLE_UDIM && (detail::TextureHandleFields::udim::extract(data) != 0); }

    public __generic<LodSampler : ILodSampler>
        T sample(LodSampler lod_sampler, float2 uv, const T default_value = T(T.Element(0)))
    {
        if (!is_valid())
            return default_value;
        This resolved_handle = resolve_udim(uv);
        if (!resolved_handle.is_valid())
            return default_value;
        if (FLIP_V_COORDINATE)
            uv.y = 1.0 - uv.y;
#ifdef __TARGET_CUDA__
        Texture2D<T> texture = DescriptorHandle<Texture2D<T>>(uint64_t(resolved_handle.texture_index));
        SamplerState sampler;
#else
        Texture2D<T> texture = DescriptorHandle<Texture2D<T>>(uint2(resolved_handle.texture_index, 0));
        SamplerState sampler = DescriptorHandle<SamplerState>(uint2(resolved_handle.sampler_index, 0));
#endif
        return lod_sampler.sample(texture, sampler, uv);
    }

    public __generic<LodSampler : ILodSampler>
        T sample(LodSampler lod_sampler, float3 uvw, const T default_value = T(T.Element(0)))
    {
        if (!is_valid())
            return default_value;
#ifdef __TARGET_CUDA__
        Texture3D<T> texture = DescriptorHandle<Texture3D<T>>(uint64_t(texture_index));
        SamplerState sampler;
#else
        Texture3D<T> texture = DescriptorHandle<Texture3D<T>>(uint2(texture_index, 0));
        SamplerState sampler = DescriptorHandle<SamplerState>(uint2(sampler_index, 0));
#endif
        return lod_sampler.sample(texture, sampler, uvw);
    }

    public T load(int2 coord, int miplevel = 0, const T default_value = T(T.Element(0)))
    {
        if (!is_valid() || is_udim())
            return default_value;
#ifdef __TARGET_CUDA__
        Texture2D<T> texture = DescriptorHandle<Texture2D<T>>(uint64_t(texture_index));
#else
        Texture2D<T> texture = DescriptorHandle<Texture2D<T>>(uint2(texture_index, 0));
#endif
        return texture.Load(int3(coord, miplevel));
    }

    public T load(int3 coord, int miplevel = 0, const T default_value = T(T.Element(0)))
    {
        if (!is_valid() || is_udim())
            return default_value;
#ifdef __TARGET_CUDA__
        Texture3D<T> texture = DescriptorHandle<Texture3D<T>>(uint64_t(texture_index));
#else
        Texture3D<T> texture = DescriptorHandle<Texture3D<T>>(uint2(texture_index, 0));
#endif
        return texture.Load(int4(coord, miplevel));
    }

    public bool get_dimensions(float2 uv, uint miplevel, out uint width, out uint height, out uint mip_levels)
    {
        width = {};
        height = {};
        mip_levels = {};

        if (!is_valid())
            return false;
        This resolved_handle = resolve_udim(uv);
        if (!resolved_handle.is_valid())
            return false;

        return get_dimensions(miplevel, width, height, mip_levels);
    }

    public bool get_dimensions(uint miplevel, out uint width, out uint height, out uint mip_levels)
    {
        width = {};
        height = {};
        mip_levels = {};

        if (!is_valid() || is_udim())
            return false;

#ifdef __TARGET_CUDA__
        Texture2D<T> texture = DescriptorHandle<Texture2D<T>>(uint64_t(texture_index));
#else
        Texture2D<T> texture = DescriptorHandle<Texture2D<T>>(uint2(texture_index, 0));
#endif
        texture.GetDimensions(miplevel, width, height, mip_levels);
        return true;
    }

    public bool get_dimensions(out uint width, out uint height, out uint depth)
    {
        width = {};
        height = {};
        depth = {};

        if (!is_valid())
            return false;

#ifdef __TARGET_CUDA__
        Texture3D<T> texture = DescriptorHandle<Texture3D<T>>(uint64_t(texture_index));
#else
        Texture3D<T> texture = DescriptorHandle<Texture3D<T>>(uint2(texture_index, 0));
#endif

        texture.GetDimensions(width, height, depth);

        return true;
    }

    public This resolve_udim(inout float2 uv)
    {
        if (ENABLE_UDIM && is_udim()) {
            int2 tile = int2(floor(uv));
#ifdef __TARGET_CUDA__
            Texture2D<uint> indirection_texture = DescriptorHandle<Texture2D<uint>>(uint64_t(texture_index));
#else
            Texture2D<uint> indirection_texture = DescriptorHandle<Texture2D<uint>>(uint2(texture_index, 0));
#endif
            int2 dim;
            indirection_texture.GetDimensions(dim.x, dim.y);
            if (any(tile < 0) || any(tile >= dim))
                return This();
            uv = frac(uv);
            return This(indirection_texture[tile]);
        } else {
            return this;
        }
    }
};

/// Evaluates the Fresnel term using Schlick's approximation.
/// Introduced in http://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
///
/// The Fresnel term equals f0 at normal incidence, and approaches f90=1.0 at 90 degrees.
/// The formulation below is generalized to allow both f0 and f90 to be specified.
///
/// @param f0 Specular reflectance at normal incidence (0 degrees).
/// @param f90 Reflectance at orthogonal incidence (90 degrees), which should be 1.0 for specular surface reflection.
/// @param cos_theta Cosine of angle between microfacet normal and incident direction (LdotH).
/// @return Fresnel term.
[Differentiable]
float3 eval_fresnel_schlick(float3 f0, float3 f90, float cos_theta)
{
    const float x = max(1.0 - cos_theta, 0.0); // Preserve the endpoint if cos_theta = 1+epsilon.
    const float x2 = x * x;
    const float x5 = x2 * x2 * x;
    return f0 + (f90 - f0) * x5;
}

[Differentiable]
float eval_fresnel_schlick(float f0, float f90, float cos_theta)
{
    const float x = max(1.0 - cos_theta, 0.0); // Preserve the endpoint if cos_theta = 1+epsilon.
    const float x2 = x * x;
    const float x5 = x2 * x2 * x;
    return f0 + (f90 - f0) * x5;
}

[Differentiable]
float3 eval_fresnel_generalized_schlick(float3 f0, float3 f90, float exponent, float cos_theta)
{
    return f0 + (f90 - f0) * pow(max(1.0 - cos_theta, 0.0), exponent); // Clamp to avoid NaN if cos_theta = 1+epsilon
}

/// Evaluates the Fresnel term using dieletric fresnel equations.
/// Based on http://www.pbr-book.org/3ed-2018/Reflection_Models/Specular_Reflection_and_Transmission.html
///
/// @param eta Relative index of refraction (etaI / etaT).
/// @param cos_theta_i Cosine of angle between normal and incident direction.
/// @param[out] cos_theta_t Cosine of angle between negative normal and transmitted direction (0 for total internal
/// reflection).
/// @return Returns Fr(eta, cos_theta_i).
[Differentiable]
float eval_fresnel_dielectric(float eta, float cos_theta_i, out float cos_theta_t)
{
    if (cos_theta_i < 0.0) {
        eta = 1.0 / eta;
        cos_theta_i = -cos_theta_i;
    }

    float sin_theta_t_sq = eta * eta * (1.0 - cos_theta_i * cos_theta_i);
    // Check for total internal reflection
    if (sin_theta_t_sq > 1.0) {
        cos_theta_t = 0.0;
        return 1.0;
    }

    cos_theta_t = sqrt(1.0 - sin_theta_t_sq); // No clamp needed

    // Note that at eta=1 and cos_theta_i=0, we get cos_theta_t=0 and NaN below.
    // It's important the framework clamps |cos_theta_i| or eta to small epsilon.
    float Rs = (eta * cos_theta_i - cos_theta_t) / (eta * cos_theta_i + cos_theta_t);
    float Rp = (eta * cos_theta_t - cos_theta_i) / (eta * cos_theta_t + cos_theta_i);

    return 0.5 * (Rs * Rs + Rp * Rp);
}

/// Evaluates the Fresnel term using dieletric fresnel equations.
/// Based on http://www.pbr-book.org/3ed-2018/Reflection_Models/Specular_Reflection_and_Transmission.html
///
/// @param eta Relative index of refraction (etaI / etaT).
/// @param cos_theta_i Cosine of angle between normal and incident direction.
/// @return Returns Fr(eta, cos_theta_i).
[Differentiable]
float eval_fresnel_dielectric(float eta, float cos_theta_i)
{
    float cos_theta_t;
    return eval_fresnel_dielectric(eta, cos_theta_i, cos_theta_t);
}

/// Evaluates the Fresnel term using conductor fresnel equations, assuming unpolarized light.
/// Base on "PHYSICALLY BASED LIGHTING CALCULATIONS FOR COMPUTER GRAPHICS" by Peter Shirley
/// http://www.cs.virginia.edu/~jdl/bib/globillum/shirley_thesis.pdf
///
/// @param eta Real part of complex index of refraction
/// @param k Imaginary part of complex index of refraction (the "absorption coefficient")
/// @param cos_theta_i Cosine of angle between normal and incident direction.
/// @return Returns conductor reflectance.
[Differentiable]
float eval_fresnel_conductor(float eta, float k, float cos_theta_i)
{
    float cos_theta_i_sq = cos_theta_i * cos_theta_i;
    float sin_theta_i_sq = max(1.0 - cos_theta_i_sq, 0.0);
    float sin_theta_i_qu = sin_theta_i_sq * sin_theta_i_sq;

    float inner_term = eta * eta - k * k - sin_theta_i_sq;
    float a_sq_plus_b_sq = sqrt(max(inner_term * inner_term + 4.0 * eta * eta * k * k, 0.0));
    float a = sqrt(max((a_sq_plus_b_sq + inner_term) * 0.5, 0.0));

    float Rs = ((a_sq_plus_b_sq + cos_theta_i_sq) - (2.0 * a * cos_theta_i))
        / ((a_sq_plus_b_sq + cos_theta_i_sq) + (2.0 * a * cos_theta_i));
    float Rp = ((cos_theta_i_sq * a_sq_plus_b_sq + sin_theta_i_qu) - (2.0 * a * cos_theta_i * sin_theta_i_sq))
        / ((cos_theta_i_sq * a_sq_plus_b_sq + sin_theta_i_qu) + (2.0 * a * cos_theta_i * sin_theta_i_sq));

    return 0.5 * (Rs + Rs * Rp);
}

/// Evaluates the Fresnel term using conductor fresnel equations, assuming unpolarized light.
/// Convenience function that takes coefficients at 3 wavelengths.
///
/// @param eta Real part of complex index of refraction
/// @param k Imaginary part of complex index of refraction (the "absorption coefficient")
/// @param cos_theta_i Cosine of angle between normal and incident direction.
/// @return Returns conductor reflectance.
[Differentiable]
float3 eval_fresnel_conductor(float3 eta, float3 k, float cos_theta_i)
{
    return float3(
        eval_fresnel_conductor(eta.x, k.x, cos_theta_i),
        eval_fresnel_conductor(eta.y, k.y, cos_theta_i),
        eval_fresnel_conductor(eta.z, k.z, cos_theta_i)
    );
}

/// Converts reflectance into IOR assuming normal incidence and n_i = 1.0
///
/// @param reflectance Reflectance at normal incidence
/// @return Returns index or refraction.
public float reflectance_to_ior(float3 reflectance)
{
    const float r = min(luminance(reflectance), 0.999);
    const float ior_i = 1.0;

    return (-ior_i - 2.0 * ior_i * sqrt(r) - ior_i * r) / (r - 1.0);
}

/// This class implements sampling and evaluation of an anisotropic GGX.
/// The class uses the visible distribution of normals for sampling.
public typealias AnisotropicGGX = AnisotropicGGXPolicy<1000, false>;

/// Policy-specialized implementation of AnisotropicGGX.
///
/// TSingularCutoffInverse controls the alpha threshold below which the lobe is
/// considered singular. The value is expressed as an inverse integer because
/// Slang generic value parameters do not currently support floats.
///
/// TClampBelowCutoff switches the policy from "treat below-threshold lobes as
/// singular" to "clamp alpha up to the threshold and keep evaluating finite
/// GGX". The AnisotropicGGX alias preserves the historical behavior, while
/// specific material systems can opt into finite near-delta evaluation when
/// their reference implementation does not use a singular branch.
public struct AnisotropicGGXPolicy<let TSingularCutoffInverse : int, let TClampBelowCutoff : bool> : IDifferentiable
{
    public float2 alpha = {};

    [Differentiable]
    public __init()
    {
        float singular_cutoff = 1.0 / float(TSingularCutoffInverse);
        this.alpha = TClampBelowCutoff ? float2(singular_cutoff) : float2(0.0);
    }

    [Differentiable]
    public __init(float2 alpha)
    {
        float singular_cutoff = 1.0 / float(TSingularCutoffInverse);
        bool singular = all(alpha < singular_cutoff);
        // If both axes are below the cutoff, the distribution is better represented by a delta lobe. If either axis
        // remains rough, keep a finite anisotropic GGX lobe and clamp both axes so its density never divides by a
        // near-zero roughness. Using the same cutoff for both decisions leaves one clear transition between the models.
        this.alpha = TClampBelowCutoff || !singular ? max(alpha, float2(singular_cutoff)) : alpha;
    }

    /// A 'singular' distribution is one which would be more ideally treated as an
    /// ideal mirror. It can't be sampled reliably.
    public bool is_singular() { return !TClampBelowCutoff && all(alpha < 1.0 / float(TSingularCutoffInverse)); }

    [Differentiable]
    public float isotropic_roughness() { return max(alpha.x, alpha.y); }

    /// Computes the Smith lambda function. This function is symmetric with respect to direction,
    /// i.e. smith_lambda(v) = smith_lambda(-v). The result returned is equivalent to assuming v.z > 0.
    [Differentiable]
    public float smith_lambda(const float3 v)
    {
        float cos_theta_sq = v.z * v.z;
        float sin_theta_sq = max(1.0 - cos_theta_sq, 0.0);
        float tan_theta_sq = sin_theta_sq / cos_theta_sq;
        if (isinf(tan_theta_sq))
            return float.INFINITY;

        float2 cos_sin_phi_sq = sin_theta_sq == 0.0 ? float2(1.0, 0.0) : float2(v.x * v.x, v.y * v.y) / sin_theta_sq;
        float alpha_sq = dot(cos_sin_phi_sq, alpha * alpha);
        return (sqrt(1.0 + alpha_sq * tan_theta_sq) - 1.0) * 0.5;
    }

    /// Compute the monostatic Smith shadowing function. This function returns the result
    /// assuming w.z > 0. This is what you want for single scattering, but you need to be careful
    /// for multiple scattering on microfacets.
    [Differentiable]
    public float G1(const float3 w) { return 1.0 / (1.0 + smith_lambda(w)); }

    /// Compute the bistatic Smith shadowing function for reflection.
    /// This will be off for refraction, where we would have to use the beta
    /// function instead. This is not worth the trouble unless we do
    /// multiple scattering on microfacet dielectrics as well.
    [Differentiable]
    public float G2(const float3 wo, const float3 wi) { return 1.0 / (1.0 + smith_lambda(wo) + smith_lambda(wi)); }

    /// Computes the normal distribution function, i.e. D(h)
    [Differentiable]
    public float eval_ndf(const float3 h)
    {
        float cos_theta_sq = h.z * h.z;
        float sin_theta_sq = max(1.0 - cos_theta_sq, 0.0);
        float tan_theta_sq = sin_theta_sq / cos_theta_sq;
        if (isinf(tan_theta_sq))
            return 0.0;

        float cos_theta_qu = cos_theta_sq * cos_theta_sq;
        if (cos_theta_qu < 1e-16)
            return 0.0;

        float2 cos_sin_phi_sq = sin_theta_sq == 0.0 ? float2(1.0, 0.0) : float2(h.x * h.x, h.y * h.y) / sin_theta_sq;
        float2 alpha_sq = cos_sin_phi_sq / (alpha * alpha);
        float e = tan_theta_sq * (alpha_sq.x + alpha_sq.y);
        return 1.0 / (float.PI* alpha.x* alpha.y* cos_theta_qu* sqr(1.0 + e));
    }

    /// Computes the distribution of visible normals, i.e. D_wi(h)
    [Differentiable]
    public float eval_vndf(const float3 w, const float3 h)
    {
        return w.z == 0.0 ? 0.0 : (G1(w) * abs(dot(w, h)) * eval_ndf(h)) / abs(w.z);
    }

    /// Samples the distribution of normals visible from direction w.
    public float3 sample<S : ISampleGenerator>(const float3 w, inout S sg)
    {
        float3 wh = normalize(float3(alpha.x * w.x, alpha.y * w.y, w.z));
        if (wh.z < 0.0)
            wh = -wh;

        float3 T1 = dot(wh.xy, wh.xy) > 0.0 ? normalize(float3(-wh.y, wh.x, 0.0)) : float3(1.0, 0.0, 0.0);
        float3 T2 = cross(wh, T1);

        float r = sqrt(next_float(sg));
        float phi = 2.0 * float.PI * next_float(sg);
        float t1 = r * cos(phi);
        float t2 = r * sin(phi);
        float s = 0.5 * (1.0 + wh.z);
        t2 = (1.0 - s) * sqrt(1.0 - t1 * t1) + s * t2;

        float3 nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * wh;
        return normalize(float3(alpha.x * nh.x, alpha.y * nh.y, max(0.0, nh.z)));
    }

    /// Returns the PDF of sampling normal h from VNDF(w)
    [Differentiable]
    float eval_pdf(float3 w, float3 h) { return eval_vndf(w, h); }
};
public enum MaterialInstanceHints : uint { none = 0, disable_normal_mapping = 0x1, adjust_shading_normal = 0x2 };
public struct MaterialSample
{
    public float3 wo_ws = {};
    public float pdf = {};
    public float3 weight = {};
    public BSDFFlags flags = BSDFFlags::none;
    public bool has_flag(BSDFFlags f) { return is_set(flags, f); }
}
}
using namespace render;
namespace mtlx {
using namespace utils;
using namespace render;
typealias IBSDF = render::IBSDF;
float luminance(float3 v)
{
    return utils::luminance(v);
}
float reflectance_to_ior(float3 v)
{
    return render::reflectance_to_ior(v);
}
}
//
// Isolated helpers for mtlx. This file intentionally does not import
// falcor2 or sgl; formulas are ported from OSL/BSDL when noted locally.
//
// Constants below are the BSDL counterparts from
// external/dbg_OSL/src/libbsdl/include/BSDL/config.h:40-49 unless noted.


namespace mtlx {

/// Reflects a local direction about the macronormal for the minimal
/// retroreflective microfacet model (Portsmouth et al., JCGT 15(1), 2026;
/// https://www.jcgt.org/published/0015/01/04/).
[ForceInline]
public float3 mx_retroreflect(const float3 w)
{
    return float3(-w.x, -w.y, w.z);
}

// BSDL: config.h:40 EPSILON = 1e-4f
public static constexpr float BSDL_EPSILON = 1e-4;
// BSDL: config.h:41 PI = float(M_PI)
public static constexpr float PI = 3.14159265358979323846;
// BSDL: config.h:42 ONEOVERPI = 1/PI
public static constexpr float INV_PI = 0.31830988618379067154;
// BSDL: config.h:45-46 FLOAT_MIN = 1.17549435e-38f (minimum normal float)
public static constexpr float FLOAT_MIN = 1.17549435e-38;
// BSDL: config.h:48 BIG = 1e12f
public static constexpr float BIG = 1e12;
// BSDL: config.h:49 PDF_MIN = 1e-6f
public static constexpr float PDF_MIN = 1e-6;
// BSDL: config.h:43-44 ALMOSTONE = 0.999999940395355224609375f (max float < 1)
public static constexpr float ALMOST_ONE = 0.999999940395355224609375;

public float mx_hmax(float2 value)
{
    return max(value.x, value.y);
}

public float mx_hmax(float3 value)
{
    return max(value.x, max(value.y, value.z));
}

public float mx_hmax(float4 value)
{
    return max(max(value.x, value.y), max(value.z, value.w));
}

} // namespace mtlx
//
// Runtime-indexed BSDL LUT payloads for mtlx. Payloads are owned and
// uploaded by the Python/C++ harness; this module only defines dimensions and
// buffer access.
//
// Dimensions match BSDL exactly:
//   - mini_microfacet_ggx_energy: 16 x 16, BSDL microfacet_tools_decl.h:67-69 (Nc x Nr x Nf=1)
//   - dielectric_{reflfront, bothfront, bothback}: 32 x 16 x 16, BSDL
//     bsdf_dielectric_decl.h:34-36 (Nf x Nr x Nc = 32 x 16 x 16)
//   - zeltner_sheen_ltc_param: 32 x 32 x 3, BSDL bsdf_sheen_decl.h:155 (ltc_res=32)
//     storing three coefficients (a_inv, b_inv, r_coeff) per (roughness, cos_NO) cell.


public static constexpr int MINI_MICROFACET_GGX_NC = 16;
public static constexpr int MINI_MICROFACET_GGX_NR = 16;
public static constexpr int MINI_MICROFACET_GGX_NF = 1;
public static constexpr int DIELECTRIC_NC = 16;
public static constexpr int DIELECTRIC_NR = 16;
public static constexpr int DIELECTRIC_NF = 32;
public static constexpr int ZELTNER_SHEEN_LTC_RES = 32;

namespace mtlx {

public struct MxLutGlobals
{
    public StructuredBuffer<float> mini_microfacet_ggx_energy;
    public StructuredBuffer<float> dielectric_reflfront_energy;
    public StructuredBuffer<float> dielectric_bothfront_energy;
    public StructuredBuffer<float> dielectric_bothback_energy;
    public StructuredBuffer<float> zeltner_sheen_ltc_param;
};

} // namespace mtlx

public mtlx::MxLutGlobals lut_globals;

public float mini_microfacet_ggx_energy(uint index)
{
    return lut_globals.mini_microfacet_ggx_energy[index];
}

public float dielectric_reflfront_energy(uint index)
{
    return lut_globals.dielectric_reflfront_energy[index];
}

public float dielectric_bothfront_energy(uint index)
{
    return lut_globals.dielectric_bothfront_energy[index];
}

public float dielectric_bothback_energy(uint index)
{
    return lut_globals.dielectric_bothback_energy[index];
}

public float3 zeltner_sheen_ltc_param(uint row, uint col)
{
    uint index = (row * uint(ZELTNER_SHEEN_LTC_RES) + col) * 3;
    return float3(
        lut_globals.zeltner_sheen_ltc_param[index],
        lut_globals.zeltner_sheen_ltc_param[index + 1],
        lut_globals.zeltner_sheen_ltc_param[index + 2]
    );
}
//
// Public Falcor-facing contract for active Mtlx BSDFs.
//
// Mtlx BSDF math is derived from BSDL/OSL sources, but active Slang code uses
// Falcor direction names throughout: `wi` is the incident/view direction and
// `wo` is the scattered/output direction. Source comments may point to BSDL
// formulas; in those cases the variables have already been translated to this
// convention.


namespace mtlx {

// OSL-derived MTLX generated graphs call sample_texture() for MaterialX
// image nodes. Keeping the wrapper in the profile module makes generated
// shaders smaller while preserving base-LOD sampling behavior through the
// supplied lod sampler.
public float4 sample_texture<TLodSampler : ILodSampler>(
    TextureHandle handle,
    float2 uv,
    TLodSampler lod_sampler,
    float4 default_value = {}
)
{
    return handle.sample(lod_sampler, uv, default_value);
}

public interface ILayerableBSDF : IBSDF
{
    // Slang 2026.14.1 miscompiles IBSDF default methods inherited through this
    // derived interface, so conformers repeat the two thin aliases explicitly.
    /// Directional scattering albedo for an incident/view direction in the
    /// local Falcor shading frame.
    public float3 directional_albedo(const float3 wi);

    /// Upper-layer opacity for an incident/view direction in the local Falcor
    /// shading frame. `1 - layer_opacity(wi)` is the share available to a
    /// lower MaterialX layer.
    public float3 layer_opacity(const float3 wi);
};

/// Internal BSDF sample primitive shared by Mtlx BSDF math.
///
/// `weight` is the BSDL-style eval/pdf sample weight. The bridge to Falcor
/// `IBSDF::eval()` multiplies it by `pdf` to return Falcor's f*cos value.
public struct MxBsdfSample
{
    public float3 wo;
    public float3 weight;
    public float pdf;
    public float roughness;

    public __init()
    {
        wo = float3(0.0);
        weight = float3(0.0);
        pdf = 0.0;
        roughness = 0.0;
    }

    public __init(float3 wo, float3 weight, float pdf, float roughness)
    {
        this.wo = wo;
        this.weight = weight;
        this.pdf = pdf;
        this.roughness = roughness;
    }

    [mutating]
    public void update(float3 ow, float opdf, float cpdf)
    {
        if (cpdf > PDF_MIN) {
            opdf *= cpdf;
            ow *= 1.0 / cpdf;
            if (opdf == pdf)
                weight = lerp(weight, ow, 0.5);
            else if (opdf < pdf)
                weight = lerp(ow, weight, 1.0 / (1.0 + opdf / pdf));
            else
                weight = lerp(weight, ow, 1.0 / (1.0 + pdf / opdf));
            pdf += opdf;
        }
    }
};

public MxBsdfSample
eval_diffuse_reflection_sample(const float3 wi, const float3 wo, const float3 weight, const float roughness)
{
    if (wi.z <= 0.0 || wo.z <= 0.0)
        return MxBsdfSample();
    return MxBsdfSample(wo, weight, wo.z * float.INV_PI, roughness);
}

} // namespace mtlx


namespace mtlx {

/// Per-bsdfs weights, result of the layering programs.
public struct MxWeights<let TBsdfCount : int>
{
    public float3 bsdf_weight[TBsdfCount];
    // Coverage is the aggregate amount of authored root closure represented by
    // bsdf_weight[]. The default is full coverage, preserving the historical
    // MxMaterialInstance normalization model. OSL-compatible complex_mix code
    // overrides this when final-root closure mass is intentionally below one.
    public float3 root_coverage;
    public __init()
    {
        for (int i = 0; i < TBsdfCount; ++i)
            bsdf_weight[i] = {};
        root_coverage = float3(1.0);
    }
}

public struct MxSubsurfaceData
{
    public float3 color;
    public float3 radius;
    public float3 N;
    public float3 T;
    public float3 w0;
    public float3 mean_cosine_0;
    public float3 mean_cosine_1;

    /// This allows using the same user input string to setup `DualHenyeyGreensteinPhaseFunction` and this class
    [[mutating]]
    public void dual_hg(float3 mean_cosine_0_value, float3 mean_cosine_1_value, float3 w0_)
    {
        mean_cosine_0 = mean_cosine_0_value;
        mean_cosine_1 = mean_cosine_1_value;
        w0 = w0_;
    }

    [[mutating]]
    public void single_hg(float mean_cosine_0_value)
    {
        mean_cosine_0 = mean_cosine_0_value;
        mean_cosine_1 = mean_cosine_0_value;
        w0 = 1.f;
    }
}

public interface IMaterialXMaterialData
{
    [ForceInline]
    int get_bsdf_count();
    [ForceInline]
    int get_layer_count();
    [ForceInline]
    IBSDF get_bsdf(int bsdfIndex);

    [ForceInline]
    float3 get_bsdf_weight(int bsdfIndex);
    [ForceInline]
    float3 get_layer_weight(int layerIndex);
    [ForceInline]
    float3 get_bsdf_n(int bsdfIndex);
    [ForceInline]
    float3 get_bsdf_t(int bsdfIndex);
    [ForceInline]
    float get_transmits(int bsdfIndex);
    [ForceInline]
    bool has_any_transmission();
    [ForceInline]
    bool get_is_front_facing();

    [ForceInline]
    bool has_subsurface_data();
    [ForceInline]
    MxSubsurfaceData get_subsurface_data();

    // These are from IBSDF, just doing static dispatch here
    [ForceInline]
    float3 eval<S : ISampleGenerator>(int bsdfIndex, const float3 wi, const float3 wo, inout S sg, BSDFContext bc);
    [ForceInline]
    BSDFEval
    eval_with_pdf<S : ISampleGenerator>(int bsdfIndex, const float3 wi, const float3 wo, inout S sg, BSDFContext bc);
    [ForceInline]
    bool
    sample<S : ISampleGenerator>(int bsdfIndex, const float3 wi, out BSDFSample sample, inout S sg, BSDFContext bc);
    [ForceInline]
    float eval_pdf(int bsdfIndex, const float3 wi, const float3 wo, BSDFContext bc);
    [ForceInline]
    AlbedoContributions eval_albedo(int bsdfIndex, const float3 wi);
    [ForceInline]
    RoughnessInformation get_roughness_information(int bsdfIndex, const float3 wi);
    [ForceInline]
    float3 get_ior_as_reflectance(int bsdfIndex);
}

// Additional functionf for the position-free layering
interface IMxPositionFreeLayeredMaterialData
{
    float3 get_sigma_a(int bsdfIndex);
    float3 get_sigma_s(int bsdfIndex);
    float get_mean_cosine(int bsdfIndex);

    int get_flattened_layer_stack_bsdf_index(int stackNr, int i);
    int get_flattened_layer_stack_count();
    int get_bsdf_stack_permutation_count(int i);
    int get_bsdf_stack_indices(int bsdfIndex, int i);
    int get_max_bsdfs_in_stack();
}

/// Interface to calculate weights. This is not intended to be dynamic dispatched ever.
/// There will be [specialize] keyword in the future to turn any dynamic dispatch into an error.
/// This is used inside MaterialXMaterialInstance.
/// The actual implementation depends on the shader graph and is generated as part of the MaterialX codegen.
/// (The actual code will be provided by Andrea's layering branch)
public interface IMxLayeredWeightCalculator
{
    public void
    calculate_weights<let TBsdfCount : int, let TLayerCount : int, TLayeredMaterialData : IMaterialXMaterialData>(
        const TLayeredMaterialData data,
        float3 wi_local[TBsdfCount],
        bool is_front_facing,
        out MxWeights<TBsdfCount> weights
    );
    public void
    calculate_plain_weights<let TBsdfCount : int, let TLayerCount : int, TLayeredMaterialData : IMaterialXMaterialData>(
        const TLayeredMaterialData data,
        out MxWeights<TBsdfCount> weights
    );
}

} // namespace mtlx


/// Keep these switches named so older MX layering behavior remains easy to compare.
#ifndef FIXED_NON_DELTA_MIXTURE_WEIGHT
#define FIXED_NON_DELTA_MIXTURE_WEIGHT 0
#endif
#ifndef FIXED_REFLECT_TRANSMIT_PDF_GATE
#define FIXED_REFLECT_TRANSMIT_PDF_GATE 1
#endif

namespace mtlx {

/// Homogeneous values associated with one runtime-selected MaterialX lobe.
struct MxSelectedLobePayload
{
    float3 wi_local;
    float3 weight;
    float selection;
    float3 weight_reverted;
    float3 ior_as_reflectance;
    float transmits;
    float3 bsdf_n;
    float3 bsdf_t;
}

/// Select all homogeneous lobe data in one compiler-generated branch ladder.
[ForceInline]
MxSelectedLobePayload select_mx_lobe_payload<let TBsdfCount : int, TMxMaterialData : IMaterialXMaterialData>(
    int sampled_lobe,
    const TMxMaterialData data,
    const float3 wi_local[TBsdfCount],
    const MxWeights<TBsdfCount> weight,
    const MxWeights<TBsdfCount> selection,
    const MxWeights<TBsdfCount> weight_reverted
)
{
    MxSelectedLobePayload result = {};
    [ForceUnroll]
    for (int i = 0; i < TBsdfCount; ++i) {
        if (sampled_lobe == i) {
            result.wi_local = wi_local[i];
            result.weight = weight.bsdf_weight[i];
            result.selection = selection.bsdf_weight[i].x;
            result.weight_reverted = weight_reverted.bsdf_weight[i];
            result.ior_as_reflectance = data.get_ior_as_reflectance(i);
            result.transmits = data.get_transmits(i);
            result.bsdf_n = data.get_bsdf_n(i);
            result.bsdf_t = data.get_bsdf_t(i);
            return result;
        }
    }
    return result;
}

/// Generic class to handle all layering combinations.
///
/// @tparam TBsdfCount Number of BSDFs, should always be greater than 0.
/// @tparam TLayerCount Number of layers, can be zero.
/// @tparam TWeightsCalc Interface to the generated code for computing layering weights, the layering program.
public struct MxMaterialInstance<
    let TBsdfCount : int,
    let TLayerCount : int,
    TLayeredMaterialData : IMaterialXMaterialData,
    TWeightsCalc : IMxLayeredWeightCalculator>
{
    public TLayeredMaterialData data;
    public TWeightsCalc calculator;
    public Frame shading_frame_ws;
    public float3 emissive_radiance = float3(0.0);
    public BSDFFlags lobe_types = BSDFFlags::all;
    public typealias Weights = MxWeights<TBsdfCount>;

    private Weights calculate_weights(float3 wi_local[TBsdfCount], bool is_front_facing)
    {
        Weights result = {};
        calculator
            .calculate_weights<TBsdfCount, TLayerCount, TLayeredMaterialData>(data, wi_local, is_front_facing, result);
        return result;
    }

    private Weights calculate_plain_weights()
    {
        Weights result = {};
        calculator.calculate_plain_weights<TBsdfCount, TLayerCount, TLayeredMaterialData>(data, result);
        return result;
    }

    public bool is_emissive() { return any(emissive_radiance > 0.0); }
    public float3 eval_emissive_radiance() { return emissive_radiance; }


    private BSDFEval eval_lobe_with_hints<let Hints : BSDFEvalHints, S : ISampleGenerator>(
        int i,
        const float3 wi,
        const float3 wo,
        inout S sg,
        const BSDFContext bc
    )
    {
        if (Hints == BSDFEvalHints::both)
            return data.eval_with_pdf(i, wi, wo, sg, bc);

        BSDFEval result = {};
        if (is_set(Hints, BSDFEvalHints::eval))
            result.value = data.eval(i, wi, wo, sg, bc);
        if (is_set(Hints, BSDFEvalHints::pdf))
            result.pdf = data.eval_pdf(i, wi, wo, bc);
        return result;
    }

    public BSDFEval eval_with_pdf<let Hints : BSDFEvalHints = BSDFEvalHints::both, S : ISampleGenerator>(
        const SurfaceInteraction si,
        const float3 wo_ws,
        inout S sg
    )
    {
        if (Hints == BSDFEvalHints::none)
            return BSDFEval();

        const bool needs_eval = is_set(Hints, BSDFEvalHints::eval);
        const bool needs_pdf = is_set(Hints, BSDFEvalHints::pdf);

        float3 wi_local[TBsdfCount];
        float3 wo_local[TBsdfCount];
        [ForceUnroll]
        for (int i = 0; i < TBsdfCount; i++) {
            const float3 bsdf_B = cross(data.get_bsdf_n(i), data.get_bsdf_t(i)); // * si.frame_ws.getHandednessSign();
            wi_local[i]
                = float3(dot(si.wi_ws, data.get_bsdf_t(i)), dot(si.wi_ws, bsdf_B), dot(si.wi_ws, data.get_bsdf_n(i)));
            wo_local[i] = float3(dot(wo_ws, data.get_bsdf_t(i)), dot(wo_ws, bsdf_B), dot(wo_ws, data.get_bsdf_n(i)));
        }

        Weights weight_zero_A;
        Weights weight_reverted;
        float3 norm_transmission = float3(0.f);

        Weights weight = calculate_weights(wi_local, data.get_is_front_facing());
        const bool has_transmission = data.has_any_transmission();

        Weights selection = {};
        if (needs_pdf) {
            [ForceUnroll]
            for (int i = 0; i < TBsdfCount; i++) {
                const AlbedoContributions a = data.eval_albedo(i, wi_local[i]);
                selection.bsdf_weight[i]
                    = luminance(weight.bsdf_weight[i] * (a.reflection + a.transmission * data.get_transmits(i)));
            }
        }

        float norm_selection = 0.f;
        if (needs_pdf) {
            for (int i = 0; i < TBsdfCount; i++)
                norm_selection += selection.bsdf_weight[i].x;
        }

        // the following computations are only necessary if the have any transmission and if we are interested in the
        // lower hemisphere
        // test hemisphere with geometric normal
        // disabled for now
        // const bool reflect = dot(si.faceN, si.frame_ws.normal) > 0.0f ? dot(si.V_ws, si.faceN) : dot(si.V_ws,
        // -si.faceN); if (!reflect)
        if (needs_eval && has_transmission) {
            // root_coverage separates missing root closure mass from real transparent/transmissive BSDF mass.
            float3 expected_transmission = weight.root_coverage;
            // TRANSMISSION TODO: figure out if we need t or t+at -> test with tinted simplebtdf
            [ForceUnroll]
            for (int i = 0; i < TBsdfCount; i++) {
                const AlbedoContributions a = data.eval_albedo(i, wi_local[i]);
                // we want to have r + ar + at
                expected_transmission -= (1.f - a.transmission) * weight.bsdf_weight[i];
            }

            // safety clamp - if the albedos are incorrect, we need this
            expected_transmission = clamp(expected_transmission, 0.f, 1.f);

            if (any(expected_transmission > 0.f)) {
                weight_reverted = calculate_weights(wi_local, !data.get_is_front_facing());
                weight_zero_A = calculate_plain_weights();

                [ForceUnroll]
                for (int i = 0; i < TBsdfCount; i++) {
                    if (data.get_transmits(i) != 0.f) {
                        // see sample() for explanation
                        const AlbedoContributions a = data.eval_albedo(i, wi_local[i]);

                        float3 real_transmission = select(
                            weight_zero_A.bsdf_weight[i] > 0.f,
                            (weight.bsdf_weight[i] * weight_reverted.bsdf_weight[i] * a.transmission)
                                / weight_zero_A.bsdf_weight[i],
                            0.f
                        );

                        norm_transmission
                            += select(expected_transmission > 0.f, real_transmission / expected_transmission, 0.f);
                    }
                }
            }
        }

        float3 eval = float3(0.f);
        float pdf = 0.f;
        [ForceUnroll]
        for (int i = 0; i < TBsdfCount; i++) {
            // only look at respective hemisphere; alternatiley we could let the BSDF split up the contribution which
            // needs a change in the interfaces
            const bool reflect = wi_local[i].z * wo_local[i].z > 0.0f;
            if (any(weight.bsdf_weight[i] > 0.f)) {
                const float ior = reflectance_to_ior(data.get_ior_as_reflectance(i));
                BSDFContext bc
                    = si.front_facing ? BSDFContext(si.exterior_ior, ior) : BSDFContext(ior, si.exterior_ior);
                if (data.get_transmits(i) <= 0.f)
                    bc.sample_lobe_types_hint = BSDFFlags::reflection | BSDFFlags::curve;
                if (reflect) {
                    BSDFEval bsdf_eval = eval_lobe_with_hints<Hints>(i, wi_local[i], wo_local[i], sg, bc);
                    if (needs_eval) {
                        let contrib = weight.bsdf_weight[i] * bsdf_eval.value;
                        eval += contrib;
                    }
                    if (needs_pdf && norm_selection > 0.f)
                        pdf += selection.bsdf_weight[i].x / norm_selection * bsdf_eval.pdf;
                } else if (has_transmission) {
                    BSDFEval bsdf_eval = eval_lobe_with_hints<Hints>(i, wi_local[i], wo_local[i], sg, bc);
                    if (needs_eval) {
                        let contrib = select(
                            (norm_transmission * weight_zero_A.bsdf_weight[i]) > 0.f,
                            weight.bsdf_weight[i] * weight_reverted.bsdf_weight[i] * data.get_transmits(i)
                                / (norm_transmission * weight_zero_A.bsdf_weight[i]) * bsdf_eval.value,
                            0.f
                        );
                        eval += contrib;
                    }
                    if (needs_pdf && norm_selection > 0.f)
                        pdf += selection.bsdf_weight[i].x / norm_selection * bsdf_eval.pdf * data.get_transmits(i);
                }
            }
        }

        if (needs_eval && (any(isnan(eval)) || any(isinf(eval))))
            eval = 0.f;
        if (needs_pdf && (isnan(pdf) || isinf(pdf)))
            pdf = 0.f;

        return BSDFEval(eval, pdf);
    }


    public Weights get_weights(const SurfaceInteraction si, bool is_front_facing)
    {
        float3 wi_local[TBsdfCount];
        [ForceUnroll]
        for (int i = 0; i < TBsdfCount; i++) {
            const float3 bsdf_B = cross(data.get_bsdf_n(i), data.get_bsdf_t(i)); // * si.frame.getHandednessSign();
            wi_local[i]
                = float3(dot(si.wi_ws, data.get_bsdf_t(i)), dot(si.wi_ws, bsdf_B), dot(si.wi_ws, data.get_bsdf_n(i)));
        }

        // weights top to bottom
        return calculate_weights(wi_local, data.get_is_front_facing());
    }


    public bool sample<S : ISampleGenerator>(const SurfaceInteraction si, inout S sg, out MaterialSample sample)
    {
        sample = {};
        float3 wi_local[TBsdfCount];
        [ForceUnroll]
        for (int i = 0; i < TBsdfCount; i++) {
            const float3 bsdf_B = cross(data.get_bsdf_n(i), data.get_bsdf_t(i)); // * si.frame.getHandednessSign();
            wi_local[i]
                = float3(dot(si.wi_ws, data.get_bsdf_t(i)), dot(si.wi_ws, bsdf_B), dot(si.wi_ws, data.get_bsdf_n(i)));
        }

        // weights top to bottom
        Weights weight = calculate_weights(wi_local, data.get_is_front_facing());
        const bool has_transmission = data.has_any_transmission();

        // bottom up for transmission
        Weights weight_reverted = {};
        if (has_transmission)
            weight_reverted = calculate_weights(wi_local, !data.get_is_front_facing());

        // PERFORMANCE TODO: split up into reflection and transmission albedo -> do not sample transmission if it will
        // be blocked PERFORMANCE TODO: re-use albedo all over the place calculate selection weights to pick BSDF - bias
        // with absorption
        Weights selection;
        [ForceUnroll]
        for (int i = 0; i < TBsdfCount; i++) {
            const AlbedoContributions a = data.eval_albedo(i, wi_local[i]);
            selection.bsdf_weight[i]
                = luminance(weight.bsdf_weight[i] * (a.reflection + a.transmission * data.get_transmits(i)));
        }

        //  normalize weights for rand selection
        float norm_selection = 0.f;
        for (int i = 0; i < TBsdfCount; i++)
            norm_selection += selection.bsdf_weight[i].x;

        if (norm_selection <= 0.f)
            return false;

        int sampled_lobe = 0;
        // No need to pay for selection on single lobe
        if (TBsdfCount > 1) {
            float sum = 0.f;
            const float rand = next_float(sg) * norm_selection;
            for (int i = 0; i < TBsdfCount - 1; ++i) {
                sum += selection.bsdf_weight[i].x;
                if (rand < sum) {
                    sampled_lobe = i;
                    break;
                }
                sampled_lobe = i + 1;
            }
        }

        Weights weight_zero_A = {};

        const MxSelectedLobePayload chosen
            = select_mx_lobe_payload<TBsdfCount>(sampled_lobe, data, wi_local, weight, selection, weight_reverted);

        // sample outgoing direction of selectd BSDF
        BSDFSample bsdf_sample;
        const float ior = reflectance_to_ior(chosen.ior_as_reflectance);
        BSDFContext bc = si.front_facing ? BSDFContext(si.exterior_ior, ior) : BSDFContext(ior, si.exterior_ior);
        // Restrict samples to not waste transmission on dielectric when on top of opaque and thus not transmitting.
        if (chosen.transmits <= 0.f)
            bc.sample_lobe_types_hint = BSDFFlags::reflection | BSDFFlags::curve;
        if (!data.sample(sampled_lobe, chosen.wi_local, bsdf_sample, sg, bc))
            return false;
        sample.pdf = bsdf_sample.pdf;
        sample.weight = bsdf_sample.weight;
        sample.flags = bsdf_sample.flags;
        if (!has_transmission && sample.has_flag(BSDFFlags::transmission))
            return false;

        // apply weights
        sample.weight *= chosen.weight * norm_selection / chosen.selection;

        const float3 bsdf_B = cross(chosen.bsdf_n, chosen.bsdf_t); // * si.frame.getHandednessSign();
        sample.wo_ws = chosen.bsdf_t * bsdf_sample.wo.x + bsdf_B * bsdf_sample.wo.y + chosen.bsdf_n * bsdf_sample.wo.z;

        float3 norm_transmission = float3(0.f);
        if (has_transmission && sample.has_flag(BSDFFlags::transmission)) {
            // Defer transmission-only work until sampling has succeeded.
            weight_zero_A = calculate_plain_weights();

            // root_coverage separates missing root closure mass from real transparent/transmissive BSDF mass.
            float3 expected_transmission = weight.root_coverage;
            [ForceUnroll]
            for (int i = 0; i < TBsdfCount; i++) {
                const AlbedoContributions a = data.eval_albedo(i, wi_local[i]);
                expected_transmission -= (1.f - a.transmission) * weight.bsdf_weight[i];
            }
            expected_transmission = clamp(expected_transmission, 0.f, 1.f);

            [ForceUnroll]
            for (int i = 0; i < TBsdfCount; i++) {
                if (data.get_transmits(i) > 0.f) {
                    const AlbedoContributions a = data.eval_albedo(i, wi_local[i]);
                    const float3 real_transmission = select(
                        weight_zero_A.bsdf_weight[i] > 0.f,
                        (weight.bsdf_weight[i] * weight_reverted.bsdf_weight[i] * a.transmission)
                            / weight_zero_A.bsdf_weight[i],
                        0.f
                    );
                    norm_transmission
                        += select(expected_transmission > 0.f, real_transmission / expected_transmission, 0.f);
                }
            }
            norm_transmission = select(norm_transmission < 1.f, 1.f, norm_transmission);
        }

        sample.pdf = 0.f;
        [ForceUnroll]
        for (int i = 0; i < TBsdfCount; i++) {
            if (has_transmission && sample.has_flag(BSDFFlags::transmission) && sampled_lobe == i) {
                // Weights are frontWeights * backWeights * transmits and normalized by all transmissions. Divide out
                // the second set of weights from the reverse traversal so it is not applied twice.
                sample.weight *= select(
                    norm_transmission * weight_zero_A.bsdf_weight[i] > 0.f,
                    chosen.weight_reverted * chosen.transmits / (norm_transmission * weight_zero_A.bsdf_weight[i]),
                    0.f
                );
            }
            if (!sample.has_flag(BSDFFlags::delta) && any(weight.bsdf_weight[i] > 0.f) && norm_selection > 0.f) {
                const float ior = reflectance_to_ior(data.get_ior_as_reflectance(i));
                BSDFContext bc
                    = si.front_facing ? BSDFContext(si.exterior_ior, ior) : BSDFContext(ior, si.exterior_ior);
                if (data.get_transmits(i) <= 0.f)
                    bc.sample_lobe_types_hint = BSDFFlags::reflection | BSDFFlags::curve;
                const float3 bsdf_B = cross(data.get_bsdf_n(i), data.get_bsdf_t(i));
                const float3 lobe_wo_local = float3(
                    dot(sample.wo_ws, data.get_bsdf_t(i)),
                    dot(sample.wo_ws, bsdf_B),
                    dot(sample.wo_ws, data.get_bsdf_n(i))
                );
                const float pdfBSDF = data.eval_pdf(i, wi_local[i], lobe_wo_local, bc);
                const bool reflect = wi_local[i].z * lobe_wo_local.z > 0.0f;
#if FIXED_REFLECT_TRANSMIT_PDF_GATE
                // Reflection and transmission are complementary for a concrete sampled direction. The original
                // code added both branches and can over-count transmissive leaves; this guarded path reports the
                // marginal PDF of the branch that can actually produce sample.wo_ws.
                sample.pdf
                    += selection.bsdf_weight[i].x / norm_selection * pdfBSDF * (reflect ? 1.0f : data.get_transmits(i));
#else
                // Original behavior: add the upper-hemisphere branch and the lower-hemisphere branch.
                sample.pdf += selection.bsdf_weight[i].x / norm_selection * pdfBSDF;
                sample.pdf += selection.bsdf_weight[i].x / norm_selection * pdfBSDF * data.get_transmits(i);
#endif
            }
        }

        if (isnan(sample.pdf) || isinf(sample.pdf))
            return false;
        if (!sample.has_flag(BSDFFlags::delta) && sample.pdf <= 0.f)
            return false;

        // Branch/lobe mixture sampling has two valid contracts. The original 1.38 contract keeps the sampled
        // branch throughput; it is unbiased when sample.pdf is the marginal direction PDF of the hidden branch
        // selection process. The guarded path below instead reports the collapsed full-mixture throughput for
        // non-delta samples, matching diagnostics and callers that expect sample.weight == eval() / sample.pdf.
#if FIXED_NON_DELTA_MIXTURE_WEIGHT
        if (!sample.has_flag(BSDFFlags::delta)) {
            if (sample.pdf <= 0.0f)
                return false;
            sample.weight = eval(si, sample.wo_ws, sg) / sample.pdf;
        }
#endif

        return true;
    }
}

} // namespace mtlx
//
// Thin-film Airy Fresnel helpers for Mtlx microfacet Fresnel policies.


namespace mtlx {

static const int TF_MODEL_DIELECTRIC = 0;
static const int TF_MODEL_CONDUCTOR = 1;
static const int TF_MODEL_SCHLICK = 2;
static const int TF_AIRY_FRESNEL_ITERATIONS = 2;

// Keep compatibility with the shared MaterialX 1.39 thin-film switch while
// making the Mtlx Fresnel implementation independent from that helper file.
#ifndef MTLX_THIN_FILM_USE_OPENPBR_PT_ENABLED
#ifdef MTLX_THIN_FILM_USE_OPENPBR_PT
#define MTLX_THIN_FILM_USE_OPENPBR_PT_ENABLED MTLX_THIN_FILM_USE_OPENPBR_PT
#else
#define MTLX_THIN_FILM_USE_OPENPBR_PT_ENABLED 0
#endif
#endif

static const float TF_OPENPBR_MIN_DENOM = 1e-9;
static const float3 TF_OPENPBR_RGB_WAVELENGTHS_NM = float3(620.0, 540.0, 450.0);

float3 tf_f0_to_ior(float3 f0)
{
    float3 sqrt_f0 = sqrt(clamp(f0, 0.01, 0.99));
    return (float3(1.0) + sqrt_f0) / (float3(1.0) - sqrt_f0);
}

float tf_fresnel_dielectric(float cos_theta, float ior)
{
    float c = cos_theta;
    float g2 = ior * ior + c * c - 1.0;
    if (g2 < 0.0)
        return 1.0;

    float g = sqrt(g2);
    float g_minus_c = g - c;
    float g_plus_c = g + c;
    float numerator = (g_plus_c * c) - 1.0;
    float denominator = (g_minus_c * c) + 1.0;
    return 0.5 * (g_minus_c * g_minus_c) / (g_plus_c * g_plus_c)
        * (1.0 + (numerator * numerator) / (denominator * denominator));
}

float2 tf_fresnel_dielectric_polarized(float cos_theta, float ior)
{
    float cos_theta2 = clamp(cos_theta, 0.0, 1.0);
    cos_theta2 *= cos_theta2;
    float sin_theta2 = 1.0 - cos_theta2;

    float t0 = max(ior * ior - sin_theta2, 0.0);
    float t1 = t0 + cos_theta2;
    float t2 = 2.0 * sqrt(t0) * cos_theta;
    float rs = (t1 - t2) / (t1 + t2);

    float t3 = cos_theta2 * t0 + sin_theta2 * sin_theta2;
    float t4 = t2 * sin_theta2;
    float rp = rs * (t3 - t4) / (t3 + t4);

    return float2(rp, rs);
}

void tf_fresnel_conductor_polarized(float cos_theta, float3 n, float3 k, out float3 rp, out float3 rs)
{
    float cos_theta2 = clamp(cos_theta, 0.0, 1.0);
    cos_theta2 *= cos_theta2;
    float sin_theta2 = 1.0 - cos_theta2;
    float3 n2 = n * n;
    float3 k2 = k * k;

    float3 t0 = n2 - k2 - float3(sin_theta2);
    float3 a2_plus_b2 = sqrt(t0 * t0 + 4.0 * n2 * k2);
    float3 t1 = a2_plus_b2 + float3(cos_theta2);
    float3 a = sqrt(max(0.5 * (a2_plus_b2 + t0), 0.0));
    float3 t2 = 2.0 * a * cos_theta;
    rs = (t1 - t2) / (t1 + t2);

    float3 t3 = cos_theta2 * a2_plus_b2 + float3(sin_theta2 * sin_theta2);
    float3 t4 = t2 * sin_theta2;
    rp = rs * (t3 - t4) / (t3 + t4);
}

float3 tf_fresnel_conductor(float cos_theta, float3 n, float3 k)
{
    float3 rp;
    float3 rs;
    tf_fresnel_conductor_polarized(cos_theta, n, k, rp, rs);
    return 0.5 * (rp + rs);
}

void tf_fresnel_conductor_phase_polarized(
    float cos_theta,
    float eta1,
    float3 eta2,
    float3 kappa2,
    out float3 phi_p,
    out float3 phi_s
)
{
    float3 k2 = kappa2 / eta2;
    float3 sin_theta_sq = float3(1.0 - cos_theta * cos_theta);
    float3 A = eta2 * eta2 * (float3(1.0) - k2 * k2) - eta1 * eta1 * sin_theta_sq;
    float3 B = sqrt(A * A + (2.0 * eta2 * eta2 * k2) * (2.0 * eta2 * eta2 * k2));
    float3 U = sqrt((A + B) * 0.5);
    float3 V = max(float3(0.0), sqrt((B - A) * 0.5));

    phi_s = float3(
        atan2(2.0 * eta1 * V.x * cos_theta, U.x * U.x + V.x * V.x - (eta1 * cos_theta) * (eta1 * cos_theta)),
        atan2(2.0 * eta1 * V.y * cos_theta, U.y * U.y + V.y * V.y - (eta1 * cos_theta) * (eta1 * cos_theta)),
        atan2(2.0 * eta1 * V.z * cos_theta, U.z * U.z + V.z * V.z - (eta1 * cos_theta) * (eta1 * cos_theta))
    );

    float3 numer = 2.0 * eta1 * eta2 * eta2 * cos_theta * (2.0 * k2 * U - (float3(1.0) - k2 * k2) * V);
    float3 denom
        = (eta2 * eta2 * (float3(1.0) + k2 * k2) * cos_theta) * (eta2 * eta2 * (float3(1.0) + k2 * k2) * cos_theta)
        - eta1 * eta1 * (U * U + V * V);
    phi_p = float3(atan2(numer.x, denom.x), atan2(numer.y, denom.y), atan2(numer.z, denom.z));
}

float3 tf_eval_sensitivity(float opd, float3 shift)
{
    float phase = float.TWO_PI * opd;
    float3 val = float3(5.4856e-13, 4.4201e-13, 5.2481e-13);
    float3 pos = float3(1.6810e+06, 1.7953e+06, 2.2084e+06);
    float3 var = float3(4.3278e+09, 9.3046e+09, 6.6121e+09);

    float3 xyz = float3(
        val.x * sqrt(float.TWO_PI* var.x) * cos(pos.x * phase + shift.x) * exp(-var.x * phase * phase),
        val.y * sqrt(float.TWO_PI* var.y) * cos(pos.y * phase + shift.y) * exp(-var.y * phase * phase),
        val.z * sqrt(float.TWO_PI* var.z) * cos(pos.z * phase + shift.z) * exp(-var.z * phase * phase)
    );
    xyz.x += 9.7470e-14 * sqrt(float.TWO_PI * 4.5282e+09) * cos(2.2399e+06 * phase + shift.x)
        * exp(-4.5282e+09 * phase * phase);
    return xyz / 1.0685e-7;
}

float3 tf_xyz_to_rgb(float3 xyz)
{
    return clamp(
        float3(
            2.3706743 * xyz.x - 0.9000405 * xyz.y - 0.4706338 * xyz.z,
            -0.5138850 * xyz.x + 1.4253036 * xyz.y + 0.0885814 * xyz.z,
            0.0052982 * xyz.x - 0.0146949 * xyz.y + 1.0093968 * xyz.z
        ),
        0.0,
        1.0
    );
}

float tf_safe_denom(float value)
{
    if (abs(value) >= TF_OPENPBR_MIN_DENOM)
        return value;
    return value >= 0.0 ? TF_OPENPBR_MIN_DENOM : -TF_OPENPBR_MIN_DENOM;
}

float2 tf_complex_mul(float2 a, float2 b)
{
    return float2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

float2 tf_complex_div(float2 a, float2 b, float2 fallback)
{
    float denom = b.x * b.x + b.y * b.y;
    if (denom < TF_OPENPBR_MIN_DENOM * TF_OPENPBR_MIN_DENOM)
        return fallback;
    return float2((a.x * b.x + a.y * b.y) / denom, (a.y * b.x - a.x * b.y) / denom);
}

float2 tf_complex_sqrt(float2 z)
{
    float m = sqrt(z.x * z.x + z.y * z.y);
    float imag_sign = copysign(1.0, z.y);
    return float2(sqrt(max(0.5 * (m + z.x), 0.0)), imag_sign * sqrt(max(0.5 * (m - z.x), 0.0)));
}

float2 tf_complex_exp_i(float angle)
{
    return float2(cos(angle), sin(angle));
}

float tf_complex_mag2(float2 z)
{
    return z.x * z.x + z.y * z.y;
}

float2 tf_openpbr_enforce_decaying_branch(float2 eta_t, float2 cos_theta_t)
{
    float2 eta_t_cos = tf_complex_mul(eta_t, cos_theta_t);
    return eta_t_cos.y < 0.0 ? -cos_theta_t : cos_theta_t;
}

float2 tf_openpbr_snell_cos_unified(float cos_theta_i, float eta_i, float2 eta_t)
{
    float sin_theta_i = sqrt(clamp(1.0 - cos_theta_i * cos_theta_i, 0.0, 1.0));
    float2 eta_i_over_eta_t = tf_complex_div(float2(eta_i, 0.0), eta_t, float2(1.0, 0.0));
    float2 sin_theta_t = sin_theta_i * eta_i_over_eta_t;
    float2 cos_theta_t_sq = float2(1.0, 0.0) - tf_complex_mul(sin_theta_t, sin_theta_t);
    return tf_openpbr_enforce_decaying_branch(eta_t, tf_complex_sqrt(cos_theta_t_sq));
}

void tf_openpbr_fresnel_unified_polarized_amplitude(
    float cos_theta_i,
    float eta_i,
    float2 eta_t,
    out float2 r_s,
    out float2 r_p,
    out float2 cos_theta_t
)
{
    cos_theta_t = tf_openpbr_snell_cos_unified(cos_theta_i, eta_i, eta_t);

    float2 eta_i_cos_theta_i = float2(eta_i * cos_theta_i, 0.0);
    float2 eta_t_cos_theta_t = tf_complex_mul(eta_t, cos_theta_t);
    float2 eta_t_cos_theta_i = cos_theta_i * eta_t;
    float2 eta_i_cos_theta_t = eta_i * cos_theta_t;

    r_s = tf_complex_div(
        eta_i_cos_theta_i - eta_t_cos_theta_t,
        eta_i_cos_theta_i + eta_t_cos_theta_t,
        float2(1.0, 0.0)
    );
    r_p = tf_complex_div(
        eta_t_cos_theta_i - eta_i_cos_theta_t,
        eta_t_cos_theta_i + eta_i_cos_theta_t,
        float2(1.0, 0.0)
    );
}

void tf_openpbr_fresnel_dielectric_polarized_amplitude(
    float cos_theta_i,
    float eta_i,
    float eta_t,
    out float r_s,
    out float r_p,
    out float t_s,
    out float t_p,
    out float cos_theta_t
)
{
    float sin_theta_i = sqrt(clamp(1.0 - cos_theta_i * cos_theta_i, 0.0, 1.0));
    float sin_theta_t = eta_i / eta_t * sin_theta_i;

    if (sin_theta_t >= 1.0) {
        r_s = 1.0;
        r_p = 1.0;
        t_s = 0.0;
        t_p = 0.0;
        cos_theta_t = 0.0;
        return;
    }

    cos_theta_t = sqrt(clamp(1.0 - sin_theta_t * sin_theta_t, 0.0, 1.0));

    float eta_i_cos_theta_i = eta_i * cos_theta_i;
    float eta_t_cos_theta_t = eta_t * cos_theta_t;
    float eta_t_cos_theta_i = eta_t * cos_theta_i;
    float eta_i_cos_theta_t = eta_i * cos_theta_t;

    float denom_s = tf_safe_denom(eta_i_cos_theta_i + eta_t_cos_theta_t);
    float denom_p = tf_safe_denom(eta_t_cos_theta_i + eta_i_cos_theta_t);
    r_s = (eta_i_cos_theta_i - eta_t_cos_theta_t) / denom_s;
    r_p = (eta_t_cos_theta_i - eta_i_cos_theta_t) / denom_p;
    t_s = (2.0 * eta_i_cos_theta_i) / denom_s;
    t_p = (2.0 * eta_i_cos_theta_i) / denom_p;
}

float tf_openpbr_airy_reflectance(float r12, float t12, float r21, float t21, float2 r23, float delta_phi)
{
    float2 phase = tf_complex_exp_i(delta_phi);
    float2 numerator = (t12 * t21) * tf_complex_mul(r23, phase);
    float2 denominator = float2(1.0, 0.0) - r21 * tf_complex_mul(r23, phase);
    float2 total = float2(r12, 0.0) + tf_complex_div(numerator, denominator, float2(0.0));
    return tf_complex_mag2(total);
}

float tf_openpbr_channel_reflectance(
    float cos_theta_i,
    float eta_exterior,
    float eta_film,
    float eta_base,
    float kappa_base,
    float thinfilm_thickness_nm,
    float wavelength_nm
)
{
    float r12_s;
    float r12_p;
    float t12_s;
    float t12_p;
    float cos_theta_t_film;
    tf_openpbr_fresnel_dielectric_polarized_amplitude(
        cos_theta_i,
        eta_exterior,
        eta_film,
        r12_s,
        r12_p,
        t12_s,
        t12_p,
        cos_theta_t_film
    );

    if (cos_theta_t_film <= 0.0)
        return 1.0;

    float r21_s = -r12_s;
    float r21_p = -r12_p;
    float safe_cos_theta_i = max(cos_theta_i, TF_OPENPBR_MIN_DENOM);
    float t21_scale = (eta_film * cos_theta_t_film) / (eta_exterior * safe_cos_theta_i);
    float t21_s = t12_s * t21_scale;
    float t21_p = t12_p * t21_scale;

    float2 r23_s;
    float2 r23_p;
    float2 cos_theta_t_base_unused;
    tf_openpbr_fresnel_unified_polarized_amplitude(
        cos_theta_t_film,
        eta_film,
        float2(eta_base, kappa_base),
        r23_s,
        r23_p,
        cos_theta_t_base_unused
    );

    float opd = 2.0 * eta_film * thinfilm_thickness_nm * cos_theta_t_film;
    float delta_phi = float.TWO_PI * opd / wavelength_nm;
    float reflectance_s = tf_openpbr_airy_reflectance(r12_s, t12_s, r21_s, t21_s, r23_s, delta_phi);
    float reflectance_p = tf_openpbr_airy_reflectance(r12_p, t12_p, r21_p, t21_p, r23_p, delta_phi);
    return 0.5 * (reflectance_s + reflectance_p);
}

float3 tf_fresnel_airy_openpbr_pt(
    float cos_theta,
    float thinfilm_thickness_nm,
    float thinfilm_ior,
    float3 eta_base,
    float3 kappa_base
)
{
    float cos_theta_i = clamp(cos_theta, 0.0, 1.0);
    float eta_exterior = 1.0;
    float eta_film = max(thinfilm_ior, eta_exterior);
    float3 eta_base_safe = max(eta_base, float3(TF_OPENPBR_MIN_DENOM));

    return clamp(
        float3(
            tf_openpbr_channel_reflectance(
                cos_theta_i,
                eta_exterior,
                eta_film,
                eta_base_safe.x,
                max(kappa_base.x, 0.0),
                thinfilm_thickness_nm,
                TF_OPENPBR_RGB_WAVELENGTHS_NM.x
            ),
            tf_openpbr_channel_reflectance(
                cos_theta_i,
                eta_exterior,
                eta_film,
                eta_base_safe.y,
                max(kappa_base.y, 0.0),
                thinfilm_thickness_nm,
                TF_OPENPBR_RGB_WAVELENGTHS_NM.y
            ),
            tf_openpbr_channel_reflectance(
                cos_theta_i,
                eta_exterior,
                eta_film,
                eta_base_safe.z,
                max(kappa_base.z, 0.0),
                thinfilm_thickness_nm,
                TF_OPENPBR_RGB_WAVELENGTHS_NM.z
            )
        ),
        0.0,
        1.0
    );
}

public struct MxThinFilmFresnelData<let MODEL : int>
{
    public bool airy;
    public float3 ior;
    public float3 extinction;
    public float3 f0;
    public float3 f82;
    public float3 f90;
    public float exponent;
    public float thinfilm_thickness;
    public float thinfilm_ior;

    public __init() { }

    public __init(float ior, float thinfilm_thickness, float thinfilm_ior)
    {
        this.airy = thinfilm_thickness > 0.0;
        this.ior = float3(ior);
        this.extinction = float3(0.0);
        this.f0 = float3(0.0);
        this.f82 = float3(0.0);
        this.f90 = float3(0.0);
        this.exponent = 0.0;
        this.thinfilm_thickness = thinfilm_thickness;
        this.thinfilm_ior = thinfilm_ior;
    }

    public __init(float3 ior, float3 extinction, float thinfilm_thickness, float thinfilm_ior)
    {
        this.airy = thinfilm_thickness > 0.0;
        this.ior = ior;
        this.extinction = extinction;
        this.f0 = float3(0.0);
        this.f82 = float3(0.0);
        this.f90 = float3(0.0);
        this.exponent = 0.0;
        this.thinfilm_thickness = thinfilm_thickness;
        this.thinfilm_ior = thinfilm_ior;
    }

    public __init(float3 f0, float3 f82, float3 f90, float exponent, float thinfilm_thickness, float thinfilm_ior)
    {
        this.airy = thinfilm_thickness > 0.0;
        this.ior = float3(0.0);
        this.extinction = float3(0.0);
        this.f0 = f0;
        this.f82 = f82;
        this.f90 = f90;
        this.exponent = exponent;
        this.thinfilm_thickness = thinfilm_thickness;
        this.thinfilm_ior = thinfilm_ior;
    }

    public float3 compute_fresnel(float cos_theta)
    {
        if (airy)
            return eval_fresnel_airy(cos_theta);
        if (MODEL == TF_MODEL_DIELECTRIC)
            return float3(tf_fresnel_dielectric(cos_theta, ior.x));
        if (MODEL == TF_MODEL_CONDUCTOR)
            return tf_fresnel_conductor(cos_theta, ior, extinction);
        return eval_fresnel_hoffman_schlick(cos_theta);
    }

    public float3 directional_albedo(float cos_theta, float alpha)
    {
        if (airy) {
            float3 mirror_dir_albedo = compute_fresnel(cos_theta);
            float3 F0 = eval_fresnel_airy(1.0);
            float3 rough_dir_albedo = ggx_directional_albedo_analytic(cos_theta, alpha, F0, float3(1.0));
            return lerp(mirror_dir_albedo, rough_dir_albedo, sqrt(alpha));
        }

        if (MODEL == TF_MODEL_DIELECTRIC) {
            float F0 = ((ior.x - 1.0) / (ior.x + 1.0));
            F0 *= F0;
            return ggx_directional_albedo_analytic(cos_theta, alpha, float3(F0), float3(1.0));
        }

        if (MODEL == TF_MODEL_CONDUCTOR) {
            float3 F0 = tf_fresnel_conductor(1.0, ior, extinction);
            return ggx_directional_albedo_analytic(cos_theta, alpha, F0, float3(1.0));
        }

        return ggx_directional_albedo_analytic(cos_theta, alpha, f0, f90);
    }

    float3 eval_fresnel_hoffman_schlick(float cos_theta)
    {
        const float cos_theta_max = 1.0 / 7.0;
        const float cos_theta_factor = 1.0 / (cos_theta_max * pow(1.0 - cos_theta_max, 6.0));

        float x = clamp(cos_theta, 0.0, 1.0);
        float3 a = lerp(f0, f90, pow(1.0 - cos_theta_max, exponent)) * (float3(1.0) - f82) * cos_theta_factor;
        float one_minus_x = 1.0 - x;
        float one_minus_x6 = one_minus_x * one_minus_x * one_minus_x * one_minus_x * one_minus_x * one_minus_x;
        return lerp(f0, f90, pow(one_minus_x, exponent)) - a * x * one_minus_x6;
    }

    float3 eval_fresnel_airy(float cos_theta)
    {
#if MTLX_THIN_FILM_USE_OPENPBR_PT_ENABLED
        float3 eta3 = (MODEL == TF_MODEL_SCHLICK) ? tf_f0_to_ior(f0) : ior;
        float3 kappa3 = (MODEL == TF_MODEL_CONDUCTOR) ? extinction : float3(0.0);
        return tf_fresnel_airy_openpbr_pt(cos_theta, thinfilm_thickness, thinfilm_ior, eta3, kappa3);
#else
        return eval_fresnel_airy_materialx(cos_theta);
#endif
    }

    float3 eval_fresnel_airy_materialx(float cos_theta)
    {
        float eta1 = 1.0;
        float eta2 = max(thinfilm_ior, eta1);
        float3 eta3 = (MODEL == TF_MODEL_SCHLICK) ? tf_f0_to_ior(f0) : ior;
        float3 kappa3 = (MODEL == TF_MODEL_SCHLICK) ? float3(0.0) : extinction;
        float eta1_over_eta2 = eta1 / eta2;
        float cos_theta_t = sqrt(max(1.0 - (1.0 - cos_theta * cos_theta) * (eta1_over_eta2 * eta1_over_eta2), 0.0));

        float2 R12 = tf_fresnel_dielectric_polarized(cos_theta, eta2 / eta1);
        if (cos_theta_t <= 0.0)
            R12 = float2(1.0);
        float2 T121 = float2(1.0) - R12;

        float3 R23p;
        float3 R23s;
        if (MODEL == TF_MODEL_SCHLICK) {
            float3 f = eval_fresnel_hoffman_schlick(cos_theta_t);
            R23p = 0.5 * f;
            R23s = 0.5 * f;
        } else {
            tf_fresnel_conductor_polarized(cos_theta_t, eta3 / eta2, kappa3 / eta2, R23p, R23s);
        }

        float cos_B = rsqrt(1.0 + (eta2 / eta1) * (eta2 / eta1));
        float2 phi21 = float2(cos_theta < cos_B ? 0.0 : float.PI, float.PI);
        float3 phi23p;
        float3 phi23s;
        if (MODEL == TF_MODEL_SCHLICK) {
            phi23p = float3(
                eta3.x < eta2 ? float.PI : 0.0,
                eta3.y < eta2 ? float.PI : 0.0,
                eta3.z < eta2 ? float.PI : 0.0
            );
            phi23s = phi23p;
        } else {
            tf_fresnel_conductor_phase_polarized(cos_theta_t, eta2, eta3, kappa3, phi23p, phi23s);
        }

        float3 r123p = max(sqrt(R12.x * R23p), 0.0);
        float3 r123s = max(sqrt(R12.y * R23s), 0.0);

        float3 I = float3(0.0);
        float dist_meters = thinfilm_thickness * 1.0e-9;
        float opd = 2.0 * eta2 * cos_theta_t * dist_meters;

        float3 Rs = ((T121.x * T121.x) * R23p) / (float3(1.0) - R12.x * R23p);
        I += R12.x + Rs;

        float3 Cm = Rs - T121.x;
        for (int m = 1; m <= TF_AIRY_FRESNEL_ITERATIONS; ++m) {
            Cm *= r123p;
            float3 Sm = 2.0 * tf_eval_sensitivity(float(m) * opd, float(m) * (phi23p + float3(phi21.x)));
            I += Cm * Sm;
        }

        float3 Rp = ((T121.y * T121.y) * R23s) / (float3(1.0) - R12.y * R23s);
        I += R12.y + Rp;

        Cm = Rp - T121.y;
        for (int m = 1; m <= TF_AIRY_FRESNEL_ITERATIONS; ++m) {
            Cm *= r123s;
            float3 Sm = 2.0 * tf_eval_sensitivity(float(m) * opd, float(m) * (phi23s + float3(phi21.y)));
            I += Cm * Sm;
        }

        I *= 0.5;
        return tf_xyz_to_rgb(I);
    }
};

public typealias MxThinFilmDielectricFresnelData = MxThinFilmFresnelData<TF_MODEL_DIELECTRIC>;
public typealias MxThinFilmConductorFresnelData = MxThinFilmFresnelData<TF_MODEL_CONDUCTOR>;
public typealias MxThinFilmSchlickFresnelData = MxThinFilmFresnelData<TF_MODEL_SCHLICK>;

} // namespace mtlx
//
// Fresnel policies for Mtlx microfacet BSDFs.


namespace mtlx {

public float oriented_dielectric_eta(float eta_value, bool backside)
{
    if (backside)
        eta_value = 1.0 / eta_value;
    return eta_value >= 1.0 ? clamp(eta_value, 1.001, 5.0) : clamp(eta_value, 1.0 / 5.0, 1.0 / 1.001);
}

public float dielectric_base_ior(float eta)
{
    return clamp(eta < 1.0 ? 1.0 / eta : eta, 1.001, 5.0);
}

public float dielectric_tir_cos(float eta)
{
    return eta >= 1.0 ? 0.0 : sqrt(1.0 - eta * eta);
}

// BSDL: bsdf_dielectric_impl.h:21-64 DielectricFresnel
[ForceInline]
public float dielectric_fresnel_eval(float eta, float c)
{
    c = clamp(c, 0.0, 1.0);
    float g = (eta - 1.0) * (eta + 1.0) + c * c;
    if (g > 0.0) {
        g = sqrt(g);
        float A = (g - c) / (g + c);
        float B = (c * (g + c) - 1.0) / (c * (g - c) + 1.0);
        return 0.5 * A * A * (1.0 + B * B);
    }
    return 1.0;
}

public float3 schlick_fresnel(float3 F0, float3 F90, float exponent, float c)
{
    return lerp(F0, F90, pow(1.0 - c, exponent));
}

public float3 schlick_color82_fresnel(float3 F0, float3 F82, float3 F90, float exponent, float c)
{
    const float cos_theta_max = 1.0 / 7.0;
    const float cos_theta_factor = 1.0 / (cos_theta_max * pow(1.0 - cos_theta_max, 6.0));
    float one_minus_max = 1.0 - cos_theta_max;
    float one_minus_c = 1.0 - c;
    float one_minus_c6 = one_minus_c * one_minus_c * one_minus_c * one_minus_c * one_minus_c * one_minus_c;
    float3 a = schlick_fresnel(F0, F90, exponent, cos_theta_max) * (float3(1.0) - F82) * cos_theta_factor;
    return schlick_fresnel(F0, F90, exponent, c) - a * c * one_minus_c6;
}

public interface IMxFresnelPolicy
{
    float3 eval(float cos_theta);
}

public interface IMxReflectiveFresnelPolicy : IMxFresnelPolicy
{
    float3 directional_albedo(float cos_theta, float roughness);
}

public interface IMxRefractiveFresnelPolicy : IMxFresnelPolicy
{
    float refraction_eta();
}

public struct MxConductorFresnel : IMxReflectiveFresnelPolicy
{
    public float3 IOR;
    public float3 extinction;

    public __init(float3 IOR, float3 extinction)
    {
        this.IOR = IOR;
        this.extinction = extinction;
    }

    public __init()
    {
        IOR = float3(1.0);
        extinction = float3(0.0);
    }

    // BSDL: bsdf_conductor_impl.h:19-42 ConductorFresnel::eval
    [ForceInline]
    public float3 eval(float cos_theta)
    {
        cos_theta = clamp(cos_theta, 0.0, 1.0);
        const float3 one = float3(1.0);
        const float3 cos2_theta = float3(cos_theta * cos_theta);
        const float3 sin2_theta = one - cos2_theta;
        const float3 n = IOR;
        const float3 k = extinction;
        const float3 n2 = n * n;
        const float3 k2 = k * k;
        const float3 t0 = n2 - k2 - sin2_theta;
        const float3 a2plusb2 = sqrt(t0 * t0 + 4.0 * n2 * k2);
        const float3 t1 = a2plusb2 + cos2_theta;
        const float3 a = sqrt(0.5 * (a2plusb2 + t0));
        const float3 t2 = (2.0 * cos_theta) * a;
        const float3 rs = (t1 - t2) / clamp(t1 + t2, float.MIN, BIG);
        const float3 t3 = cos2_theta * a2plusb2 + sin2_theta * sin2_theta;
        const float3 t4 = t2 * sin2_theta;
        const float3 rp = rs * (t3 - t4) / clamp(t3 + t4, float.MIN, BIG);
        return 0.5 * clamp(rp + rs, 0.0, 2.0);
    }

    // BSDL: bsdf_conductor_impl.h:62-76 ConductorFresnel::avg
    public float3 avg()
    {
        const float a = -0.32775145;
        const float b = 0.18346033;
        const float c = 0.61146583;
        const float d = -0.07785134;
        const float3 p = a + b * IOR + c * extinction + d * IOR * extinction;
        return clamp(p / (float3(1.0) + p), 0.0, 1.0);
    }

    [ForceInline]
    public float3 directional_albedo(float cos_theta, float roughness) { return avg(); }
};

public struct MxConductorAiryFresnel : IMxReflectiveFresnelPolicy
{
    public float3 IOR;
    public float3 extinction;
    public float thinfilm_thickness;
    public float thinfilm_ior;

    public __init(float3 IOR, float3 extinction, float thinfilm_thickness, float thinfilm_ior)
    {
        this.IOR = IOR;
        this.extinction = extinction;
        this.thinfilm_thickness = thinfilm_thickness;
        this.thinfilm_ior = thinfilm_ior;
    }

    public __init()
    {
        IOR = float3(1.0);
        extinction = float3(0.0);
        thinfilm_thickness = 0.0;
        thinfilm_ior = 1.5;
    }

    [ForceInline]
    public float3 eval(float cos_theta)
    {
        if (thinfilm_thickness <= 0.0)
            return MxConductorFresnel(IOR, extinction).eval(cos_theta);
        MxThinFilmConductorFresnelData fd
            = MxThinFilmConductorFresnelData(IOR, extinction, thinfilm_thickness, thinfilm_ior);
        return fd.compute_fresnel(clamp(cos_theta, 0.0, 1.0));
    }

    [ForceInline]
    public float3 directional_albedo(float cos_theta, float roughness)
    {
        if (thinfilm_thickness <= 0.0)
            return MxConductorFresnel(IOR, extinction).directional_albedo(cos_theta, roughness);
        MxThinFilmConductorFresnelData fd
            = MxThinFilmConductorFresnelData(IOR, extinction, thinfilm_thickness, thinfilm_ior);
        return fd.directional_albedo(clamp(cos_theta, 0.0, 1.0), roughness);
    }
};

public struct MxDielectricFresnel : IMxRefractiveFresnelPolicy
{
    public float eta;

    public __init(float eta_value, bool backside) { eta = oriented_dielectric_eta(eta_value, backside); }

    public __init() { eta = 1.5; }

    [ForceInline]
    public float eval_scalar(float c) { return dielectric_fresnel_eval(eta, c); }

    [ForceInline]
    public float3 eval(float c) { return float3(eval_scalar(c)); }

    [ForceInline]
    public float refraction_eta() { return eta; }
};

public struct MxDielectricAiryFresnel : IMxRefractiveFresnelPolicy
{
    public float eta;
    public float base_ior;
    public float tir_cos;
    public float thinfilm_thickness;
    public float thinfilm_ior;

    public __init(float eta_value, bool backside, float thinfilm_thickness, float thinfilm_ior)
    {
        eta = oriented_dielectric_eta(eta_value, backside);
        base_ior = dielectric_base_ior(eta);
        tir_cos = dielectric_tir_cos(eta);
        this.thinfilm_thickness = thinfilm_thickness;
        this.thinfilm_ior = thinfilm_ior;
    }

    public __init()
    {
        eta = 1.5;
        base_ior = 1.5;
        tir_cos = 0.0;
        thinfilm_thickness = 0.0;
        thinfilm_ior = 1.5;
    }

    [ForceInline]
    public float3 eval(float c)
    {
        c = clamp(c, 0.0, 1.0);
        if (c < tir_cos)
            return float3(1.0);
        if (thinfilm_thickness <= 0.0)
            return float3(dielectric_fresnel_eval(eta, c));

        MxThinFilmDielectricFresnelData fd
            = MxThinFilmDielectricFresnelData(base_ior, thinfilm_thickness, thinfilm_ior);
        return fd.compute_fresnel(c);
    }

    [ForceInline]
    public float refraction_eta() { return eta; }
};

// BSDL: bsdf_schlick_impl.h:16-35 SchlickFresnel (extends DielectricFresnel)
public struct MxSchlickFresnel : IMxRefractiveFresnelPolicy
{
    public float3 F0;
    public float3 F90;
    public float exponent;
    public float eta;
    public float tir_cos;

    public __init(float3 F0, float3 F90, float exponent, float eta_value, bool backfacing)
    {
        eta = oriented_dielectric_eta(eta_value, backfacing);
        this.F0 = clamp(F0, 0.0, 1.0);
        this.F90 = clamp(F90, 0.0, 1.0);
        this.exponent = exponent;
        tir_cos = dielectric_tir_cos(eta);
    }

    public __init()
    {
        F0 = float3(0.04);
        F90 = float3(1.0);
        exponent = 5.0;
        eta = 1.5;
        tir_cos = 0.0;
    }

    [ForceInline]
    public float3 eval(float c)
    {
        c = clamp(c, 0.0, 1.0);
        if (c < tir_cos)
            return float3(1.0);
        return schlick_fresnel(F0, F90, exponent, c);
    }

    [ForceInline]
    public float refraction_eta() { return eta; }
};

public struct MxSchlickColor82Fresnel : IMxRefractiveFresnelPolicy
{
    public float3 F0;
    public float3 F82;
    public float3 F90;
    public float exponent;
    public float eta;
    public float tir_cos;

    public __init(float3 F0, float3 F82, float3 F90, float exponent, float eta_value, bool backfacing)
    {
        eta = oriented_dielectric_eta(eta_value, backfacing);
        this.F0 = clamp(F0, 0.0, 1.0);
        this.F82 = max(F82, float3(0.0));
        this.F90 = clamp(F90, 0.0, 1.0);
        this.exponent = exponent;
        tir_cos = dielectric_tir_cos(eta);
    }

    public __init()
    {
        F0 = float3(0.04);
        F82 = float3(1.0);
        F90 = float3(1.0);
        exponent = 5.0;
        eta = 1.5;
        tir_cos = 0.0;
    }

    [ForceInline]
    public float3 eval(float c)
    {
        c = clamp(c, 0.0, 1.0);
        if (c < tir_cos)
            return float3(1.0);
        return schlick_color82_fresnel(F0, F82, F90, exponent, c);
    }

    [ForceInline]
    public float refraction_eta() { return eta; }
};

public struct MxSchlickAiryFresnel : IMxRefractiveFresnelPolicy
{
    public float3 F0;
    public float3 F82;
    public float3 F90;
    public float exponent;
    public float eta;
    public float tir_cos;
    public float thinfilm_thickness;
    public float thinfilm_ior;

    public __init(
        float3 F0,
        float3 F82,
        float3 F90,
        float exponent,
        float eta_value,
        bool backfacing,
        float thinfilm_thickness,
        float thinfilm_ior
    )
    {
        eta = oriented_dielectric_eta(eta_value, backfacing);
        this.F0 = clamp(F0, 0.0, 1.0);
        this.F82 = max(F82, float3(0.0));
        this.F90 = clamp(F90, 0.0, 1.0);
        this.exponent = exponent;
        tir_cos = dielectric_tir_cos(eta);
        this.thinfilm_thickness = thinfilm_thickness;
        this.thinfilm_ior = thinfilm_ior;
    }

    public __init()
    {
        F0 = float3(0.04);
        F82 = float3(1.0);
        F90 = float3(1.0);
        exponent = 5.0;
        eta = 1.5;
        tir_cos = 0.0;
        thinfilm_thickness = 0.0;
        thinfilm_ior = 1.5;
    }

    [ForceInline]
    public float3 eval(float c)
    {
        c = clamp(c, 0.0, 1.0);
        if (c < tir_cos)
            return float3(1.0);
        MxThinFilmSchlickFresnelData fd
            = MxThinFilmSchlickFresnelData(F0, F82, F90, exponent, thinfilm_thickness, thinfilm_ior);
        return fd.compute_fresnel(c);
    }

    [ForceInline]
    public float refraction_eta() { return eta; }
};

} // namespace mtlx
//
// Shared microfacet helpers ported from OSL/BSDL. The core GGX distribution
// uses Falcor's shared policy while bounded reflection helpers remain local.
// BSDL: external/dbg_OSL/src/libbsdl/include/BSDL/microfacet_tools_impl.h
// BSDL: external/dbg_OSL/src/libbsdl/include/BSDL/tools.h (reflect/refract)


namespace mtlx {

// BSDL: tools.h:301-329 refract
// Not a direct wrapper around Slang builtin refract(I, N, eta): callers pass
// the BSDL incoming/outgoing convention used by these MTLX microfacet samplers.
// This helper orients the normal, inverts eta when needed, handles eta == 0 and
// total internal reflection, and normalizes the result.
public float3 mx_refract(float3 incident, float3 normal, float eta)
{
    if (eta == 0.0)
        return float3(0.0);

    float3 normal_oriented;
    float cosi = dot(incident, normal);
    float neta;
    if (cosi > 0.0) {
        neta = 1.0 / eta;
        normal_oriented = normal;
    } else {
        cosi = -cosi;
        neta = eta;
        normal_oriented = -normal;
    }

    float arg = 1.0 - (neta * neta * (1.0 - cosi * cosi));
    if (arg >= 0.0) {
        float dnp = sqrt(arg);
        float normal_scale = (neta * cosi) - dnp;
        return normalize(incident * (-neta) + normal_oriented * normal_scale);
    }
    return float3(0.0);
}

// BSDL: microfacet_tools_decl.h:13-47 GGXDist + microfacet_tools_impl.h:19-111
// (D, G1, G2_G1, sample, sample_for_refl, D_refl_D).
public struct MxGGXDist
{
    private AnisotropicGGXPolicy<10000, true> m_dist;

    public __init(float rough, float aniso, bool flip_aniso)
    {
        if (flip_aniso)
            aniso = -aniso;
        m_dist
            = AnisotropicGGXPolicy<10000, true>(float2(rough * rough * (1.0 + aniso), rough * rough * (1.0 - aniso)));
    }

    public __init(float2 alpha)
    {
        alpha = clamp(alpha, float2(BSDL_EPSILON), float2(1.0));
        m_dist = AnisotropicGGXPolicy<10000, true>(alpha);
    }

    public __init() { m_dist = AnisotropicGGXPolicy<10000, true>(float2(1.0)); }

    public float D(float3 Hr) { return m_dist.eval_ndf(Hr); }

    public float G1(float3 w) { return m_dist.G1(w); }

    public float G2_G1(float3 wi, float3 wo) { return m_dist.G2(wi, wo) / m_dist.G1(wo); }

    public float3 sample(float3 wo, float randu, float randv)
    {
        const float2 alpha = m_dist.alpha;
        const float ax = alpha.x;
        const float ay = alpha.y;
        const float3 V = normalize(float3(ax * wo.x, ay * wo.y, wo.z));
        const float3 T1 = V.z < 0.9999 ? normalize(float3(V.y, -V.x, 0.0)) : float3(1.0, 0.0, 0.0);
        const float3 T2 = cross(T1, V);
        const float2 p = square_to_uniform_disk_concentric(float2(randu, randv));
        const float s = 0.5 * (1.0 + V.z);
        const float p2o = s * p.y + (1.0 - s) * sqrt(1.0 - p.x * p.x);
        const float p3 = sqrt(max(1.0 - p.x * p.x - p2o * p2o, 0.0));
        const float3 N = p.x * T1 + p2o * T2 + p3 * V;
        return normalize(float3(ax * N.x, ay * N.y, max(N.z, 0.0)));
    }

    public float3 sample_for_refl(float3 wo, float randu, float randv)
    {
        const float2 alpha = m_dist.alpha;
        const float ax = alpha.x;
        const float ay = alpha.y;
        float3 i_std = normalize(float3(wo.x * ax, wo.y * ay, wo.z));
        const float phi = 2.0 * PI * randu;
        const float a = clamp(min(ax, ay), 0.0, 1.0);
        const float s = 1.0 + sqrt(wo.x * wo.x + wo.y * wo.y);
        const float a2 = a * a;
        const float s2 = s * s;
        const float k = (1.0 - a2) * s2 / (s2 + a2 * wo.z * wo.z);
        const float b = k * i_std.z;
        const float z = (1.0 - randv) * (1.0 + b) - b;
        const float sin_theta = sqrt(clamp(1.0 - z * z, 0.0, 1.0));
        const float3 o_std = float3(sin_theta * cos(phi), sin_theta * sin(phi), z);
        const float3 m_std = i_std + o_std;
        return normalize(float3(m_std.x * ax, m_std.y * ay, m_std.z));
    }

    public float D_refl_D(float3 wo, float3 m)
    {
        const float2 alpha = m_dist.alpha;
        const float ax = alpha.x;
        const float ay = alpha.y;
        const float2 ai = float2(wo.x * ax, wo.y * ay);
        const float len2 = dot(ai, ai);
        const float t = sqrt(len2 + wo.z * wo.z);
        const float a = clamp(min(ax, ay), 0.0, 1.0);
        const float s = 1.0 + sqrt(wo.x * wo.x + wo.y * wo.y);
        const float a2 = a * a;
        const float s2 = s * s;
        const float k = (1.0 - a2) * s2 / (s2 + a2 * wo.z * wo.z);
        return 2.0 * wo.z / (k * wo.z + t);
    }
};

public float microfacet_roughness(float2 roughness_xy)
{
    const float2 r = clamp(roughness_xy, float2(BSDL_EPSILON), float2(1.0));
    const float ax = max(r.x, r.y);
    const float ay = min(r.x, r.y);
    const float b = ay / ax;
    const float aniso = (1.0 - b) / (1.0 + b);
    return sqrt(ax / (1.0 + aniso));
}

public float microfacet_aniso(float2 roughness_xy)
{
    const float2 r = clamp(roughness_xy, float2(BSDL_EPSILON), float2(1.0));
    const float ax = max(r.x, r.y);
    const float ay = min(r.x, r.y);
    const float b = ay / ax;
    return (1.0 - b) / (1.0 + b);
}

public MxGGXDist make_ggx_dist(float2 roughness_xy)
{
    return MxGGXDist(roughness_xy);
}

public float mini_microfacet_get_cosine(int i)
{
    const float x = float(i) * (1.0 / float(MINI_MICROFACET_GGX_NC - 1));
    return max(x * x, 1e-6);
}

public float tabulated_energy_interpolate_mini_microfacet(float roughness, int i)
{
    float rf = roughness * float(MINI_MICROFACET_GGX_NR - 1);
    int ra = int(rf);
    int rb = min(ra + 1, MINI_MICROFACET_GGX_NR - 1);
    rf -= float(ra);
    uint ia = uint(ra * MINI_MICROFACET_GGX_NC + i);
    uint ib = uint(rb * MINI_MICROFACET_GGX_NC + i);
    return lerp(mini_microfacet_ggx_energy(ia), mini_microfacet_ggx_energy(ib), rf);
}

// BSDL: microfacet_tools_impl.h:178-210 TabulatedEnergyCurve::Emiss_eval (SPI Nf=1 path).
public float tabulated_energy_emiss_eval_mini_microfacet(float roughness, float c)
{
    float cos0 = mini_microfacet_get_cosine(0);
    if (c <= cos0)
        return tabulated_energy_interpolate_mini_microfacet(roughness, 0);
    for (int i = 1; i < MINI_MICROFACET_GGX_NC; i++) {
        const float cos1 = mini_microfacet_get_cosine(i);
        if (c < cos1) {
            const float q = (c - cos0) / (cos1 - cos0);
            return lerp(
                tabulated_energy_interpolate_mini_microfacet(roughness, i - 1),
                tabulated_energy_interpolate_mini_microfacet(roughness, i),
                q
            );
        }
        cos0 = cos1;
    }
    return tabulated_energy_interpolate_mini_microfacet(roughness, MINI_MICROFACET_GGX_NC - 1);
}

public float dielectric_get_cosine(int i)
{
    return max(float(i) * (1.0 / float(DIELECTRIC_NC - 1)), 1e-6);
}

public float
tabulated_energy_interpolate_dielectric(StructuredBuffer<float> table, float roughness, float fresnel_index, int i)
{
    float rf = roughness * float(DIELECTRIC_NR - 1);
    int ra = int(rf);
    int rb = min(ra + 1, DIELECTRIC_NR - 1);
    rf -= float(ra);

    float ff = fresnel_index * float(DIELECTRIC_NF - 1);
    int fa = int(ff);
    int fb = min(fa + 1, DIELECTRIC_NF - 1);
    ff -= float(fa);

    uint iaa = uint((fa * DIELECTRIC_NR + ra) * DIELECTRIC_NC + i);
    uint iab = uint((fa * DIELECTRIC_NR + rb) * DIELECTRIC_NC + i);
    uint iba = uint((fb * DIELECTRIC_NR + ra) * DIELECTRIC_NC + i);
    uint ibb = uint((fb * DIELECTRIC_NR + rb) * DIELECTRIC_NC + i);
    return lerp(lerp(table[iaa], table[iab], rf), lerp(table[iba], table[ibb], rf), ff);
}

// BSDL: microfacet_tools_impl.h:178-210 TabulatedEnergyCurve::Emiss_eval (MTX Nf=32 path).
public float
tabulated_energy_emiss_eval_dielectric(StructuredBuffer<float> table, float roughness, float fresnel_index, float c)
{
    float cos0 = dielectric_get_cosine(0);
    if (c <= cos0)
        return tabulated_energy_interpolate_dielectric(table, roughness, fresnel_index, 0);
    for (int i = 1; i < DIELECTRIC_NC; i++) {
        const float cos1 = dielectric_get_cosine(i);
        if (c < cos1) {
            const float q = (c - cos0) / (cos1 - cos0);
            return lerp(
                tabulated_energy_interpolate_dielectric(table, roughness, fresnel_index, i - 1),
                tabulated_energy_interpolate_dielectric(table, roughness, fresnel_index, i),
                q
            );
        }
        cos0 = cos1;
    }
    return tabulated_energy_interpolate_dielectric(table, roughness, fresnel_index, DIELECTRIC_NC - 1);
}

// BSDL: microfacet_tools_impl.h:342-372 eval_turquin_microms_reflection.
// BSDL's "microms" abbreviation means microfacet multiple scattering. Mtlx
// keeps this as a generic GGX reflection lobe helper: callers provide the
// pre-evaluated Fresnel term and lobe scale, so the selected compensation
// remains visible at the BSDF call site.
public MxBsdfSample eval_ggx_reflection_lobe(MxGGXDist dist, float3 fresnel, float3 lobe_scale, float3 wi, float3 wo)
{
    const float cos_NO = wi.z;
    const float cos_NI = wo.z;
    if (cos_NI <= 0.0 || cos_NO <= 0.0)
        return MxBsdfSample();

    const float3 m = normalize(wi + wo);
    const float cos_MO = dot(m, wi);
    const float D = dist.D(m);
    const float D_refl_D = dist.D_refl_D(wi, m);
    const float D_refl = D_refl_D * D;
    const float G1 = dist.G1(wi);
    const float pdf = D_refl / (4.0 * cos_NO);
    const float out_value = dist.G2_G1(wo, wi) * G1 / D_refl_D;
    const float3 weight = out_value * fresnel * lobe_scale;
    return MxBsdfSample(wo, weight, pdf, -1.0);
}

// BSDL: microfacet_tools_impl.h:374-391 sample_turquin_microms_reflection.
// Samples the GGX reflection lobe and evaluates it through
// eval_ggx_reflection_lobe. Sampling owns the policy call because the
// microfacet normal and Fresnel term are discovered here; direct eval keeps the
// lobe scale explicit so compensation decisions stay visible in BSDF code.
public MxBsdfSample
sample_ggx_reflection_lobe<TFresnel : IMxFresnelPolicy, TCompensation : IMxMicrofacetCompensationPolicy>(
    MxGGXDist dist,
    TFresnel fresnel,
    float roughness,
    float3 wi,
    float3 rnd
)
{
    if (wi.z <= 0.0)
        return MxBsdfSample();

    float3 m = dist.sample_for_refl(wi, rnd.x, rnd.y);
    float3 wo = reflect(-wi, m);
    if (wo.z <= 0.0)
        return MxBsdfSample();
    const float3 F = fresnel.eval(dot(m, wi));
    return eval_ggx_reflection_lobe(dist, F, TCompensation::reflection_scale(roughness, wi.z, F), wi, wo);
}

} // namespace mtlx
//
// Energy-compensation policies for Mtlx microfacet BSDFs.


namespace mtlx {

public interface IMxMicrofacetCompensationPolicy
{
    public static float3 reflection_scale(float roughness, float cos_theta, float3 fresnel);
}

public interface IMxRefractiveCompensationPolicy : IMxMicrofacetCompensationPolicy
{
    public static float layer_energy(
        bool has_reflection,
        bool has_refraction,
        bool backfacing,
        float refraction_eta,
        float roughness,
        float cos_theta
    );
    public static float3 refractive_reflection_scale(float roughness, float cos_theta, float3 fresnel);
    public static float refraction_scale(
        bool has_reflection,
        bool has_refraction,
        bool backfacing,
        float refraction_eta,
        float roughness,
        float cos_theta
    );
}

public float bsdl_dielectric_energy_table_index_from_eta(float eta)
{
    const float x = (dielectric_base_ior(eta) - 1.001) * (1.0 / (5.0 - 1.001));
    return sqrt(x);
}

// MaterialX 1.39 mx_microfacet_specular.glsl analytic GGX directional albedo
// fit, named DIRECTIONAL_ALBEDO_ANALYTIC in the raster implementation.
public float3 ggx_directional_albedo_analytic(float n_dot_v, float alpha, float3 F0, float3 F90)
{
    float x = n_dot_v;
    float y = alpha;
    float x2 = x * x;
    float y2 = y * y;
    float4 r = float4(0.1003, 0.9345, 1.0, 1.0) + float4(-0.6303, -2.323, -1.765, 0.2281) * x
        + float4(9.748, 2.229, 8.263, 15.94) * y + float4(-2.038, -3.748, 11.53, -55.83) * x * y
        + float4(29.34, 1.424, 28.96, 13.08) * x2 + float4(-8.245, -0.7684, -7.507, 41.26) * y2
        + float4(-26.44, 1.436, -36.11, 54.9) * x2 * y + float4(19.99, 0.2913, 15.86, 300.2) * x * y2
        + float4(-5.448, 0.6286, 33.37, -285.1) * x2 * y2;
    float2 ab = clamp(r.xy / r.zw, 0.0, 1.0);
    return F0 * ab.x + F90 * ab.y;
}

public float3 turquin_analytic_reflection_scale(float n_dot_v, float alpha, float3 first_bounce_fresnel)
{
    float first_bounce_energy = ggx_directional_albedo_analytic(n_dot_v, alpha, float3(1.0), float3(1.0)).x;
    return float3(1.0) + first_bounce_fresnel * (1.0 - first_bounce_energy) / max(first_bounce_energy, 1e-6);
}

// Layering pass-through for dielectric policies that do not have BSDL
// tabulated layer-energy curves. Lobe compensation controls how the upper
// microfacet lobe is scaled, but a reflection-only dielectric top still needs
// to expose the base layer by the macro Fresnel transmission share. Returning
// zero here makes the top layer fully opaque, which is only correct for a
// perfect mirror, not for ordinary dielectric reflection.
public float dielectric_no_compensation_layer_energy(
    bool has_reflection,
    bool has_refraction,
    bool backfacing,
    float refraction_eta,
    float roughness,
    float cos_theta
)
{
    if (has_reflection && !has_refraction) {
        const float F = dielectric_fresnel_eval(refraction_eta, abs(cos_theta));
        return 1.0 - F;
    }
    return 0.0;
}

// Turquin 2019, "Practical multiple scattering compensation for microfacet
// models" (https://blog.selfshadow.com/publications/turquin/ms_comp_final.pdf).
// This policy uses the OSL/BSDL TabulatedEnergyCurve<MiniMicrofacetGGX>
// missing-energy LUT and matches the BSDL helper historically named
// eval_turquin_microms_reflection:
// scale = 1 + F * E_miss / max(0.01, 1 - E_miss).
public struct MxTurquinLutMicrofacetCompensation : IMxMicrofacetCompensationPolicy
{
    [ForceInline]
    public static float3 reflection_scale(float roughness, float cos_theta, float3 fresnel)
    {
        const float E_ms = tabulated_energy_emiss_eval_mini_microfacet(roughness, cos_theta);
        return float3(1.0) + fresnel * (E_ms / max(0.01, 1.0 - E_ms));
    }
};

// Turquin 2019 lobe scaling with the MaterialX 1.39 analytic GGX directional
// albedo fit in mx_microfacet_specular.glsl:
// mx_ggx_dir_albedo_analytic plus mx_ggx_energy_compensation.
// MaterialX names this directional-albedo method DIRECTIONAL_ALBEDO_ANALYTIC.
public struct MxTurquinAnalyticMicrofacetCompensation : IMxMicrofacetCompensationPolicy
{
    [ForceInline]
    public static float3 reflection_scale(float roughness, float cos_theta, float3 fresnel)
    {
        return turquin_analytic_reflection_scale(abs(cos_theta), roughness, fresnel);
    }
};

// Explicit no-compensation policy for MaterialX 1.38/PBRT-style parity
// experiments. This keeps lobe scales at one, but still models the macro
// Fresnel pass-through that reflection-only dielectric layers need in order to
// expose their base layer. Do not use a blanket zero layer_energy() here: that
// makes reflection-only dielectric tops fully opaque.
public struct MxNoCompensationMicrofacetCompensation : IMxMicrofacetCompensationPolicy
{
    [ForceInline]
    public static float3 reflection_scale(float roughness, float cos_theta, float3 fresnel) { return float3(1.0); }
};

public struct MxNoCompensationDielectricCompensation : IMxRefractiveCompensationPolicy
{
    [ForceInline]
    public static float3 reflection_scale(float roughness, float cos_theta, float3 fresnel) { return float3(1.0); }

    [ForceInline]
    public static float layer_energy(
        bool has_reflection,
        bool has_refraction,
        bool backfacing,
        float refraction_eta,
        float roughness,
        float cos_theta
    )
    {
        return dielectric_no_compensation_layer_energy(
            has_reflection,
            has_refraction,
            backfacing,
            refraction_eta,
            roughness,
            cos_theta
        );
    }

    [ForceInline]
    public static float3 refractive_reflection_scale(float roughness, float cos_theta, float3 fresnel)
    {
        return float3(1.0);
    }

    [ForceInline]
    public static float refraction_scale(
        bool has_reflection,
        bool has_refraction,
        bool backfacing,
        float refraction_eta,
        float roughness,
        float cos_theta
    )
    {
        return 1.0;
    }
};

// Turquin LUT reflection compensation plus OSL/BSDL tabulated dielectric layer
// energies for reflection/refraction modes. The layer tables are BSDL
// TabulatedEnergyCurve<DielectricReflFront/DielectricBothFront/DielectricBothBack>.
public struct MxTurquinLutDielectricCompensation : IMxRefractiveCompensationPolicy
{
    [ForceInline]
    public static float dielectric_energy_table_index(float refraction_eta)
    {
        return bsdl_dielectric_energy_table_index_from_eta(refraction_eta);
    }

    [ForceInline]
    public static float3 reflection_scale(float roughness, float cos_theta, float3 fresnel)
    {
        return MxTurquinLutMicrofacetCompensation::reflection_scale(roughness, cos_theta, fresnel);
    }

    [ForceInline]
    public static float layer_energy(
        bool has_reflection,
        bool has_refraction,
        bool backfacing,
        float refraction_eta,
        float roughness,
        float cos_theta
    )
    {
        const float fresnel_index = dielectric_energy_table_index(refraction_eta);
        if (has_reflection && !has_refraction) {
            return tabulated_energy_emiss_eval_dielectric(
                lut_globals.dielectric_reflfront_energy,
                roughness,
                fresnel_index,
                cos_theta
            );
        }
        if (has_refraction) {
            if (backfacing) {
                return tabulated_energy_emiss_eval_dielectric(
                    lut_globals.dielectric_bothback_energy,
                    roughness,
                    fresnel_index,
                    cos_theta
                );
            }
            return tabulated_energy_emiss_eval_dielectric(
                lut_globals.dielectric_bothfront_energy,
                roughness,
                fresnel_index,
                cos_theta
            );
        }
        return 0.0;
    }

    [ForceInline]
    public static float3 refractive_reflection_scale(float roughness, float cos_theta, float3 fresnel)
    {
        return float3(1.0);
    }

    [ForceInline]
    public static float refraction_scale(
        bool has_reflection,
        bool has_refraction,
        bool backfacing,
        float refraction_eta,
        float roughness,
        float cos_theta
    )
    {
        if (!has_refraction)
            return 1.0;
        return 1.0
            / max(0.01,
                  1.0 - layer_energy(has_reflection, has_refraction, backfacing, refraction_eta, roughness, cos_theta));
    }
};

// Turquin 2019 lobe scaling with the MaterialX 1.39 analytic GGX directional
// albedo fit. Refraction remains uncompensated, matching the MaterialX 1.39
// raster BSDF helpers that apply mx_ggx_energy_compensation only to reflection.
public struct MxTurquinAnalyticDielectricCompensation : IMxRefractiveCompensationPolicy
{
    [ForceInline]
    public static float3 reflection_scale(float roughness, float cos_theta, float3 fresnel)
    {
        return MxTurquinAnalyticMicrofacetCompensation::reflection_scale(roughness, cos_theta, fresnel);
    }

    [ForceInline]
    public static float layer_energy(
        bool has_reflection,
        bool has_refraction,
        bool backfacing,
        float refraction_eta,
        float roughness,
        float cos_theta
    )
    {
        return dielectric_no_compensation_layer_energy(
            has_reflection,
            has_refraction,
            backfacing,
            refraction_eta,
            roughness,
            cos_theta
        );
    }

    [ForceInline]
    public static float3 refractive_reflection_scale(float roughness, float cos_theta, float3 fresnel)
    {
        return reflection_scale(roughness, cos_theta, fresnel);
    }

    [ForceInline]
    public static float refraction_scale(
        bool has_reflection,
        bool has_refraction,
        bool backfacing,
        float refraction_eta,
        float roughness,
        float cos_theta
    )
    {
        return 1.0;
    }
};

} // namespace mtlx
//
// Scatter-mode policies for Mtlx dielectric-like MaterialX microfacet BSDFs.
//
// MaterialX exposes scatter_mode as R, T, or RT. The runtime policy preserves
// the current generated behavior, where the enum is folded into reflection and
// transmission tint values. Static policies are selected by codegen only when
// the enum value is proven constant; they remove the enum-dependent branch while
// still respecting zero tint values.


namespace mtlx {

public struct MxScatterTints
{
    public float3 reflection;
    public float3 transmission;
};

public MxScatterTints split_scatter_tints(int scatter_mode, float3 tint)
{
    MxScatterTints result;
    result.reflection = (scatter_mode == 1) ? float3(0.0) : tint;
    result.transmission = (scatter_mode == 0) ? float3(0.0) : tint;
    return result;
}

public interface IMxScatterModePolicy
{
    [ForceInline]
    public static bool dorefl(float3 reflection_tint);
    [ForceInline]
    public static bool dorefr(float3 transmission_tint);
}

public struct MxScatterRuntimeMode : IMxScatterModePolicy
{
    [ForceInline]
    public static bool dorefl(float3 reflection_tint) { return mx_hmax(reflection_tint) > 0.0; }

    [ForceInline]
    public static bool dorefr(float3 transmission_tint) { return mx_hmax(transmission_tint) > 0.0; }
}

public struct MxScatterStaticRMode : IMxScatterModePolicy
{
    [ForceInline]
    public static bool dorefl(float3 reflection_tint) { return MxScatterRuntimeMode::dorefl(reflection_tint); }

    [ForceInline]
    public static bool dorefr(float3 transmission_tint) { return false; }
}

public struct MxScatterStaticTMode : IMxScatterModePolicy
{
    [ForceInline]
    public static bool dorefl(float3 reflection_tint) { return false; }

    [ForceInline]
    public static bool dorefr(float3 transmission_tint) { return MxScatterRuntimeMode::dorefr(transmission_tint); }
}

public struct MxScatterStaticRTMode : IMxScatterModePolicy
{
    [ForceInline]
    public static bool dorefl(float3 reflection_tint) { return MxScatterRuntimeMode::dorefl(reflection_tint); }

    [ForceInline]
    public static bool dorefr(float3 transmission_tint) { return MxScatterRuntimeMode::dorefr(transmission_tint); }
}

} // namespace mtlx


namespace mtlx {

public static constexpr float OSLE_DIELECTRIC_IOR_MIN = 1.001;
public static constexpr float OSLE_DIELECTRIC_IOR_MAX = 5.0;

public float clamp_dielectric_ior(float ior)
{
    // BSDL's MaterialX DielectricLobe clamps the raw material IOR before
    // converting it through globals.relative_eta(...) and before backside
    // handling. Keep this at the MTLX wrapper boundary so the Mtlx BSDF can
    // still model BSDL eta/refraction behavior internally.
    return clamp(ior, OSLE_DIELECTRIC_IOR_MIN, OSLE_DIELECTRIC_IOR_MAX);
}

public float3 mx_geomprop_tangent(SurfaceInteraction si)
{
    return si.shading_frame_ws.tangent;
}

public float3 mx_geomprop_bitangent(SurfaceInteraction si)
{
    return si.shading_frame_ws.bitangent;
}

public float3 mx_viewdirection_vector3(SurfaceInteraction si)
{
    // MaterialX <viewdirection> is defined as viewer/camera-to-surface. Falcor
    // `si.wi_ws` points surface-to-viewer, so flip it at this graph-helper
    // boundary.
    return -si.wi_ws;
}

public float mx_smoothstep_float(float low, float high, float value)
{
    // Match MaterialX GLSL/MSL and GenOSL high-first edge behavior.
    // Reversed or degenerate intervals collapse to step(high, value).
    if (value >= high)
        return 1.0;
    if (value <= low)
        return 0.0;
    return smoothstep(low, high, value);
}

public float balance_probability(float3 albedo_a, float3 albedo_b)
{
    const float weight_a = luminance(albedo_a);
    const float weight_b = luminance(albedo_b);
    float sum = weight_a + weight_b;
    return sum <= 0.0 ? 0.5 : weight_a / sum;
}

public struct MxBranchChoice
{
    public bool first = false;
    public float pdf = 0.0;

    public __init() { }
    public __init(bool first, float pdf)
    {
        this.first = first;
        this.pdf = pdf;
    }
}

public MxBranchChoice mx_choose_branch<S : ISampleGenerator>(inout S sg, float p_first)
{
    p_first = clamp(p_first, 0.0, 1.0);
    bool first = next_float(sg) < p_first;
    float choice_pdf = first ? p_first : 1.0 - p_first;
    return MxBranchChoice(first, choice_pdf);
}

/// Reconstruct a partition at boundaries that provide only scattering albedo.
/// This fallback assigns all unattributed energy to upper-side absorption. Callers
/// with full AlbedoContributions should combine them directly when the operation
/// defines an exact combination.
public AlbedoContributions from_scattering_albedo(float3 reflection, float3 transmission)
{
    float3 absorption = max(float3(0.0), float3(1.0) - reflection - transmission);
    return AlbedoContributions(reflection, absorption, transmission, float3(0.0));
}

public float schlick_refraction_ior_from_color0(float3 color0)
{
    const float avg_F0 = clamp((color0.x + color0.y + color0.z) * (1.0 / 3.0), 0.0, 0.99);
    const float sqrt_F0 = sqrt(avg_F0);
    return (1.0 + sqrt_F0) / (1.0 - sqrt_F0);
}

public struct MxNullBSDF : ILayerableBSDF
{
    // ILayerableBSDF

    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return BSDFEval();
    }
    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        sample = {};
        return false;
    }
    public AlbedoContributions eval_albedo(const float3 wi)
    {
        return AlbedoContributions(float3(0.0), float3(0.0), float3(1.0), float3(0.0));
    }
    public RoughnessInformation eval_roughness(const float3 wi) { return RoughnessInformation(); }
    public float3 directional_albedo(const float3 wi) { return float3(0.0); }
    public float3 layer_opacity(const float3 wi) { return float3(0.0); }
    public property float3 ior_as_reflectance
    {
        get {
            return float3(0.0);
        }
    }
}

static const float3 OSLE_SIMPLE_BTDF_TRANSMITTANCE = float3(1.0);

/// Untinted straight-through delta transmission used as the transparent branch
/// for Mtlx root opacity lowering. Keep this zero-state so generated opacity
/// topology can share a single pass-through BSDF shape across all layering modes.
public struct MxSimpleBTDF : ILayerableBSDF
{
    // ILayerableBSDF

    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return BSDFEval();
    }
    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        sample = {};
        sample.wo = -wi;
        sample.pdf = 0.0;
        sample.weight = OSLE_SIMPLE_BTDF_TRANSMITTANCE;
        sample.flags = BSDFFlags::delta_transmission;
        return true;
    }
    public AlbedoContributions eval_albedo(const float3 wi)
    {
        return AlbedoContributions(float3(0.0), float3(0.0), OSLE_SIMPLE_BTDF_TRANSMITTANCE, float3(0.0));
    }
    public RoughnessInformation eval_roughness(const float3 wi) { return RoughnessInformation(); }
    public float3 directional_albedo(const float3 wi) { return OSLE_SIMPLE_BTDF_TRANSMITTANCE; }
    public float3 layer_opacity(const float3 wi) { return float3(0.0); }
    public property float3 ior_as_reflectance
    {
        get {
            return float3(0.0);
        }
    }
}

/// Layerable adapter for legacy Falcor BSDFs used as Mtlx BSDFs.
public struct MxLayerableBSDF<Inner : IBSDF> : ILayerableBSDF
{
    public Inner inner = {};

    public __init() { }
    public __init(Inner inner) { this.inner = inner; }

    // ILayerableBSDF

    public float3 directional_albedo(const float3 wi)
    {
        AlbedoContributions albedo = inner.eval_albedo(wi);
        return albedo.reflection + albedo.transmission;
    }
    public float3 layer_opacity(const float3 wi) { return saturate(float3(1.0) - inner.eval_albedo(wi).transmission); }
    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return inner.eval_with_pdf(wi, wo, sg, bc);
    }
    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        return inner.sample(wi, sample, sg, bc);
    }
    public AlbedoContributions eval_albedo(const float3 wi) { return inner.eval_albedo(wi); }
    public RoughnessInformation eval_roughness(const float3 wi) { return inner.eval_roughness(wi); }
    public property float3 ior_as_reflectance
    {
        get {
            return inner.ior_as_reflectance;
        }
    }
}

public struct MxWeightedBSDF<Inner : ILayerableBSDF> : ILayerableBSDF
{
    public Inner inner = {};
    public float3 weight = float3(1.0);
    // TODO(mtlx-vdf): Generated MTLX VDF nodes can write this scale for
    // raster-style Beer attenuation. The current BSDF path does not transport
    // medium coefficients, so eval/sample/albedo intentionally ignore this
    // field until VDF transport is wired in.
    public float3 transmission_scale = float3(1.0);

    public __init() { }
    public __init(Inner inner, float3 weight)
    {
        this.inner = inner;
        this.weight = weight;
    }
    public __init(Inner inner, float3 weight, float3 transmission_scale)
    {
        this.inner = inner;
        this.weight = weight;
        this.transmission_scale = transmission_scale;
    }

    // ILayerableBSDF

    public float3 directional_albedo(const float3 wi)
    {
        if (scalar_weight() <= 0.0)
            return float3(0.0);
        return weight * inner.directional_albedo(wi);
    }
    public float3 layer_opacity(const float3 wi)
    {
        if (scalar_weight() <= 0.0)
            return float3(0.0);
        return saturate(weight * inner.layer_opacity(wi));
    }
    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        if (scalar_weight() <= 0.0)
            return BSDFEval();
        BSDFEval result = inner.eval_with_pdf(wi, wo, sg, bc);
        result.value *= weight;
        return result;
    }
    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        sample = {};

        if (scalar_weight() <= 0.0)
            return false;
        bool valid = inner.sample(wi, sample, sg, bc);
        if (!valid)
            return false;
        sample.weight *= weight;
        return true;
    }
    public AlbedoContributions eval_albedo(const float3 wi)
    {
        if (scalar_weight() <= 0.0)
            return from_scattering_albedo(float3(0.0), float3(0.0));
        AlbedoContributions albedo = inner.eval_albedo(wi);
        return from_scattering_albedo(weight * albedo.reflection, weight * albedo.transmission);
    }
    public RoughnessInformation eval_roughness(const float3 wi)
    {
        return scalar_weight() > 0.0 ? inner.eval_roughness(wi) : RoughnessInformation();
    }
    public property float3 ior_as_reflectance
    {
        get {
            return scalar_weight() > 0.0? weight* inner.ior_as_reflectance : float3(0.0);
        }
    }

    // Implementation

    private float scalar_weight() { return clamp(mx_hmax(weight), 0.0, 1.0); }
}

public struct MxFramedWeightedBSDF<Inner : ILayerableBSDF> : ILayerableBSDF
{
    public Inner inner = {};
    public float3 weight = float3(1.0);
    // TODO(mtlx-vdf): See MxWeightedBSDF. This field is
    // accepted from generated VDF layering code but ignored for OSL parity.
    public float3 transmission_scale = float3(1.0);
    public float material_transmission = 0.0;
    public Frame frame = Frame::identity();

    public __init() { frame = Frame::identity(); }
    public __init(Inner inner, float3 weight)
    {
        this.inner = inner;
        this.weight = weight;
        this.frame = Frame::identity();
    }
    public __init(Inner inner, float3 weight, Frame frame)
    {
        this.inner = inner;
        this.weight = weight;
        this.frame = frame;
    }
    public __init(Inner inner, float3 weight, float3 transmission_scale, Frame frame)
    {
        this.inner = inner;
        this.weight = weight;
        this.transmission_scale = transmission_scale;
        this.frame = frame;
    }
    public __init(Inner inner, float3 weight, float3 transmission_scale, float material_transmission, Frame frame)
    {
        this.inner = inner;
        this.weight = weight;
        this.transmission_scale = transmission_scale;
        this.material_transmission = material_transmission;
        this.frame = frame;
    }

    // ILayerableBSDF

    public float3 directional_albedo(const float3 wi)
    {
        if (scalar_weight() <= 0.0)
            return float3(0.0);
        return weight * inner.directional_albedo(frame.to_local(wi));
    }
    public float3 layer_opacity(const float3 wi)
    {
        if (scalar_weight() <= 0.0)
            return float3(0.0);
        return saturate(weight * inner.layer_opacity(frame.to_local(wi)));
    }
    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        if (scalar_weight() <= 0.0)
            return BSDFEval();
        float3 local_wi = frame.to_local(wi);
        float3 local_wo = frame.to_local(wo);
        BSDFEval result = inner.eval_with_pdf(local_wi, local_wo, sg, bc);
        result.value *= weight;
        return result;
    }
    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        sample = {};

        if (scalar_weight() <= 0.0)
            return false;
        const float3 local_wi = frame.to_local(wi);
        BSDFSample local_sample;
        bool valid = inner.sample(local_wi, local_sample, sg, bc);
        if (!valid)
            return false;
        sample = local_sample;
        sample.wo = frame.to_global(local_sample.wo);
        sample.weight *= weight;
        return true;
    }
    public AlbedoContributions eval_albedo(const float3 wi)
    {
        if (scalar_weight() <= 0.0)
            return from_scattering_albedo(float3(0.0), float3(0.0));
        const float3 local_wi = frame.to_local(wi);
        AlbedoContributions albedo = inner.eval_albedo(local_wi);
        return from_scattering_albedo(weight * albedo.reflection, weight * albedo.transmission);
    }
    public RoughnessInformation eval_roughness(const float3 wi)
    {
        return scalar_weight() > 0.0 ? inner.eval_roughness(frame.to_local(wi)) : RoughnessInformation();
    }
    public property float3 ior_as_reflectance
    {
        get {
            return scalar_weight() > 0.0? weight* inner.ior_as_reflectance : float3(0.0);
        }
    }

    // Implementation

    private float scalar_weight() { return clamp(mx_hmax(weight), 0.0, 1.0); }
}

public struct MxLayerBSDF<Top : ILayerableBSDF, Base : ILayerableBSDF> : ILayerableBSDF
{
    public Top top = {};
    public Base base = {};

    public __init() { }
    public __init(Top top, Base base)
    {
        this.top = top;
        this.base = base;
    }

    // ILayerableBSDF

    public float3 directional_albedo(const float3 wi)
    {
        return top.directional_albedo(wi) + base_scale(wi) * base.directional_albedo(wi);
    }
    public float3 layer_opacity(const float3 wi)
    {
        // TODO(mtlx-layering): Validate this against MaterialX/OSL
        // CompositeBSDF layer opacity semantics, not a physically combined
        // coat opacity. When a
        // layer() closure is used as the top of another layer(), MTLX
        // traverses through the nested layer and returns the opacity of that
        // nested layer's base branch. This preserves the bsdf_mix closure
        // weights used by the composite BSDF preparation step.
        return base.layer_opacity(wi);
    }
    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        BSDFEval top_result = top.eval_with_pdf(wi, wo, sg, bc);
        BSDFEval base_result = base.eval_with_pdf(wi, wo, sg, bc);
        float p_top = top_sample_probability(wi);
        return BSDFEval(
            top_result.value + base_scale(wi) * base_result.value,
            p_top * top_result.pdf + (1.0 - p_top) * base_result.pdf
        );
    }
    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        sample = {};

        float p_top = top_sample_probability(wi);
        MxBranchChoice choice = mx_choose_branch(sg, p_top);
        if (choice.pdf <= 0.0)
            return false;
        bool valid = choice.first ? top.sample(wi, sample, sg, bc) : base.sample(wi, sample, sg, bc);
        if (!valid)
            return false;
        if (sample.has_flag(BSDFFlags::delta)) {
            sample.pdf = choice.pdf * sample.pdf;
            sample.weight = choice.first ? sample.weight / choice.pdf : base_scale(wi) * sample.weight / choice.pdf;
            return true;
        }
        const BSDFEval eval_result = eval_with_pdf(wi, sample.wo, sg, bc);
        sample.pdf = eval_result.pdf;
        if (sample.pdf <= 0.0)
            return false;
        sample.weight = eval_result.value / sample.pdf;
        return true;
    }
    public AlbedoContributions eval_albedo(const float3 wi)
    {
        const float3 base_weight = base_scale(wi);
        AlbedoContributions top_albedo = top.eval_albedo(wi);
        AlbedoContributions base_albedo = base.eval_albedo(wi);
        return from_scattering_albedo(
            top_albedo.reflection + base_weight * base_albedo.reflection,
            top_albedo.transmission + base_weight * base_albedo.transmission
        );
    }
    public RoughnessInformation eval_roughness(const float3 wi)
    {
        RoughnessInformation top_r = top.eval_roughness(wi);
        return top_r;
    }
    public property float3 ior_as_reflectance
    {
        get {
            return top.ior_as_reflectance;
        }
    }

    // Implementation

    private float3 base_scale(const float3 wi) { return saturate(float3(1.0) - top.layer_opacity(wi)); }
    private float top_sample_probability(const float3 wi)
    {
        return balance_probability(top.directional_albedo(wi), base_scale(wi) * base.directional_albedo(wi));
    }
}

public struct MxMixBSDF<FG : ILayerableBSDF, BG : ILayerableBSDF> : ILayerableBSDF
{
    public FG fg = {};
    public BG bg = {};
    public float3 mix_value = float3(0.0);
    public __init() { }
    public __init(FG fg, BG bg, float mix_value)
    {
        this.fg = fg;
        this.bg = bg;
        this.mix_value = saturate(float3(mix_value));
    }
    public __init(FG fg, BG bg, float3 mix_value)
    {
        this.fg = fg;
        this.bg = bg;
        this.mix_value = saturate(mix_value);
    }

    // ILayerableBSDF

    public float3 directional_albedo(const float3 wi)
    {
        return (float3(1.0) - mix_value) * bg.directional_albedo(wi) + mix_value * fg.directional_albedo(wi);
    }
    public float3 layer_opacity(const float3 wi)
    {
        return saturate((float3(1.0) - mix_value) * bg.layer_opacity(wi) + mix_value * fg.layer_opacity(wi));
    }
    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        BSDFEval bg_result = bg.eval_with_pdf(wi, wo, sg, bc);
        BSDFEval fg_result = fg.eval_with_pdf(wi, wo, sg, bc);
        const float p_fg = fg_sample_probability(wi);
        return BSDFEval(
            (float3(1.0) - mix_value) * bg_result.value + mix_value * fg_result.value,
            p_fg * fg_result.pdf + (1.0 - p_fg) * bg_result.pdf
        );
    }
    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        sample = {};

        // OSL closure mix(a, b, amount) flattens into a*(1-amount) first,
        // followed by b*amount. Keep that CDF order so the OSL-style replay
        // remaps rx by subtracting the background interval before foreground.
        MxBranchChoice choice = mx_choose_branch(sg, bg_sample_probability(wi));
        if (choice.pdf <= 0.0)
            return false;
        bool valid = choice.first ? bg.sample(wi, sample, sg, bc) : fg.sample(wi, sample, sg, bc);
        if (!valid)
            return false;
        if (sample.has_flag(BSDFFlags::delta)) {
            sample.pdf = choice.pdf * sample.pdf;
            sample.weight = (choice.first ? float3(1.0) - mix_value : mix_value) * sample.weight / choice.pdf;
            return true;
        }
        const BSDFEval eval_result = eval_with_pdf(wi, sample.wo, sg, bc);
        sample.pdf = eval_result.pdf;
        if (sample.pdf <= 0.0)
            return false;
        sample.weight = eval_result.value / sample.pdf;
        return true;
    }
    public AlbedoContributions eval_albedo(const float3 wi)
    {
        const float3 bg_mix = float3(1.0) - mix_value;
        AlbedoContributions bg_albedo = bg.eval_albedo(wi);
        AlbedoContributions fg_albedo = fg.eval_albedo(wi);
        return AlbedoContributions(
            bg_mix * bg_albedo.reflection + mix_value * fg_albedo.reflection,
            bg_mix * bg_albedo.absorption_r + mix_value * fg_albedo.absorption_r,
            bg_mix * bg_albedo.transmission + mix_value * fg_albedo.transmission,
            bg_mix * bg_albedo.absorption_t + mix_value * fg_albedo.absorption_t
        );
    }
    public RoughnessInformation eval_roughness(const float3 wi)
    {
        RoughnessInformation fr = fg.eval_roughness(wi);
        RoughnessInformation br = bg.eval_roughness(wi);
        float p_fg = fg_sample_probability(wi);
        return RoughnessInformation(lerp(br.roughness, fr.roughness, p_fg), lerp(br.scratch, fr.scratch, p_fg));
    }
    public property float3 ior_as_reflectance
    {
        get {
            return (float3(1.0) - mix_value) * bg.ior_as_reflectance + mix_value * fg.ior_as_reflectance;
        }
    }

    // Implementation

    private float fg_sample_probability(const float3 wi)
    {
        return balance_probability(
            mix_value * fg.directional_albedo(wi),
            (float3(1.0) - mix_value) * bg.directional_albedo(wi)
        );
    }
    private float bg_sample_probability(const float3 wi)
    {
        return balance_probability(
            (float3(1.0) - mix_value) * bg.directional_albedo(wi),
            mix_value * fg.directional_albedo(wi)
        );
    }
}

public struct MxAddBSDF<In1 : ILayerableBSDF, In2 : ILayerableBSDF> : ILayerableBSDF
{
    public In1 in1 = {};
    public In2 in2 = {};
    public __init() { }
    public __init(In1 in1, In2 in2)
    {
        this.in1 = in1;
        this.in2 = in2;
    }

    // ILayerableBSDF

    public float3 directional_albedo(const float3 wi)
    {
        return in1.directional_albedo(wi) + in2.directional_albedo(wi);
    }
    public float3 layer_opacity(const float3 wi) { return saturate(in1.layer_opacity(wi) + in2.layer_opacity(wi)); }
    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        BSDFEval result1 = in1.eval_with_pdf(wi, wo, sg, bc);
        BSDFEval result2 = in2.eval_with_pdf(wi, wo, sg, bc);
        float p = p_in1(wi);
        return BSDFEval(result1.value + result2.value, p * result1.pdf + (1.0 - p) * result2.pdf);
    }
    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        sample = {};

        float p = p_in1(wi);
        MxBranchChoice choice = mx_choose_branch(sg, p);
        if (choice.pdf <= 0.0)
            return false;
        bool valid = choice.first ? in1.sample(wi, sample, sg, bc) : in2.sample(wi, sample, sg, bc);
        if (!valid)
            return false;
        if (sample.has_flag(BSDFFlags::delta)) {
            sample.pdf = choice.pdf * sample.pdf;
            sample.weight = sample.weight / choice.pdf;
            return true;
        }
        const BSDFEval eval_result = eval_with_pdf(wi, sample.wo, sg, bc);
        sample.pdf = eval_result.pdf;
        if (sample.pdf <= 0.0)
            return false;
        sample.weight = eval_result.value / sample.pdf;
        return true;
    }
    public AlbedoContributions eval_albedo(const float3 wi)
    {
        AlbedoContributions albedo1 = in1.eval_albedo(wi);
        AlbedoContributions albedo2 = in2.eval_albedo(wi);
        return from_scattering_albedo(
            albedo1.reflection + albedo2.reflection,
            albedo1.transmission + albedo2.transmission
        );
    }
    public RoughnessInformation eval_roughness(const float3 wi)
    {
        RoughnessInformation r1 = in1.eval_roughness(wi);
        RoughnessInformation r2 = in2.eval_roughness(wi);
        float p = p_in1(wi);
        return RoughnessInformation(lerp(r2.roughness, r1.roughness, p), lerp(r2.scratch, r1.scratch, p));
    }
    public property float3 ior_as_reflectance
    {
        get {
            return saturate(in1.ior_as_reflectance + in2.ior_as_reflectance);
        }
    }

    // Implementation

    private float p_in1(const float3 wi)
    {
        return balance_probability(in1.directional_albedo(wi), in2.directional_albedo(wi));
    }
}

} // namespace mtlx
//
// MaterialX conductor BSDF ported from OSL/BSDL MTX.
// BSDL: external/dbg_OSL/src/libbsdl/include/BSDL/MTX/bsdf_conductor_impl.h
// BSDL: external/dbg_OSL/src/libbsdl/include/BSDL/MTX/bsdf_conductor_decl.h


namespace mtlx {

public struct MxCompensatedConductorBSDF<
    TFresnel : IMxReflectiveFresnelPolicy,
    TCompensation : IMxMicrofacetCompensationPolicy> : ILayerableBSDF
{
    public float2 roughness_xy;
    public TFresnel fresnel;
    public bool retroreflective;

    public __init() { }

    public __init(float2 roughness_xy, TFresnel fresnel, bool retroreflective)
    {
        this.roughness_xy = roughness_xy;
        this.fresnel = fresnel;
        this.retroreflective = retroreflective;
    }

    // ILayerableBSDF


    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        MxBsdfSample r = eval_bsdf_sample(retroreflective ? mx_retroreflect(wi) : wi, wo);
        return BSDFEval(r.pdf > 0.0 ? r.weight * r.pdf : float3(0.0), r.pdf);
    }

    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        sample = {};
        MxBsdfSample r = sample_bsdf_sample(wi, next_float3(sg));
        if (r.pdf <= 0.0)
            return false;
        sample.wo = retroreflective ? mx_retroreflect(r.wo) : r.wo;
        sample.pdf = r.pdf;
        sample.weight = r.weight;
        sample.flags = BSDFFlags::glossy_reflection;
        return true;
    }


    [ForceInline]
    public AlbedoContributions eval_albedo(const float3 wi)
    {
        const float3 reflection = saturate(directional_albedo(wi));
        return AlbedoContributions(reflection, float3(1.0) - reflection, float3(0.0), float3(0.0));
    }

    public RoughnessInformation eval_roughness(const float3 wi)
    {
        return RoughnessInformation(roughness_xy, float3(0.0));
    }

    // Conductor's directional albedo is the BSDL average Fresnel for legacy
    // and the MaterialX thin-film directional albedo for Airy.
    [ForceInline]
    public float3 directional_albedo(const float3 wi)
    {
        return fresnel.directional_albedo(clamp(abs(wi.z), 0.0, 1.0), microfacet_roughness(roughness_xy));
    }

    public float3 layer_opacity(const float3 wi) { return float3(1.0); }

    public property float3 ior_as_reflectance
    {
        get {
            return float3(0.0);
        }
    }

    // Implementation

    // BSDL: bsdf_conductor_impl.h:109-117 eval_impl, translated to Falcor
    // wi/wo naming.
    private MxBsdfSample eval_bsdf_sample(const float3 wi, const float3 wo)
    {
        const float roughness = microfacet_roughness(roughness_xy);
        const float3 F = fresnel.eval(dot(normalize(wi + wo), wi));
        MxBsdfSample s = eval_ggx_reflection_lobe(
            make_ggx_dist(roughness_xy),
            F,
            TCompensation::reflection_scale(roughness, wi.z, F),
            wi,
            wo
        );
        s.roughness = roughness;
        return s;
    }

    // BSDL: bsdf_conductor_impl.h:119-127 sample_impl
    private MxBsdfSample sample_bsdf_sample(const float3 wi, const float3 rnd)
    {
        const float roughness = microfacet_roughness(roughness_xy);
        MxBsdfSample s = sample_ggx_reflection_lobe<TFresnel, TCompensation>(
            make_ggx_dist(roughness_xy),
            fresnel,
            roughness,
            wi,
            rnd
        );
        s.roughness = roughness;
        return s;
    }
};

public typealias MxConductorBSDF = MxCompensatedConductorBSDF<MxConductorFresnel, MxTurquinLutMicrofacetCompensation>;
public typealias MxConductorTurquinAnalyticCompensationBSDF
    = MxCompensatedConductorBSDF<MxConductorFresnel, MxTurquinAnalyticMicrofacetCompensation>;
public typealias MxConductorNoCompensationBSDF
    = MxCompensatedConductorBSDF<MxConductorFresnel, MxNoCompensationMicrofacetCompensation>;

public typealias MxConductorAiryBSDF
    = MxCompensatedConductorBSDF<MxConductorAiryFresnel, MxTurquinLutMicrofacetCompensation>;
public typealias MxConductorAiryTurquinAnalyticCompensationBSDF
    = MxCompensatedConductorBSDF<MxConductorAiryFresnel, MxTurquinAnalyticMicrofacetCompensation>;
public typealias MxConductorAiryNoCompensationBSDF
    = MxCompensatedConductorBSDF<MxConductorAiryFresnel, MxNoCompensationMicrofacetCompensation>;

} // namespace mtlx
//
// MaterialX dielectric BSDF ported from OSL/BSDL MTX.
// BSDL: external/dbg_OSL/src/libbsdl/include/BSDL/MTX/bsdf_dielectric_impl.h
// BSDL: external/dbg_OSL/src/libbsdl/include/BSDL/MTX/bsdf_dielectric_decl.h
//
// Implements the BSDLConfig::use_bvn_refraction=true path (config.h:94 default).
// "BVN" = Bounded VNDF (Eto-Tokuyoshi, "Bounded VNDF Sampling for Smith-GGX
// Reflections"), used in DielectricBSDF::eval / sample.


namespace mtlx {

public struct MxCompensatedDielectricBSDF<
    TFresnel : IMxRefractiveFresnelPolicy,
    TCompensation : IMxRefractiveCompensationPolicy,
    TScatter : IMxScatterModePolicy> : ILayerableBSDF
{
    public float2 roughness_xy;
    public TFresnel fresnel;
    public float3 reflection_tint;
    public float3 transmission_tint;
    public float3 absorption;
    public bool retroreflective;
    public bool backfacing;

    public __init() { }

    public __init(
        float2 roughness_xy,
        TFresnel fresnel,
        float3 reflection_tint,
        float3 transmission_tint,
        float3 absorption,
        bool retroreflective,
        bool backfacing
    )
    {
        this.roughness_xy = roughness_xy;
        this.fresnel = fresnel;
        this.reflection_tint = reflection_tint;
        this.transmission_tint = transmission_tint;
        this.absorption = absorption;
        this.retroreflective = retroreflective;
        this.backfacing = backfacing;
    }

    // ILayerableBSDF


    public override
        float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        return eval_with_pdf(wi, wo, sg, bc).value;
    }

    public override float eval_pdf(const float3 wi, const float3 wo, const BSDFContext bc)
    {
        EvalPdfFallbackSampleGenerator sg = EvalPdfFallbackSampleGenerator(0u);
        return eval_with_pdf(wi, wo, sg, bc).pdf;
    }

    public BSDFEval
    eval_with_pdf<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, const BSDFContext bc)
    {
        // MaterialX retroreflection affects reflection events only; transmission directions remain unchanged.
        const float3 eval_wi = retroreflective && wi.z * wo.z > 0.0 ? mx_retroreflect(wi) : wi;
        MxBsdfSample r = eval_bsdf_sample(eval_wi, wo);
        return BSDFEval(r.pdf > 0.0 ? r.weight * r.pdf : float3(0.0), r.pdf);
    }

    public bool sample<S : ISampleGenerator>(const float3 wi, out BSDFSample sample, inout S sg, const BSDFContext bc)
    {
        sample = {};
        MxBsdfSample r = sample_bsdf_sample(wi, next_float3(sg));
        if (r.pdf <= 0.0)
            return false;
        sample.wo = retroreflective && wi.z * r.wo.z > 0.0 ? mx_retroreflect(r.wo) : r.wo;
        sample.pdf = r.pdf;
        sample.weight = r.weight;
        sample.flags = sample_lobe_types(wi, sample.wo);
        return true;
    }


    [ForceInline]
    public AlbedoContributions eval_albedo(const float3 wi)
    {
        if (!dorefl() && !dorefr())
            return AlbedoContributions(float3(0.0), float3(1.0), float3(0.0), float3(0.0));

        const float E_ms = layer_energy(wi);
        float3 reflection;
        float3 transmission;
        if (!dorefr()) {
            reflection = reflection_tint * (1.0 - E_ms);
            transmission = float3(E_ms) * wi_absorption(wi);
        } else {
            const float3 F = fresnel.eval(abs(wi.z));
            reflection = F * (1.0 - E_ms) * reflection_tint;
            transmission = (float3(1.0) - F) * (1.0 - E_ms) * transmission_tint;
        }

        reflection = saturate(reflection);
        transmission = saturate(transmission);
        return AlbedoContributions(
            reflection,
            max(float3(0.0), float3(1.0) - reflection - transmission),
            transmission,
            float3(0.0)
        );
    }

    public RoughnessInformation eval_roughness(const float3 wi)
    {
        return RoughnessInformation(roughness_xy, float3(0.0));
    }

    [ForceInline]
    public float3 directional_albedo(const float3 wi)
    {
        return !dorefr() ? reflection_tint * (1.0 - layer_energy(wi)) : float3(1.0);
    }

    public float3 layer_opacity(const float3 wi)
    {
        const float3 layer_transmission = !dorefr() ? float3(layer_energy(wi)) * wi_absorption(wi) : float3(0.0);
        return saturate(float3(1.0) - layer_transmission);
    }

    public property float3 ior_as_reflectance
    {
        get {
            return float3(0.0);
        }
    }

    // Implementation

    [ForceInline]
    private bool dorefl()
    {
        return TScatter::dorefl(reflection_tint);
    }

    [ForceInline]
    private bool dorefr() { return TScatter::dorefr(transmission_tint); }

    [ForceInline]
    private float layer_energy(float3 wi)
    {
        return TCompensation::layer_energy(
            dorefl(),
            dorefr(),
            backfacing,
            fresnel.refraction_eta(),
            microfacet_roughness(roughness_xy),
            wi.z
        );
    }

    // BSDL: bsdf_dielectric_impl.h:110-177 DielectricBSDF::eval (BVN branch)
    private MxBsdfSample spec_eval(const float3 wi, const float3 wo)
    {
        MxGGXDist dist = make_ggx_dist(roughness_xy);
        const float roughness = microfacet_roughness(roughness_xy);
        if (!dorefr()) {
            const float3 F = fresnel.eval(dot(normalize(wo + wi), wi));
            return eval_ggx_reflection_lobe(dist, F, TCompensation::reflection_scale(roughness, wi.z, F), wi, wo);
        }

        const float cos_NO = wi.z;
        const float cos_NI = wo.z;
        const bool do_reflect = dorefl();
        const bool do_refract = dorefr();
        const bool both_scatter = do_reflect && do_refract;
        if (cos_NI > 0.0) {
            if (!do_reflect)
                return MxBsdfSample();
            const float3 m = normalize(wi + wo);
            const float cos_MO = dot(m, wi);
            if (cos_MO <= 0.0)
                return MxBsdfSample();
            const float D = dist.D(m);
            const float G1 = dist.G1(wi);
            const float3 F = fresnel.eval(cos_MO);
            const float Fmax = mx_hmax(F);
            if (Fmax <= 0.0)
                return MxBsdfSample();
            const float D_refl_D = dist.D_refl_D(wi, m);
            const float D_refl = D_refl_D * D;
            // BSDL's MTX dielectric is a combined R/T helper. Advanced
            // layering can feed us as a T-only IBSDF; in that case reflection
            // is absent and the Fresnel branch probability must be one only
            // for true combined R/T BSDFs.
            const float branch_pdf = both_scatter ? balance_probability(F, float3(1.0) - F) : 1.0;
            const float3 out_value = F * TCompensation::refractive_reflection_scale(roughness, wi.z, F)
                * (dist.G2_G1(wo, wi) * G1 / (D_refl_D * branch_pdf));
            const float pdf = D_refl / (4.0 * cos_NO) * branch_pdf;
            return MxBsdfSample(wo, out_value, pdf, 0.0);
        } else if (cos_NI < 0.0) {
            if (!do_refract)
                return MxBsdfSample();
            float3 Ht = normalize(fresnel.refraction_eta() * wo + wi) * ((fresnel.refraction_eta() > 1.0) ? -1.0 : 1.0);
            const float cos_HO = dot(Ht, wi);
            const float cos_HI = dot(Ht, wo);
            if (cos_HO <= 0.0 || cos_HI >= 0.0)
                return MxBsdfSample();
            const float3 Ft = float3(1.0) - fresnel.eval(cos_HO);
            const float Ftmax = mx_hmax(Ft);
            if (Ht.z <= 0.0 || Ftmax <= 0.0)
                return MxBsdfSample();
            const float D = dist.D(Ht);
            const float G1 = dist.G1(wi);
            const float eta_refract = fresnel.refraction_eta();
            const float denom = cos_HI * eta_refract + cos_HO;
            const float J = (-cos_HI * cos_HO * eta_refract * eta_refract) / (wi.z * denom * denom);
            const float D_refl_D = dist.D_refl_D(wi, Ht);
            const float D_refl = D_refl_D * D;
            const float branch_pdf = both_scatter ? balance_probability(Ft, float3(1.0) - Ft) : 1.0;
            const float pdf = D_refl * J * branch_pdf;
            const float3 out_value = Ft * (dist.G2_G1(float3(wo.x, wo.y, -wo.z), wi) * G1 / (D_refl_D * branch_pdf));
            return MxBsdfSample(wo, out_value, pdf, 0.0);
        }
        return MxBsdfSample();
    }

    // BSDL: bsdf_dielectric_impl.h:179-206 DielectricBSDF::sample (BVN branch)
    private MxBsdfSample spec_sample(const float3 wi, const float3 rnd)
    {
        if (!dorefr()) {
            return sample_ggx_reflection_lobe<TFresnel, TCompensation>(
                make_ggx_dist(roughness_xy),
                fresnel,
                microfacet_roughness(roughness_xy),
                wi,
                rnd
            );
        }

        MxGGXDist dist = make_ggx_dist(roughness_xy);
        float3 m = dist.sample_for_refl(wi, rnd.x, rnd.y);
        const float cos_MO = dot(wi, m);
        if (cos_MO <= 0.0)
            return MxBsdfSample();
        const float3 F = fresnel.eval(cos_MO);
        const float Fmax = mx_hmax(F);
        const bool do_reflect = dorefl();
        const bool both_scatter = do_reflect && dorefr();
        // T-only advanced BSDFs must spend all samples on refraction. The
        // Fresnel roulette is only the branch probability when reflection and
        // refraction are both present in this BSDF.
        const float reflect_probability
            = both_scatter ? balance_probability(F, float3(1.0) - F) : (do_reflect ? 1.0 : 0.0);
        const bool choose_reflect = rnd.z < reflect_probability;
        const float3 wo = choose_reflect ? reflect(-wi, m) : mx_refract(wi, m, fresnel.refraction_eta());
        if ((choose_reflect && wo.z <= 0.0) || (!choose_reflect && wo.z >= 0.0))
            return MxBsdfSample();
        return spec_eval(wi, wo);
    }

    // tint(s.wo.z) is safe when s.wo == (0,0,0) (sample failure) because the
    // companion s.pdf == 0 zeros the weight regardless of which branch is picked.
    private float3 tint(float cos_NI) { return cos_NI > 0.0 ? reflection_tint : transmission_tint; }

    // BSDL: bsdf_dielectric_impl.h:285-299 DielectricLobe::eval_impl
    private MxBsdfSample eval_bsdf_sample(const float3 wi, const float3 wo)
    {
        if (!dorefl() && !dorefr())
            return MxBsdfSample();
        MxBsdfSample s = spec_eval(wi, wo);
        if (dorefr()) {
            s.weight *= TCompensation::refraction_scale(
                dorefl(),
                dorefr(),
                backfacing,
                fresnel.refraction_eta(),
                microfacet_roughness(roughness_xy),
                wi.z
            );
        }
        s.weight *= tint(s.wo.z);
        s.roughness = microfacet_roughness(roughness_xy);
        return s;
    }

    // BSDL: bsdf_dielectric_impl.h:301-319 DielectricLobe::sample_impl
    private MxBsdfSample sample_bsdf_sample(const float3 wi, const float3 rnd)
    {
        if (!dorefl() && !dorefr())
            return MxBsdfSample();
        MxBsdfSample s = spec_sample(wi, rnd);
        if (dorefr()) {
            s.weight *= TCompensation::refraction_scale(
                dorefl(),
                dorefr(),
                backfacing,
                fresnel.refraction_eta(),
                microfacet_roughness(roughness_xy),
                wi.z
            );
        }
        if (mx_hmax(abs(s.wo)) < BSDL_EPSILON)
            return MxBsdfSample();
        s.weight *= tint(s.wo.z);
        s.roughness = microfacet_roughness(roughness_xy);
        return s;
    }

    private BSDFFlags sample_lobe_types(const float3 wi, const float3 wo)
    {
        return wi.z * wo.z > 0.0 ? BSDFFlags::glossy_reflection : BSDFFlags::glossy_transmission;
    }

    [ForceInline]
    private float3 wi_absorption(const float3 wi)
    {
        if (mx_hmax(absorption) <= 0.0 || !dorefl() || dorefr())
            return float3(1.0);
        const float sin2_NO = 1.0 - wi.z * wi.z;
        const float inveta2 = (1.0 / fresnel.refraction_eta()) * (1.0 / fresnel.refraction_eta());
        const float cos_p = sqrt(1.0 - min(1.0, inveta2 * sin2_NO));
        const float dist = 1.0 / max(cos_p, float.MIN);
        return exp(-absorption * dist);
    }
};

public typealias MxDielectricBSDF
    = MxCompensatedDielectricBSDF<MxDielectricFresnel, MxTurquinLutDielectricCompensation, MxScatterRuntimeMode>;
public typealias MxDielectricTurquinAnalyticCompensationBSDF
    = MxCompensatedDielectricBSDF<MxDielectricFresnel, MxTurquinAnalyticDielectricCompensation, MxScatterRuntimeMode>;
public typealias MxDielectricNoCompensationBSDF
    = MxCompensatedDielectricBSDF<MxDielectricFresnel, MxNoCompensationDielectricCompensation, MxScatterRuntimeMode>;

public typealias MxDielectricAiryBSDF
    = MxCompensatedDielectricBSDF<MxDielectricAiryFresnel, MxTurquinLutDielectricCompensation, MxScatterRuntimeMode>;
public typealias MxDielectricAiryTurquinAnalyticCompensationBSDF = MxCompensatedDielectricBSDF<
    MxDielectricAiryFresnel,
    MxTurquinAnalyticDielectricCompensation,
    MxScatterRuntimeMode>;
public typealias MxDielectricAiryNoCompensationBSDF = MxCompensatedDielectricBSDF<
    MxDielectricAiryFresnel,
    MxNoCompensationDielectricCompensation,
    MxScatterRuntimeMode>;

public MxDielectricBSDF create_mx_dielectric(
    float3 tint,
    float ior,
    float2 roughness,
    bool retroreflective,
    float thinfilm_thickness,
    float thinfilm_ior,
    float3 normal,
    float3 tangent,
    int distribution,
    int scatter_mode,
    bool front_facing = true
)
{
    MxScatterTints scatter_tints = split_scatter_tints(scatter_mode, tint);
    float material_ior = clamp_dielectric_ior(ior);
    const bool backfacing = !front_facing;
    return MxDielectricBSDF(
        roughness,
        MxDielectricFresnel(material_ior, backfacing),
        scatter_tints.reflection,
        scatter_tints.transmission,
        float3(0.0),
        retroreflective,
        backfacing
    );
}

public MxDielectricAiryBSDF create_mx_dielectric_airy(
    float3 tint,
    float ior,
    float2 roughness,
    bool retroreflective,
    float thinfilm_thickness,
    float thinfilm_ior,
    float3 normal,
    float3 tangent,
    int distribution,
    int scatter_mode,
    bool front_facing = true
)
{
    MxScatterTints scatter_tints = split_scatter_tints(scatter_mode, tint);
    float material_ior = clamp_dielectric_ior(ior);
    const bool backfacing = !front_facing;
    return MxDielectricAiryBSDF(
        roughness,
        MxDielectricAiryFresnel(material_ior, backfacing, thinfilm_thickness, thinfilm_ior),
        scatter_tints.reflection,
        scatter_tints.transmission,
        float3(0.0),
        retroreflective,
        backfacing
    );
}

public MxDielectricTurquinAnalyticCompensationBSDF create_mx_dielectric_comp_turquin_analytic(
    float3 tint,
    float ior,
    float2 roughness,
    bool retroreflective,
    float thinfilm_thickness,
    float thinfilm_ior,
    float3 normal,
    float3 tangent,
    int distribution,
    int scatter_mode,
    bool front_facing = true
)
{
    MxScatterTints scatter_tints = split_scatter_tints(scatter_mode, tint);
    float material_ior = clamp_dielectric_ior(ior);
    const bool backfacing = !front_facing;
    return MxDielectricTurquinAnalyticCompensationBSDF(
        roughness,
        MxDielectricFresnel(material_ior, backfacing),
        scatter_tints.reflection,
        scatter_tints.transmission,
        float3(0.0),
        retroreflective,
        backfacing
    );
}

public MxDielectricNoCompensationBSDF create_mx_dielectric_comp_no_compensation(
    float3 tint,
    float ior,
    float2 roughness,
    bool retroreflective,
    float thinfilm_thickness,
    float thinfilm_ior,
    float3 normal,
    float3 tangent,
    int distribution,
    int scatter_mode,
    bool front_facing = true
)
{
    MxScatterTints scatter_tints = split_scatter_tints(scatter_mode, tint);
    float material_ior = clamp_dielectric_ior(ior);
    const bool backfacing = !front_facing;
    return MxDielectricNoCompensationBSDF(
        roughness,
        MxDielectricFresnel(material_ior, backfacing),
        scatter_tints.reflection,
        scatter_tints.transmission,
        float3(0.0),
        retroreflective,
        backfacing
    );
}

public MxDielectricAiryTurquinAnalyticCompensationBSDF create_mx_dielectric_airy_comp_turquin_analytic(
    float3 tint,
    float ior,
    float2 roughness,
    bool retroreflective,
    float thinfilm_thickness,
    float thinfilm_ior,
    float3 normal,
    float3 tangent,
    int distribution,
    int scatter_mode,
    bool front_facing = true
)
{
    MxScatterTints scatter_tints = split_scatter_tints(scatter_mode, tint);
    float material_ior = clamp_dielectric_ior(ior);
    const bool backfacing = !front_facing;
    return MxDielectricAiryTurquinAnalyticCompensationBSDF(
        roughness,
        MxDielectricAiryFresnel(material_ior, backfacing, thinfilm_thickness, thinfilm_ior),
        scatter_tints.reflection,
        scatter_tints.transmission,
        float3(0.0),
        retroreflective,
        backfacing
    );
}

public MxDielectricAiryNoCompensationBSDF create_mx_dielectric_airy_comp_no_compensation(
    float3 tint,
    float ior,
    float2 roughness,
    bool retroreflective,
    float thinfilm_thickness,
    float thinfilm_ior,
    float3 normal,
    float3 tangent,
    int distribution,
    int scatter_mode,
    bool front_facing = true
)
{
    MxScatterTints scatter_tints = split_scatter_tints(scatter_mode, tint);
    float material_ior = clamp_dielectric_ior(ior);
    const bool backfacing = !front_facing;
    return MxDielectricAiryNoCompensationBSDF(
        roughness,
        MxDielectricAiryFresnel(material_ior, backfacing, thinfilm_thickness, thinfilm_ior),
        scatter_tints.reflection,
        scatter_tints.transmission,
        float3(0.0),
        retroreflective,
        backfacing
    );
}

} // namespace mtlx
// File generated by MTLX codegen from a MaterialX 1.39 source.
// MaterialX graph nodes are emitted through the genslangpt target; C++ only wraps the generated closure graph.


namespace mtlx {

public struct Mx_Tiled_Brass1_01d3906d90ce_Data
{
    // Uniform graph inputs and texture handles.
    public TextureHandle image_color_file;
    public TextureHandle image_roughness_file;
};

#define MTLX_ADVANCED_LAYERING 1
struct MxStackData : IMaterialXMaterialData
{
    static const int bsdf_count = 2;
    static const int layer_count = 1;
    float3 bsdf_weights[2] = {};
    float3 bsdf_transmission_scales[2] = {};
    float3 layer_weights[1] = {};
    float3 layer_transmission_scales[1] = {};
    float3 mix_values[1] = {};
    float3 bsdf_n[2] = {};
    float3 bsdf_t[2] = {};
    float transmits[2] = {};
    bool front_facing = false;
    MxCompensatedDielectricBSDF<MxDielectricFresnel, MxTurquinAnalyticDielectricCompensation, MxScatterStaticRMode>
        bsdf0 = {};
    MxCompensatedConductorBSDF<MxConductorFresnel, MxTurquinAnalyticMicrofacetCompensation> bsdf1 = {};
    [mutating]
    void set_bsdf<TBSDF : IBSDF>(int bsdf_index, TBSDF bsdf)
    {
        switch (bsdf_index) {
        case 0:
            bsdf0 = reinterpret<
                MxCompensatedDielectricBSDF<
                    MxDielectricFresnel,
                    MxTurquinAnalyticDielectricCompensation,
                    MxScatterStaticRMode>,
                TBSDF>(bsdf);
            break;
        default:
            bsdf1 = reinterpret<
                MxCompensatedConductorBSDF<MxConductorFresnel, MxTurquinAnalyticMicrofacetCompensation>,
                TBSDF>(bsdf);
            break;
        }
    }
    [ForceInline]
    int get_bsdf_count() { return bsdf_count; }
    [ForceInline]
    int get_layer_count() { return layer_count; }
    [ForceInline]
    IBSDF get_bsdf(int bsdf_index)
    {
        switch (bsdf_index) {
        case 0:
            return bsdf0;
        default:
            return bsdf1;
        }
    }
    [ForceInline]
    float3 get_bsdf_weight(int bsdf_index) { return bsdf_weights[bsdf_index]; }
    [ForceInline]
    float3 get_layer_weight(int layer_index) { return layer_weights[layer_index]; }
    [ForceInline]
    float3 get_bsdf_n(int bsdf_index) { return bsdf_n[bsdf_index]; }
    [ForceInline]
    float3 get_bsdf_t(int bsdf_index) { return bsdf_t[bsdf_index]; }
    [ForceInline]
    float get_transmits(int bsdf_index) { return transmits[bsdf_index]; }
    [ForceInline]
    bool has_any_transmission() { return false; }
    [ForceInline]
    bool get_is_front_facing() { return front_facing; }
    [ForceInline]
    bool has_subsurface_data() { return false; }
    [ForceInline]
    MxSubsurfaceData get_subsurface_data() { return {}; }
    [ForceInline]
    float3 eval<S : ISampleGenerator>(int bsdf_index, const float3 wi, const float3 wo, inout S sg, BSDFContext bc)
    {
        switch (bsdf_index) {
        case 0:
            return bsdf0.eval(wi, wo, sg, bc);
        default:
            return bsdf1.eval(wi, wo, sg, bc);
        }
    }
    [ForceInline]
    BSDFEval
    eval_with_pdf<S : ISampleGenerator>(int bsdf_index, const float3 wi, const float3 wo, inout S sg, BSDFContext bc)
    {
        switch (bsdf_index) {
        case 0:
            return bsdf0.eval_with_pdf(wi, wo, sg, bc);
        default:
            return bsdf1.eval_with_pdf(wi, wo, sg, bc);
        }
    }
    [ForceInline]
    bool
    sample<S : ISampleGenerator>(int bsdf_index, const float3 wi, out BSDFSample sample, inout S sg, BSDFContext bc)
    {
        switch (bsdf_index) {
        case 0:
            return bsdf0.sample(wi, sample, sg, bc);
        default:
            return bsdf1.sample(wi, sample, sg, bc);
        }
    }
    [ForceInline]
    float eval_pdf(int bsdf_index, const float3 wi, const float3 wo, BSDFContext bc)
    {
        switch (bsdf_index) {
        case 0:
            return bsdf0.eval_pdf(wi, wo, bc);
        default:
            return bsdf1.eval_pdf(wi, wo, bc);
        }
    }
    [ForceInline]
    AlbedoContributions eval_albedo(int bsdf_index, const float3 wi)
    {
        bool3 active_channels = bsdf_weights[bsdf_index] > float3(0.0);
        // Advanced MaterialInstance albedo is a layer-program query, not a raw BSDF query.
        // Zero-weight closures can still carry invalid placeholder parameters, such as glTF sheen normalizing 0/0 when
        // sheen color is black. Treat zero-weight channels as absent before asking the IBSDF for albedo so their NaNs
        // cannot poison layer weights.
        if (!any(active_channels)) {
            return AlbedoContributions(float3(0.0), float3(0.0), float3(0.0), float3(0.0));
        }
        AlbedoContributions raw_albedo = AlbedoContributions(float3(0.0), float3(0.0), float3(0.0), float3(0.0));
        switch (bsdf_index) {
        case 0:
            raw_albedo = bsdf0.eval_albedo(wi);
            break;
        default:
            raw_albedo = bsdf1.eval_albedo(wi);
            break;
        }
        return AlbedoContributions(
            select(active_channels, raw_albedo.reflection, float3(0.0)),
            select(active_channels, raw_albedo.absorption_r, float3(0.0)),
            select(active_channels, raw_albedo.transmission, float3(0.0)),
            select(active_channels, raw_albedo.absorption_t, float3(0.0))
        );
    }
    [ForceInline]
    RoughnessInformation get_roughness_information(int bsdf_index, const float3 wi)
    {
        switch (bsdf_index) {
        case 0:
            return bsdf0.eval_roughness(wi);
        default:
            return bsdf1.eval_roughness(wi);
        }
    }
    [ForceInline]
    float3 get_ior_as_reflectance(int bsdf_index)
    {
        switch (bsdf_index) {
        case 0:
            return bsdf0.ior_as_reflectance;
        default:
            return bsdf1.ior_as_reflectance;
        }
    }
    [mutating]
    void advanced_multiply_closure_weight(int closure_index, float3 weight)
    {
        if (closure_index > 0) {
            bsdf_weights[closure_index - 1] *= weight;
        } else if (closure_index < 0) {
            layer_weights[-closure_index - 1] *= weight;
        }
    }
    [mutating]
    void apply_advanced_mix_weights() { }
    [mutating]
    void prepare_advanced_frames(Frame shading_frame_ws)
    {
        bsdf_n[0] = normalize(shading_frame_ws.to_global(bsdf_n[0]));
        bsdf_t[0] = normalize(shading_frame_ws.to_global(bsdf_t[0]));
        orthonormalize(bsdf_n[0], bsdf_t[0]);
        bsdf_n[1] = normalize(shading_frame_ws.to_global(bsdf_n[1]));
        bsdf_t[1] = normalize(shading_frame_ws.to_global(bsdf_t[1]));
        orthonormalize(bsdf_n[1], bsdf_t[1]);
    }
    [mutating]
    void prepare_advanced_layering(Frame shading_frame_ws)
    {
        apply_advanced_mix_weights();
        prepare_advanced_frames(shading_frame_ws);
    }
    [mutating]
    void init_vdf_transmission_scales()
    {
        bsdf_transmission_scales[0] = float3(1.0);
        bsdf_transmission_scales[1] = float3(1.0);
        layer_transmission_scales[0] = float3(1.0);
    }
}

struct BSDF
{
    uint index;
    [mutating]
    void set_bsdf(int i) { index = i; }
    [mutating]
    void set_layer(int i) { index = i | 0x80000000; }
    bool is_bsdf() { return (index & 0x80000000) == 0; }
    bool is_layer() { return (index & 0x80000000) != 0; }
    int get_index() { return index & ~0x80000000; }
}
struct EDF
{
    float3 radiance = {};
}
struct VDF
{
    float3 throughput = float3(1.0);
}
struct surfaceshader
{
    BSDF root = {};
    EDF edf = {};
    float opacity = 1.0;
    bool thin_walled = false;
}
struct volumeshader
{
    float3 color;
    float3 transparency;
};
struct displacementshader
{
    float3 offset;
    float scale;
};
struct lightshader
{
    float3 intensity;
    float3 direction;
};
#define material surfaceshader

struct ClosureData
{ }
static const int mx_space_model = 0;
static const int mx_space_object = 1;
static const int mx_space_world = 2;
#define M_FLOAT_EPS 1e-8

#define mx_inversesqrt rsqrt
#define mx_sin sin
#define mx_cos cos
#define mx_tan tan
#define mx_asin asin
#define mx_acos acos
#define mx_radians radians
#define mx_float_bits_to_int asint

/// The GLSL we are piggybacking on has all its matrices transposed compared to Slang and the MaterialX spec.
/// (The matrices are defined like float3x3(1, 2, 3, 4, 5, 6, 7, 8, 9) where the spec says it should be row-major order,
/// but GLSL creates it as col-major) So when GLSL code says "mul(M, v)" it means "v * transpose(M)", and since in Slang
/// the matrices are stored in row-major order, we need to reverse the order of multiplication to get the same result.
float2 mx_matrix_mul(float2 v, float2x2 m)
{
    return mul(m, v);
}
float3 mx_matrix_mul(float3 v, float3x3 m)
{
    return mul(m, v);
}
float4 mx_matrix_mul(float4 v, float4x4 m)
{
    return mul(m, v);
}
float2 mx_matrix_mul(float2x2 m, float2 v)
{
    return mul(v, m);
}
float3 mx_matrix_mul(float3x3 m, float3 v)
{
    return mul(v, m);
}
float4 mx_matrix_mul(float4x4 m, float4 v)
{
    return mul(v, m);
}
float2x2 mx_matrix_mul(float2x2 m1, float2x2 m2)
{
    return mul(m2, m1);
}
float3x3 mx_matrix_mul(float3x3 m1, float3x3 m2)
{
    return mul(m2, m1);
}
float4x4 mx_matrix_mul(float4x4 m1, float4x4 m2)
{
    return mul(m2, m1);
}

float mx_square(float x)
{
    return x * x;
}

float2 mx_square(float2 x)
{
    return x * x;
}

float3 mx_square(float3 x)
{
    return x * x;
}

float3 mx_srgb_encode(float3 color)
{
    bool3 isAbove = (color > float3(0.0031308));
    float3 linSeg = color * 12.92;
    float3 powSeg = 1.055 * pow(max(color, float3(0.0)), float3(1.0 / 2.4)) - 0.055;
    return select(isAbove, powSeg, linSeg);
}

/// Library assumes GLSL style mod (result has the sign of y), so cannot use fmod
float mx_mod(float a, float b)
{
    return (a - b * floor(a / b));
}
float2 mx_mod(float2 a, float2 b)
{
    return (a - b * floor(a / b));
}
float3 mx_mod(float3 a, float3 b)
{
    return (a - b * floor(a / b));
}
float4 mx_mod(float4 a, float4 b)
{
    return (a - b * floor(a / b));
}
float2 mx_mod(float2 a, float b)
{
    return (a - b * floor(a / b));
}
float3 mx_mod(float3 a, float b)
{
    return (a - b * floor(a / b));
}
float4 mx_mod(float4 a, float b)
{
    return (a - b * floor(a / b));
}

/// The float3x3 and float4x4 inverse are taken from the Slang's SGL library
/// https://github.com/shader-slang/sgl/
/// Specifically the matrix_math.h
/// https://github.com/shader-slang/sgl/blob/main/src/sgl/math/matrix_math.h
float3x3 mx_inverse(float3x3 m)
{
    float one_over_det = 1.f / determinant(m);

    float3x3 result;
    result[0][0] = +(m[1][1] * m[2][2] - m[1][2] * m[2][1]) * one_over_det;
    result[0][1] = -(m[0][1] * m[2][2] - m[0][2] * m[2][1]) * one_over_det;
    result[0][2] = +(m[0][1] * m[1][2] - m[0][2] * m[1][1]) * one_over_det;
    result[1][0] = -(m[1][0] * m[2][2] - m[1][2] * m[2][0]) * one_over_det;
    result[1][1] = +(m[0][0] * m[2][2] - m[0][2] * m[2][0]) * one_over_det;
    result[1][2] = -(m[0][0] * m[1][2] - m[0][2] * m[1][0]) * one_over_det;
    result[2][0] = +(m[1][0] * m[2][1] - m[1][1] * m[2][0]) * one_over_det;
    result[2][1] = -(m[0][0] * m[2][1] - m[0][1] * m[2][0]) * one_over_det;
    result[2][2] = +(m[0][0] * m[1][1] - m[0][1] * m[1][0]) * one_over_det;
    return result;
}

float4x4 mx_inverse(float4x4 m)
{
    float c00 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    float c02 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    float c03 = m[2][1] * m[3][2] - m[2][2] * m[3][1];

    float c04 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
    float c06 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
    float c07 = m[1][1] * m[3][2] - m[1][2] * m[3][1];

    float c08 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
    float c10 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
    float c11 = m[1][1] * m[2][2] - m[1][2] * m[2][1];

    float c12 = m[0][2] * m[3][3] - m[0][3] * m[3][2];
    float c14 = m[0][1] * m[3][3] - m[0][3] * m[3][1];
    float c15 = m[0][1] * m[3][2] - m[0][2] * m[3][1];

    float c16 = m[0][2] * m[2][3] - m[0][3] * m[2][2];
    float c18 = m[0][1] * m[2][3] - m[0][3] * m[2][1];
    float c19 = m[0][1] * m[2][2] - m[0][2] * m[2][1];

    float c20 = m[0][2] * m[1][3] - m[0][3] * m[1][2];
    float c22 = m[0][1] * m[1][3] - m[0][3] * m[1][1];
    float c23 = m[0][1] * m[1][2] - m[0][2] * m[1][1];

    float4 fac0 = float4(c00, c00, c02, c03);
    float4 fac1 = float4(c04, c04, c06, c07);
    float4 fac2 = float4(c08, c08, c10, c11);
    float4 fac3 = float4(c12, c12, c14, c15);
    float4 fac4 = float4(c16, c16, c18, c19);
    float4 fac5 = float4(c20, c20, c22, c23);

    float4 vec0 = float4(m[0][1], m[0][0], m[0][0], m[0][0]);
    float4 vec1 = float4(m[1][1], m[1][0], m[1][0], m[1][0]);
    float4 float2 = float4(m[2][1], m[2][0], m[2][0], m[2][0]);
    float4 float3 = float4(m[3][1], m[3][0], m[3][0], m[3][0]);

    float4 inv0 = float4(vec1 * fac0 - float2 * fac1 + float3 * fac2);
    float4 inv1 = float4(vec0 * fac0 - float2 * fac3 + float3 * fac4);
    float4 inv2 = float4(vec0 * fac1 - vec1 * fac3 + float3 * fac5);
    float4 inv3 = float4(vec0 * fac2 - vec1 * fac4 + float2 * fac5);

    float4 sign_a = float4(+1, -1, +1, -1);
    float4 sign_b = float4(-1, +1, -1, +1);
    // make matrix from columns
    float4x4 inverse = transpose(float4x4(inv0 * sign_a, inv1 * sign_b, inv2 * sign_a, inv3 * sign_b));

    float4 row0 = float4(inverse[0][0], inverse[0][1], inverse[0][2], inverse[0][3]);
    float4 col0 = float4(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

    float4 dot0 = float4(col0 * row0);
    float dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

    float one_over_det = 1.f / dot1;

    return inverse * one_over_det;
}

float mx_atan(float y_over_x)
{
    return atan(y_over_x);
}

float mx_atan(float y, float x)
{
    return atan2(y, x);
}

float2 mx_atan(float2 y, float2 x)
{
    return atan2(y, x);
}

float3 mx_atan(float3 y, float3 x)
{
    return atan2(y, x);
}

float4 mx_atan(float4 y, float4 x)
{
    return atan2(y, x);
}

struct Mx_Tiled_Brass1_01d3906d90ce_Graph<TLodSampler : ILodSampler>
{
    TextureHandle image_roughness_file;
    TextureHandle image_color_file;
    SurfaceInteraction si;
    uint hints;
    TLodSampler lod_sampler;
    float4x4 object_from_world;
    float4x4 world_from_object;
    float3x3 object_from_world_it;
    float3x3 world_from_object_it;
    ClosureData closure_data;
    int mx_stack_next_bsdf = 0;
    int mx_stack_next_layer = 0;
    MxStackData mx_stack = {};
    material out1 = {};

    __init(Mx_Tiled_Brass1_01d3906d90ce_Data data_, SurfaceInteraction si_, TLodSampler lod_sampler_, const uint hints_)
    {
        si = si_;
        lod_sampler = lod_sampler_;
        hints = hints_;
        object_from_world = GeomPropProvider::get_space_transform(si, GeomPropSpace::object, GeomPropSpace::world);
        world_from_object = GeomPropProvider::get_space_transform(si, GeomPropSpace::world, GeomPropSpace::object);
        object_from_world_it = (float3x3)transpose(world_from_object);
        world_from_object_it = (float3x3)transpose(object_from_world);
        image_color_file = data_.image_color_file;
        image_roughness_file = data_.image_roughness_file;
        mx_stack.init_vdf_transmission_scales();
        eval_shader_graph();
    }

    [mutating]
    void eval_shader_graph()
    {
        float2 geomprop_UV0_out1 = si.uv;
        float3 __mtlx_static_pruning_defaultgeom_SR_brass1_coat_normal_out = si.shading_frame_ws.normal;
        float3 __mtlx_static_pruning_defaultgeom_SR_brass1_normal_out = si.shading_frame_ws.normal;
        float3 __mtlx_static_pruning_defaultgeom_SR_brass1_tangent_out = mx_geomprop_tangent(si);
        float image_roughness_out = 0.0;
        NG_tiledimage_float(
            image_roughness_file,
            image_roughness_default,
            geomprop_UV0_out1,
            image_roughness_uvtiling,
            image_roughness_uvoffset,
            image_roughness_realworldimagesize,
            image_roughness_realworldtilesize,
            image_roughness_filtertype,
            image_roughness_framerange,
            image_roughness_frameoffset,
            image_roughness_frameendaction,
            image_roughness_out
        );
        float3 image_color_out = float3(0.0);
        NG_tiledimage_color3(
            image_color_file,
            image_color_default,
            geomprop_UV0_out1,
            image_color_uvtiling,
            image_color_uvoffset,
            image_color_realworldimagesize,
            image_color_realworldtilesize,
            image_color_filtertype,
            image_color_framerange,
            image_color_frameoffset,
            image_color_frameendaction,
            image_color_out
        );
        float3 image_color_out_cm_out = float3(0.0);
        NG_srgb_texture_to_lin_rec709_color3(image_color_out, image_color_out_cm_out);
        surfaceshader SR_brass1_out = {};
        NG_standard_surface_surfaceshader_100(
            SR_brass1_base,
            SR_brass1_base_color,
            SR_brass1_diffuse_roughness,
            SR_brass1_metalness,
            SR_brass1_specular,
            SR_brass1_specular_color,
            image_roughness_out,
            SR_brass1_specular_IOR,
            SR_brass1_specular_anisotropy,
            SR_brass1_specular_rotation,
            SR_brass1_transmission,
            SR_brass1_transmission_color,
            SR_brass1_transmission_depth,
            SR_brass1_transmission_scatter,
            SR_brass1_transmission_scatter_anisotropy,
            SR_brass1_transmission_dispersion,
            SR_brass1_transmission_extra_roughness,
            SR_brass1_subsurface,
            SR_brass1_subsurface_color,
            SR_brass1_subsurface_radius,
            SR_brass1_subsurface_scale,
            SR_brass1_subsurface_anisotropy,
            SR_brass1_sheen,
            SR_brass1_sheen_color,
            SR_brass1_sheen_roughness,
            SR_brass1_coat,
            image_color_out_cm_out,
            image_roughness_out,
            SR_brass1_coat_anisotropy,
            SR_brass1_coat_rotation,
            SR_brass1_coat_IOR,
            __mtlx_static_pruning_defaultgeom_SR_brass1_coat_normal_out,
            SR_brass1_coat_affect_color,
            SR_brass1_coat_affect_roughness,
            SR_brass1_thin_film_thickness,
            SR_brass1_thin_film_IOR,
            SR_brass1_emission,
            SR_brass1_emission_color,
            SR_brass1_opacity,
            SR_brass1_thin_walled,
            __mtlx_static_pruning_defaultgeom_SR_brass1_normal_out,
            __mtlx_static_pruning_defaultgeom_SR_brass1_tangent_out,
            SR_brass1_out
        );
        material Tiled_Brass_out = SR_brass1_out;
        out1 = Tiled_Brass_out;
    }

    /// @brief Returns whether MaterialX surface-lobe normals should be repaired.
    ///
    /// Keeping the policy behind this graph-local query lets configurations that always
    /// enable or disable the repair return a constant without storing material hints.
    bool mx_should_adjust_shading_normal()
    {
        return bool(MaterialInstanceHints(hints) & MaterialInstanceHints::adjust_shading_normal);
    }

    /// @brief Converts and optionally repairs a MaterialX surface-lobe normal.
    ///
    /// @param normal_ws BSDF MaterialX normal input in the generated shader's
    /// surface vector space, which is world space for the active SlangPT bindings.
    /// Normal/bump-map nodes have already expanded tangent-space texture values
    /// through their normal/tangent inputs before reaching this point.
    /// @return Normal in `si.shading_frame_ws` local coordinates. This is
    /// the space stored in `mx_stack.bsdf_n[]` and used to build each BSDF
    /// shading frame.
    float3 adjust_normal(float3 normal_ws)
    {
        const float3 candidate_normal_ls = normalize(si.shading_frame_ws.to_local(normal_ws));
        if (!mx_should_adjust_shading_normal())
            return candidate_normal_ls;

        const float3 view_direction_ls = normalize(si.shading_frame_ws.to_local(si.wi_ws));
        const float3 front_geometric_normal_ls = normalize(si.shading_frame_ws.to_local(si.front_normal_ws()));
        return adjust_shading_normal(candidate_normal_ls, front_geometric_normal_ls, view_direction_ls);
    }
    [mutating]
    void NG_tiledimage_float(
        TextureHandle file,
        float default1,
        float2 texcoord,
        float2 uvtiling,
        float2 uvoffset,
        float2 realworldimagesize,
        float2 realworldtilesize,
        int filtertype,
        int framerange,
        int frameoffset,
        int frameendaction,
        out float out1
    )
    {
        float2 N_mult_float_out = texcoord * uvtiling;
        float2 N_sub_float_out = N_mult_float_out - uvoffset;
        float2 N_divtilesize_float_out = N_sub_float_out / realworldimagesize;
        float2 N_multtilesize_float_out = N_divtilesize_float_out * realworldtilesize;
        float N_img_float_out
            = sample_texture(file, N_multtilesize_float_out, lod_sampler, float4(default1, 0.0, 0.0, 1.0)).x;
        out1 = N_img_float_out;
    }

    [mutating]
    void NG_tiledimage_color3(
        TextureHandle file,
        float3 default1,
        float2 texcoord,
        float2 uvtiling,
        float2 uvoffset,
        float2 realworldimagesize,
        float2 realworldtilesize,
        int filtertype,
        int framerange,
        int frameoffset,
        int frameendaction,
        out float3 out1
    )
    {
        float2 N_mult_color3_out = texcoord * uvtiling;
        float2 N_sub_color3_out = N_mult_color3_out - uvoffset;
        float2 N_divtilesize_color3_out = N_sub_color3_out / realworldimagesize;
        float2 N_multtilesize_color3_out = N_divtilesize_color3_out * realworldtilesize;
        float3 N_img_color3_out
            = sample_texture(file, N_multtilesize_color3_out, lod_sampler, float4(default1, 1.0)).xyz;
        out1 = N_img_color3_out;
    }

    [mutating]
    void NG_separate3_color3(float3 in1, out float outr, out float outg, out float outb)
    {
        const int N_extract_0_index_tmp = 0;
        float N_extract_0_out = in1[N_extract_0_index_tmp];
        const int N_extract_1_index_tmp = 1;
        float N_extract_1_out = in1[N_extract_1_index_tmp];
        const int N_extract_2_index_tmp = 2;
        float N_extract_2_out = in1[N_extract_2_index_tmp];
        outr = N_extract_0_out;
        outg = N_extract_1_out;
        outb = N_extract_2_out;
    }

    [mutating]
    void NG_srgb_texture_to_lin_rec709_color3(float3 in1, out float3 out1)
    {
        const float bias_in2_tmp = 0.055;
        float3 bias_out = in1 + bias_in2_tmp;
        const float linSeg_in2_tmp = 12.92;
        float3 linSeg_out = in1 / linSeg_in2_tmp;
        float colorSeparate_outr = 0.0;
        float colorSeparate_outg = 0.0;
        float colorSeparate_outb = 0.0;
        NG_separate3_color3(in1, colorSeparate_outr, colorSeparate_outg, colorSeparate_outb);
        const float max_in2_tmp = 0;
        float3 max_out = max(bias_out, max_in2_tmp);
        const float isAboveR_value2_tmp = 0.04045;
        const float isAboveR_in1_tmp = 1;
        const float isAboveR_in2_tmp = 0;
        float isAboveR_out = (colorSeparate_outr > isAboveR_value2_tmp) ? isAboveR_in1_tmp : isAboveR_in2_tmp;
        const float isAboveG_value2_tmp = 0.04045;
        const float isAboveG_in1_tmp = 1;
        const float isAboveG_in2_tmp = 0;
        float isAboveG_out = (colorSeparate_outg > isAboveG_value2_tmp) ? isAboveG_in1_tmp : isAboveG_in2_tmp;
        const float isAboveB_value2_tmp = 0.04045;
        const float isAboveB_in1_tmp = 1;
        const float isAboveB_in2_tmp = 0;
        float isAboveB_out = (colorSeparate_outb > isAboveB_value2_tmp) ? isAboveB_in1_tmp : isAboveB_in2_tmp;
        const float scale_in2_tmp = 1.055;
        float3 scale_out = max_out / scale_in2_tmp;
        float3 isAbove_out = float3(isAboveR_out, isAboveG_out, isAboveB_out);
        const float powSeg_in2_tmp = 2.4;
        float3 powSeg_out = pow(scale_out, float3(powSeg_in2_tmp));
        float3 mix_out = lerp(linSeg_out, powSeg_out, isAbove_out);
        out1 = mix_out;
    }

    // SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
    // SPDX-License-Identifier: Apache-2.0

    [mutating]
    void mx_roughness_anisotropy(float roughness, float anisotropy, out float2 result)
    {
        float roughness_sqr = clamp(roughness * roughness, M_FLOAT_EPS, 1.0);
        if (anisotropy > 0.0) {
            float aspect = sqrt(1.0 - clamp(anisotropy, 0.0, 0.98));
            result.x = min(roughness_sqr / aspect, 1.0);
            result.y = roughness_sqr * aspect;
        } else {
            result.x = roughness_sqr;
            result.y = roughness_sqr;
        }
    }

    [mutating]
    void mx_roughness_dual(float2 roughness, out float2 result)
    {
        result.x = clamp(roughness.x * roughness.x, M_FLOAT_EPS, 1.0);
        result.y = roughness.y < 0.0 ? result.x : clamp(roughness.y * roughness.y, M_FLOAT_EPS, 1.0);
    }

    [mutating]
    void mx_artistic_ior(float3 reflectivity, float3 edge_color, out float3 ior, out float3 extinction)
    {
        float3 r = clamp(reflectivity, 0.0, 0.99);
        float3 r_sqrt = sqrt(r);
        float3 n_min = (1.0 - r) / (1.0 + r);
        float3 n_max = (1.0 + r_sqrt) / (1.0 - r_sqrt);
        ior = lerp(n_max, n_min, edge_color);

        float3 np1 = ior + 1.0;
        float3 nm1 = ior - 1.0;
        float3 k2 = (np1 * np1 * r - nm1 * nm1) / (1.0 - r);
        k2 = max(k2, 0.0);
        extinction = sqrt(k2);
    }

    [mutating]
    void mx_deon_hair_absorption_from_melanin(
        float melanin_concentration,
        float melanin_redness,
        float3 eumelanin_color,
        float3 pheomelanin_color,
        out float3 absorption
    )
    {
        float melanin = -log(max(1.0 - melanin_concentration, 0.0001));
        float eumelanin = melanin * (1.0 - melanin_redness);
        float pheomelanin = melanin * melanin_redness;
        absorption = max(
            eumelanin * -log(max(eumelanin_color, float3(0.0001)))
                + pheomelanin * -log(max(pheomelanin_color, float3(0.0001))),
            float3(0.0)
        );
    }

    [mutating]
    void mx_chiang_hair_absorption_from_color(float3 color, float azimuthal_roughness, out float3 absorption)
    {
        float beta_n = clamp(azimuthal_roughness, 0.001, 1.0);
        float beta_n2 = beta_n * beta_n;
        float beta_n4 = beta_n2 * beta_n2;
        float b_fac = 5.969 - 0.215 * beta_n + 2.532 * beta_n2 - 10.73 * beta_n2 * beta_n + 5.574 * beta_n4
            + 0.245 * beta_n4 * beta_n;
        float3 sigma = log(clamp(color, float3(0.001), float3(1.0))) / b_fac;
        absorption = sigma * sigma;
    }

    [mutating]
    void mx_chiang_hair_roughness(
        float longitudinal,
        float azimuthal,
        float scale_TT,
        float scale_TRT,
        out float2 roughness_R,
        out float2 roughness_TT,
        out float2 roughness_TRT
    )
    {
        float lr = clamp(longitudinal, 0.001, 1.0);
        float ar = clamp(azimuthal, 0.001, 1.0);

        float v = 0.726 * lr + 0.812 * lr * lr + 3.7 * pow(lr, 20.0);
        v *= v;

        float s = 0.265 * ar + 1.194 * ar * ar + 5.372 * pow(ar, 22.0);

        roughness_R = float2(v, s);
        roughness_TT = float2(v * scale_TT * scale_TT, s);
        roughness_TRT = float2(v * scale_TRT * scale_TRT, s);
    }

    [mutating]
    void mx_luminance_color3(float3 _in, float3 lumacoeffs, out float3 result)
    {
        result = float3(dot(_in, lumacoeffs));
    }

    float4x4 mx_rotationMatrix(float3 axis, float angle)
    {
        axis = normalize(axis);
        float s = mx_sin(angle);
        float c = mx_cos(angle);
        float oc = 1.0 - c;

        return float4x4(
            oc * axis.x * axis.x + c,
            oc * axis.x * axis.y - axis.z * s,
            oc * axis.z * axis.x + axis.y * s,
            0.0,
            oc * axis.x * axis.y + axis.z * s,
            oc * axis.y * axis.y + c,
            oc * axis.y * axis.z - axis.x * s,
            0.0,
            oc * axis.z * axis.x - axis.y * s,
            oc * axis.y * axis.z + axis.x * s,
            oc * axis.z * axis.z + c,
            0.0,
            0.0,
            0.0,
            0.0,
            1.0
        );
    }

    [mutating]
    void mx_rotate_vector3(float3 _in, float amount, float3 axis, out float3 result)
    {
        float rotationRadians = mx_radians(amount);
        float4x4 m = mx_rotationMatrix(axis, rotationRadians);
        result = mx_matrix_mul(m, float4(_in, 1.0)).xyz;
    }

    // SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
    // SPDX-License-Identifier: Apache-2.0

    [mutating]
    void mx_layer_bsdf(ClosureData closure_data, BSDF top, BSDF base, out BSDF result)
    {
        result.set_layer(mx_stack_next_layer++);
        mx_stack.layer_weights[result.get_index()] = float3(1.0);
        mx_stack.mix_values[result.get_index()] = float3(0.0);
    }

    [mutating]
    void mx_weighted_layer_bsdf(ClosureData closure_data, BSDF top, float3 top_weight, BSDF base, out BSDF result)
    {
        result.set_layer(mx_stack_next_layer++);
        mx_stack.layer_weights[result.get_index()] = float3(1.0);
        mx_stack.mix_values[result.get_index()] = float3(0.0);

        float3 top_coverage = saturate(top_weight);
        if (top.is_bsdf())
            mx_stack.bsdf_weights[top.get_index()] *= top_coverage;
        else
            mx_stack.layer_weights[top.get_index()] *= top_coverage;
    }

    [mutating]
    void mx_mix_bsdf(ClosureData closure_data, BSDF fg, BSDF bg, float mix_value, out BSDF result)
    {
        mx_mix_bsdf(closure_data, fg, bg, float3(mix_value), result);
    }

    [mutating]
    void mx_mix_bsdf(ClosureData closure_data, BSDF fg, BSDF bg, float3 mix_value, out BSDF result)
    {
        result.set_layer(mx_stack_next_layer++);
        mx_stack.layer_weights[result.get_index()] = float3(1.0);
        mx_stack.mix_values[result.get_index()] = saturate(mix_value);
    }

    [mutating]
    void mx_add_bsdf(ClosureData closure_data, BSDF in1, BSDF in2, out BSDF result)
    {
        result.set_layer(mx_stack_next_layer++);
        mx_stack.layer_weights[result.get_index()] = float3(1.0);
        mx_stack.mix_values[result.get_index()] = float3(0.0);
    }

    [mutating]
    void mx_multiply_bsdf(ClosureData closure_data, BSDF in1, float3 in2, out BSDF result)
    {
        if (in1.is_bsdf())
            mx_stack.bsdf_weights[in1.get_index()] *= in2;
        else
            mx_stack.layer_weights[in1.get_index()] *= in2;
        result = in1;
    }

    [mutating]
    void mx_multiply_bsdf(ClosureData closure_data, BSDF in1, float in2, out BSDF result)
    {
        mx_multiply_bsdf(closure_data, in1, float3(in2), result);
    }

    [mutating]
    void mx_uniform_edf(ClosureData closure_data, float3 color, out EDF result) { result.radiance = color; }

    [mutating]
    void mx_multiply_edf(ClosureData closure_data, EDF in1, float3 in2, out EDF result)
    {
        result.radiance = in1.radiance * in2;
    }

    [mutating]
    void mx_multiply_edf(ClosureData closure_data, EDF in1, float in2, out EDF result)
    {
        result.radiance = in1.radiance * in2;
    }

    [mutating]
    void mx_add_edf(ClosureData closure_data, EDF in1, EDF in2, out EDF result)
    {
        result.radiance = in1.radiance + in2.radiance;
    }

    [mutating]
    void mx_mix_edf(ClosureData closure_data, EDF fg, EDF bg, float mix_value, out EDF result)
    {
        result.radiance = lerp(bg.radiance, fg.radiance, clamp(mix_value, 0.0, 1.0));
    }

    [mutating]
    void mx_mix_surfaceshader(surfaceshader fg, surfaceshader bg, float mix_value, out surfaceshader result)
    {
        float clamped_mix = clamp(mix_value, 0.0, 1.0);
        result.root.set_layer(mx_stack_next_layer++);
        mx_stack.layer_weights[result.root.get_index()] = float3(1.0);
        mx_stack.mix_values[result.root.get_index()] = float3(clamped_mix);
        result.opacity = lerp(bg.opacity, fg.opacity, clamped_mix);
        result.edf.radiance = lerp(bg.edf.radiance, fg.edf.radiance, clamped_mix);
        result.thin_walled = fg.thin_walled || bg.thin_walled;
    }

    [mutating]
    void mx_generalized_schlick_edf(
        ClosureData closure_data,
        float3 color0,
        float3 color90,
        float exponent,
        EDF base,
        out EDF result
    )
    {
        // MaterialX generalized_schlick_edf is directional: MaterialX 1.39 GLSL tints the base
        // EDF by a generalized Schlick factor using the view-normal angle.
        float3 normal = dot(si.shading_frame_ws.normal, si.wi_ws) < 0.0 ? -si.shading_frame_ws.normal
                                                                        : si.shading_frame_ws.normal;
        float n_dot_v = clamp(dot(normal, si.wi_ws), 1.0e-6, 1.0);
        float x = clamp(1.0 - n_dot_v, 0.0, 1.0);
        result.radiance = base.radiance * lerp(color0, color90, pow(x, exponent));
    }

    [mutating]
    void mx_blackbody(float temperature_kelvin, out float3 color_value)
    {
        float xc;
        float yc;

        temperature_kelvin = clamp(temperature_kelvin, 800.0, 25000.0);
        float t = 1000.0 / temperature_kelvin;
        float t2 = t * t;
        float t3 = t2 * t;

        if (temperature_kelvin < 4000.0)
            xc = -0.2661239 * t3 - 0.2343580 * t2 + 0.8776956 * t + 0.179910;
        else
            xc = -3.0258469 * t3 + 2.1070379 * t2 + 0.2226347 * t + 0.240390;

        float xc2 = xc * xc;
        float xc3 = xc2 * xc;

        if (temperature_kelvin < 2222.0)
            yc = -1.1063814 * xc3 - 1.34811020 * xc2 + 2.18555832 * xc - 0.20219683;
        else if (temperature_kelvin < 4000.0)
            yc = -0.9549476 * xc3 - 1.37418593 * xc2 + 2.09137015 * xc - 0.16748867;
        else
            yc = 3.0817580 * xc3 - 5.87338670 * xc2 + 3.75112997 * xc - 0.37001483;

        if (yc <= 0.0) {
            color_value = float3(1.0);
            return;
        }

        float3 xyz = float3(xc / yc, 1.0, (1.0 - xc - yc) / yc);
        float3x3 xyz_to_rgb = float3x3(3.2406, -0.9689, 0.0557, -1.5372, 1.8758, -0.2040, -0.4986, 0.0415, 1.0570);
        color_value = max(mx_matrix_mul(xyz_to_rgb, xyz), float3(0.0));
    }

    [mutating]
    void mx_surface(BSDF bsdf, EDF edf, float opacity, bool thin_walled, out surfaceshader result)
    {
        result.root = bsdf;
        result.opacity = clamp(opacity, 0.0, 1.0);
        result.edf.radiance = edf.radiance * result.opacity;
        result.thin_walled = thin_walled;
    }

    [mutating]
    void direct_prepare_bsdf(BSDF bsdf, float3 normal, float3 tangent)
    {
        const int bsdf_index = bsdf.get_index();
        mx_stack.bsdf_n[bsdf_index] = adjust_normal(normal);
        mx_stack.bsdf_t[bsdf_index] = si.shading_frame_ws.to_local(tangent);
        orthonormalize(mx_stack.bsdf_n[bsdf_index], mx_stack.bsdf_t[bsdf_index]);
#ifdef MTLX_STACK_HAS_BSDF_FRAMES
        mx_stack.bsdf_frames[bsdf_index] = Frame(mx_stack.bsdf_n[bsdf_index], mx_stack.bsdf_t[bsdf_index]);
#endif
    }
    [mutating]
    void NG_convert_float_color3(float in1, out float3 out1)
    {
        float3 combine_out = float3(in1, in1, in1);
        out1 = combine_out;
    }

    [mutating]
    void NG_standard_surface_surfaceshader_100(
        float base,
        float3 base_color,
        float diffuse_roughness,
        float metalness,
        float specular,
        float3 specular_color,
        float specular_roughness,
        float specular_IOR,
        float specular_anisotropy,
        float specular_rotation,
        float transmission,
        float3 transmission_color,
        float transmission_depth,
        float3 transmission_scatter,
        float transmission_scatter_anisotropy,
        float transmission_dispersion,
        float transmission_extra_roughness,
        float subsurface,
        float3 subsurface_color,
        float3 subsurface_radius,
        float subsurface_scale,
        float subsurface_anisotropy,
        float sheen,
        float3 sheen_color,
        float sheen_roughness,
        float coat,
        float3 coat_color,
        float coat_roughness,
        float coat_anisotropy,
        float coat_rotation,
        float coat_IOR,
        float3 coat_normal,
        float coat_affect_color,
        float coat_affect_roughness,
        float thin_film_thickness,
        float thin_film_IOR,
        float emission,
        float3 emission_color,
        float3 opacity,
        bool thin_walled,
        float3 normal,
        float3 tangent,
        out surfaceshader out1
    )
    {
        float2 coat_roughness_vector_out = float2(0.0);
        mx_roughness_anisotropy(coat_roughness, coat_anisotropy, coat_roughness_vector_out);
        const float coat_tangent_rotate_degree_in2_tmp = 360;
        float coat_tangent_rotate_degree_out = coat_rotation * coat_tangent_rotate_degree_in2_tmp;
        const float metalness_mix_fg_weight_in1_tmp = 1;
        float metalness_mix_fg_weight_out = metalness_mix_fg_weight_in1_tmp * metalness;
        float3 metal_reflectivity_out = base_color * base;
        float3 metal_edgecolor_out = specular_color * specular;
        float coat_affect_roughness_multiply1_out = coat_affect_roughness * coat;
        const float tangent_rotate_degree_in2_tmp = 360;
        float tangent_rotate_degree_out = specular_rotation * tangent_rotate_degree_in2_tmp;
        const float3 coat_attenuation_bg_tmp = float3(1, 1, 1);
        float3 coat_attenuation_out = lerp(coat_attenuation_bg_tmp, coat_color, coat);
        const float one_minus_coat_ior_in1_tmp = 1;
        float one_minus_coat_ior_out = one_minus_coat_ior_in1_tmp - coat_IOR;
        const float one_plus_coat_ior_in1_tmp = 1;
        float one_plus_coat_ior_out = one_plus_coat_ior_in1_tmp + coat_IOR;
        float3 emission_weight_out = emission_color * emission;
        float3 opacity_luminance_out = float3(0.0);
        mx_luminance_color3(opacity, float3(0.272229, 0.674082, 0.0536895), opacity_luminance_out);
        float3 coat_tangent_rotate_out = float3(0.0);
        mx_rotate_vector3(tangent, coat_tangent_rotate_degree_out, coat_normal, coat_tangent_rotate_out);
        float3 artistic_ior_ior = float3(0.0);
        float3 artistic_ior_extinction = float3(0.0);
        mx_artistic_ior(metal_reflectivity_out, metal_edgecolor_out, artistic_ior_ior, artistic_ior_extinction);
        float coat_affect_roughness_multiply2_out = coat_affect_roughness_multiply1_out * coat_roughness;
        float3 tangent_rotate_out = float3(0.0);
        mx_rotate_vector3(tangent, tangent_rotate_degree_out, normal, tangent_rotate_out);
        float coat_ior_to_F0_sqrt_out = one_minus_coat_ior_out / one_plus_coat_ior_out;
        const int opacity_luminance_float_index_tmp = 0;
        float opacity_luminance_float_out = opacity_luminance_out[opacity_luminance_float_index_tmp];
        float3 coat_tangent_rotate_normalize_out = normalize(coat_tangent_rotate_out);
        const float coat_affected_roughness_fg_tmp = 1;
        float coat_affected_roughness_out
            = lerp(specular_roughness, coat_affected_roughness_fg_tmp, coat_affect_roughness_multiply2_out);
        float3 tangent_rotate_normalize_out = normalize(tangent_rotate_out);
        float coat_ior_to_F0_out = coat_ior_to_F0_sqrt_out * coat_ior_to_F0_sqrt_out;
        const float coat_tangent_value2_tmp = 0;
        float3 coat_tangent_out
            = (coat_anisotropy > coat_tangent_value2_tmp) ? coat_tangent_rotate_normalize_out : tangent;
        float2 main_roughness_out = float2(0.0);
        mx_roughness_anisotropy(coat_affected_roughness_out, specular_anisotropy, main_roughness_out);
        const float main_tangent_value2_tmp = 0;
        float3 main_tangent_out
            = (specular_anisotropy > main_tangent_value2_tmp) ? tangent_rotate_normalize_out : tangent;
        const float one_minus_coat_ior_to_F0_in1_tmp = 1;
        float one_minus_coat_ior_to_F0_out = one_minus_coat_ior_to_F0_in1_tmp - coat_ior_to_F0_out;
        float3 emission_color0_out = float3(0.0);
        NG_convert_float_color3(one_minus_coat_ior_to_F0_out, emission_color0_out);
        BSDF coat_bsdf_out = { 0 };
        coat_bsdf_out.set_bsdf(mx_stack_next_bsdf++);
        mx_stack.bsdf_weights[coat_bsdf_out.get_index()] = float3(coat);
        mx_stack.set_bsdf(
            coat_bsdf_out.get_index(),
            MxCompensatedDielectricBSDF<
                MxDielectricFresnel,
                MxTurquinAnalyticDielectricCompensation,
                MxScatterStaticRMode>(
                coat_roughness_vector_out,
                MxDielectricFresnel(clamp_dielectric_ior(coat_IOR), !si.front_facing),
                float3(1, 1, 1),
                float3(0.0),
                float3(0.0),
                false,
                !si.front_facing
            )
        );
        direct_prepare_bsdf(coat_bsdf_out, coat_normal, coat_tangent_out);
        BSDF metal_bsdf_out = { 0 };
        metal_bsdf_out.set_bsdf(mx_stack_next_bsdf++);
        mx_stack.bsdf_weights[metal_bsdf_out.get_index()] = float3(metalness_mix_fg_weight_out);
        mx_stack.set_bsdf(
            metal_bsdf_out.get_index(),
            MxCompensatedConductorBSDF<MxConductorFresnel, MxTurquinAnalyticMicrofacetCompensation>(
                main_roughness_out,
                MxConductorFresnel(artistic_ior_ior, artistic_ior_extinction),
                false
            )
        );
        direct_prepare_bsdf(metal_bsdf_out, normal, main_tangent_out);
        BSDF thin_film_layer_attenuated_out = { 0 };
        mx_multiply_bsdf(closure_data, metal_bsdf_out, coat_attenuation_out, thin_film_layer_attenuated_out);
        BSDF coat_layer_out = { 0 };
        mx_layer_bsdf(closure_data, coat_bsdf_out, thin_film_layer_attenuated_out, coat_layer_out);
        EDF emission_edf_out = {};
        mx_uniform_edf(closure_data, emission_weight_out, emission_edf_out);
        EDF coat_tinted_emission_edf_out = {};
        mx_multiply_edf(closure_data, emission_edf_out, coat_color, coat_tinted_emission_edf_out);
        EDF coat_emission_edf_out = {};
        mx_generalized_schlick_edf(
            closure_data,
            emission_color0_out,
            float3(0, 0, 0),
            5,
            coat_tinted_emission_edf_out,
            coat_emission_edf_out
        );
        surfaceshader shader_constructor_out = {};
        mx_surface(coat_layer_out, coat_emission_edf_out, opacity_luminance_float_out, false, shader_constructor_out);
        out1 = shader_constructor_out;
    }

    static const surfaceshader backsurfaceshader = { 0 };
    static const displacementshader displacementshader1 = {};
    static const int geomprop_UV0_index = 0;
    static const int __mtlx_static_pruning_defaultgeom_SR_brass1_coat_normal_space = mx_space_world;
    static const int __mtlx_static_pruning_defaultgeom_SR_brass1_normal_space = mx_space_world;
    static const int __mtlx_static_pruning_defaultgeom_SR_brass1_tangent_space = mx_space_world;
    static const int __mtlx_static_pruning_defaultgeom_SR_brass1_tangent_index = 0;
    static const float image_roughness_default = 0;
    static const float2 image_roughness_uvtiling = float2(1, 1);
    static const float2 image_roughness_uvoffset = float2(0, 0);
    static const float2 image_roughness_realworldimagesize = float2(1, 1);
    static const float2 image_roughness_realworldtilesize = float2(1, 1);
    static const int image_roughness_filtertype = 1;
    static const int image_roughness_framerange = 0;
    static const int image_roughness_frameoffset = 0;
    static const int image_roughness_frameendaction = 0;
    static const float3 image_color_default = float3(0, 0, 0);
    static const float2 image_color_uvtiling = float2(1, 1);
    static const float2 image_color_uvoffset = float2(0, 0);
    static const float2 image_color_realworldimagesize = float2(1, 1);
    static const float2 image_color_realworldtilesize = float2(1, 1);
    static const int image_color_filtertype = 1;
    static const int image_color_framerange = 0;
    static const int image_color_frameoffset = 0;
    static const int image_color_frameendaction = 0;
    static const float SR_brass1_base = 1;
    static const float3 SR_brass1_base_color = float3(1, 1, 1);
    static const float SR_brass1_diffuse_roughness = 0;
    static const float SR_brass1_metalness = 1;
    static const float SR_brass1_specular = 0;
    static const float3 SR_brass1_specular_color = float3(1, 1, 1);
    static const float SR_brass1_specular_IOR = 1.5;
    static const float SR_brass1_specular_anisotropy = 0;
    static const float SR_brass1_specular_rotation = 0;
    static const float SR_brass1_transmission = 0;
    static const float3 SR_brass1_transmission_color = float3(1, 1, 1);
    static const float SR_brass1_transmission_depth = 0;
    static const float3 SR_brass1_transmission_scatter = float3(0, 0, 0);
    static const float SR_brass1_transmission_scatter_anisotropy = 0;
    static const float SR_brass1_transmission_dispersion = 0;
    static const float SR_brass1_transmission_extra_roughness = 0;
    static const float SR_brass1_subsurface = 0;
    static const float3 SR_brass1_subsurface_color = float3(1, 1, 1);
    static const float3 SR_brass1_subsurface_radius = float3(1, 1, 1);
    static const float SR_brass1_subsurface_scale = 1;
    static const float SR_brass1_subsurface_anisotropy = 0;
    static const float SR_brass1_sheen = 0;
    static const float3 SR_brass1_sheen_color = float3(1, 1, 1);
    static const float SR_brass1_sheen_roughness = 0.3;
    static const float SR_brass1_coat = 1;
    static const float SR_brass1_coat_anisotropy = 0;
    static const float SR_brass1_coat_rotation = 0;
    static const float SR_brass1_coat_IOR = 1.5;
    static const float SR_brass1_coat_affect_color = 0;
    static const float SR_brass1_coat_affect_roughness = 0;
    static const float SR_brass1_thin_film_thickness = 0;
    static const float SR_brass1_thin_film_IOR = 1.5;
    static const float SR_brass1_emission = 0;
    static const float3 SR_brass1_emission_color = float3(1, 1, 1);
    static const float3 SR_brass1_opacity = float3(1, 1, 1);
    static const bool SR_brass1_thin_walled = false;
}

float3 safe_layering_div(float3 numerator, float3 denominator)
{
    bool3 valid = abs(denominator) > float3(1e-8);
    float3 safe_denominator = select(valid, denominator, float3(1.0));
    return select(valid, numerator / safe_denominator, float3(0.0));
}

public struct MxAdvancedWeights_01d3906d90ce : IMxLayeredWeightCalculator
{
    public void
    calculate_weights<let TBsdfCount : int, let TLayerCount : int, TMxMaterialData : IMaterialXMaterialData>(
        const TMxMaterialData data,
        float3 wi_local[TBsdfCount],
        bool is_front_facing,
        out MxWeights<TBsdfCount> weights
    )
    {
        if (is_front_facing) {
            const float3 Wlb0 = data.get_bsdf_weight(0);
            const float3 Wlb1 = data.get_bsdf_weight(1);
            const float3 Wly0 = data.get_layer_weight(0);
            const AlbedoContributions a0 = data.eval_albedo(0, wi_local[0]);
            const float3 Alb0 = (a0.reflection + a0.absorption_r + a0.absorption_t) * Wlb0;


            const float3 n0 = (1 - Alb0);
            const float3 n1 = Wlb1;
            const float3 n2 = Wly0;
            const float3 n3 = Wlb0;
            const float3 n4 = (n0 * n1);
            const float3 n5 = (n2 * n3);
            const float3 n6 = (n2 * n4);
            weights.bsdf_weight[0] = n5;
            weights.bsdf_weight[1] = n6;
        } else {
            const float3 Wlb0 = data.get_bsdf_weight(0);
            const float3 Wlb1 = data.get_bsdf_weight(1);
            const float3 Wly0 = data.get_layer_weight(0);
            const AlbedoContributions a1 = data.eval_albedo(1, wi_local[1]);
            const float3 Alb1 = (a1.reflection + a1.absorption_r + a1.absorption_t) * Wlb1;


            const float3 n0 = (1 - Alb1);
            const float3 n1 = Wlb0;
            const float3 n2 = Wly0;
            const float3 n3 = Wlb1;
            const float3 n4 = (n0 * n1);
            const float3 n5 = (n2 * n4);
            const float3 n6 = (n2 * n3);
            weights.bsdf_weight[0] = n5;
            weights.bsdf_weight[1] = n6;
        }
    }
    public void
    calculate_plain_weights<let TBsdfCount : int, let TLayerCount : int, TMxMaterialData : IMaterialXMaterialData>(
        const TMxMaterialData data,
        out MxWeights<TBsdfCount> weights
    )
    {
        const float3 Wlb0 = data.get_bsdf_weight(0);
        const float3 Wlb1 = data.get_bsdf_weight(1);
        const float3 Wly0 = data.get_layer_weight(0);
        const float3 Alb1 = 0.0;


        const float3 n0 = (1 - Alb1);
        const float3 n1 = Wlb0;
        const float3 n2 = Wly0;
        const float3 n3 = Wlb1;
        const float3 n4 = (n0 * n1);
        const float3 n5 = (n2 * n4);
        const float3 n6 = (n2 * n3);
        weights.bsdf_weight[0] = n5;
        weights.bsdf_weight[1] = n6;
    }
};

#define MATERIAL_PAYLOAD_LIMIT 0
#define MATERIALX_PAYLOAD_SIZE 8
#define MATERIALX_HAS_EMISSION 1
#define MATERIALX_EVALUATE_OPACITY 0
#define MATERIALX_OPACITY_IS_OPAQUE 1
#define MATERIALX_STATIC_OPACITY 1.0

typealias Mx_Tiled_Brass1_01d3906d90ce_MaterialInstance
    = MxMaterialInstance<2, 1, MxStackData, MxAdvancedWeights_01d3906d90ce>;

}
// Buffer ABI for isolated evaluation. The texture handles use Falcor's normal
// bindless TextureHandle ABI; bind g_tiled_brass_data[0] to the generated
// MaterialX data record.
struct TiledBrassEvalInput
{
    float2 uv;
    float3 wi_ws;
    float3 wo_ws;
    uint seed;
};

struct TiledBrassEvalOutput
{
    float3 value;
    float pdf;
};

struct TiledBrassSampleInput
{
    float2 uv;
    float3 wi_ws;
    uint seed;
};

struct TiledBrassSampleOutput
{
    float3 wo_ws;
    float pdf;
    float3 weight;
    uint flags;
};

StructuredBuffer<mtlx::Mx_Tiled_Brass1_01d3906d90ce_Data> g_tiled_brass_data;
StructuredBuffer<TiledBrassEvalInput> g_eval_input;
RWStructuredBuffer<TiledBrassEvalOutput> g_eval_output;
StructuredBuffer<TiledBrassSampleInput> g_sample_input;
RWStructuredBuffer<TiledBrassSampleOutput> g_sample_output;
uint g_count;

SurfaceInteraction make_surface_interaction(float2 uv, float3 wi_ws)
{
    SurfaceInteraction si = {};
    si.uv = uv;
    si.wi_ws = normalize(wi_ws);
    si.normal_ws = float3(0.0, 0.0, 1.0);
    si.front_facing = dot(si.wi_ws, si.normal_ws) >= 0.0;
    si.shading_frame_ws = Frame::identity();
    return si;
}

mtlx::Mx_Tiled_Brass1_01d3906d90ce_MaterialInstance
make_material_instance(const mtlx::Mx_Tiled_Brass1_01d3906d90ce_Data data, const SurfaceInteraction si)
{
    SurfaceInteraction graph_si = si;
    if (!graph_si.front_facing)
        graph_si.shading_frame_ws.normal = -graph_si.shading_frame_ws.normal;
    let graph
        = mtlx::Mx_Tiled_Brass1_01d3906d90ce_Graph<ExplicitLodSampler>(data, graph_si, ExplicitLodSampler(0.0), 0u);
    mtlx::Mx_Tiled_Brass1_01d3906d90ce_MaterialInstance result = {};
    result.shading_frame_ws = graph_si.shading_frame_ws;
    result.data = graph.mx_stack;
    result.data.prepare_advanced_layering(result.shading_frame_ws);
    result.data.front_facing = si.front_facing;
    result.data.transmits[0] = 0.0;
    result.data.transmits[1] = 0.0;
    result.lobe_types = BSDFFlags::glossy_reflection;
    return result;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void eval_buffer(uint3 dispatch_thread_id: SV_DispatchThreadID)
{
    const uint index = dispatch_thread_id.x;
    if (index >= g_count)
        return;

    const TiledBrassEvalInput input = g_eval_input[index];
    const SurfaceInteraction si = make_surface_interaction(input.uv, input.wi_ws);
    let material = make_material_instance(g_tiled_brass_data[0], si);
    TinyUniformSampleGenerator sg = TinyUniformSampleGenerator(input.seed);
    const BSDFEval eval = material.eval_with_pdf(si, normalize(input.wo_ws), sg);
    g_eval_output[index] = TiledBrassEvalOutput(eval.value, eval.pdf);
}

[shader("compute")]
[numthreads(64, 1, 1)]
void sample_buffer(uint3 dispatch_thread_id: SV_DispatchThreadID)
{
    const uint index = dispatch_thread_id.x;
    if (index >= g_count)
        return;

    const TiledBrassSampleInput input = g_sample_input[index];
    const SurfaceInteraction si = make_surface_interaction(input.uv, input.wi_ws);
    let material = make_material_instance(g_tiled_brass_data[0], si);
    TinyUniformSampleGenerator sg = TinyUniformSampleGenerator(input.seed);
    MaterialSample sample = {};
    material.sample(si, sg, sample);
    g_sample_output[index] = TiledBrassSampleOutput(sample.wo_ws, sample.pdf, sample.weight, uint(sample.flags));
}
