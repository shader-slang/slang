// GLSL.std.450 extended instruction set implementation.
// Instruction numbers from the GLSL.std.450 spec (Table 1).
#include "../core/value.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace spvdb
{

// Instruction IDs from GLSL.std.450
enum GlslStd450 : uint32_t
{
    Round = 1,
    RoundEven = 2,
    Trunc = 3,
    FAbs = 4,
    SAbs = 5,
    FSign = 6,
    SSign = 7,
    Floor = 8,
    Ceil = 9,
    Fract = 10,
    Radians = 11,
    Degrees = 12,
    Sin = 13,
    Cos = 14,
    Tan = 15,
    Asin = 16,
    Acos = 17,
    Atan = 18,
    Sinh = 19,
    Cosh = 20,
    Tanh = 21,
    Asinh = 22,
    Acosh = 23,
    Atanh = 24,
    Atan2 = 25,
    Pow = 26,
    Exp = 27,
    Log = 28,
    Exp2 = 29,
    Log2 = 30,
    Sqrt = 31,
    InverseSqrt = 32,
    Determinant = 33,
    MatrixInverse = 34,
    Modf = 35,
    ModfStruct = 36,
    FMin = 37,
    UMin = 38,
    SMin = 39,
    FMax = 40,
    UMax = 41,
    SMax = 42,
    FClamp = 43,
    UClamp = 44,
    SClamp = 45,
    FMix = 46,
    IMix = 47,
    Step = 48,
    SmoothStep = 49,
    Fma = 50,
    Frexp = 51,
    FrexpStruct = 52,
    Ldexp = 53,
    PackSnorm4x8 = 54,
    PackUnorm4x8 = 55,
    PackSnorm2x16 = 56,
    PackUnorm2x16 = 57,
    PackHalf2x16 = 58,
    PackDouble2x32 = 59,
    UnpackSnorm2x16 = 60,
    UnpackUnorm2x16 = 61,
    UnpackHalf2x16 = 62,
    UnpackSnorm4x8 = 63,
    UnpackUnorm4x8 = 64,
    UnpackDouble2x32 = 65,
    Length = 66,
    Distance = 67,
    Cross = 68,
    Normalize = 69,
    FaceForward = 70,
    Reflect = 71,
    Refract = 72,
    FindILsb = 73,
    FindSMsb = 74,
    FindUMsb = 75,
    InterpolateAtCentroid = 76,
    InterpolateAtSample = 77,
    InterpolateAtOffset = 78,
    NMin = 79,
    NMax = 80,
    NClamp = 81,
};

// ---- float16 helpers -------------------------------------------------------

static uint16_t f32_to_f16(float f)
{
    uint32_t bits;
    std::memcpy(&bits, &f, 4);
    uint32_t sign = (bits >> 31) & 0x1u;
    int32_t exp = static_cast<int32_t>((bits >> 23) & 0xFFu) - 127 + 15;
    uint32_t mantissa = bits & 0x7FFFFFu;
    if (exp <= 0) {
        if (exp < -10)
            return static_cast<uint16_t>(sign << 15);
        mantissa = (mantissa | 0x800000u) >> (1 - exp);
        return static_cast<uint16_t>((sign << 15) | (mantissa >> 13));
    }
    if (exp >= 31)
        return static_cast<uint16_t>((sign << 15) | (31u << 10)); // inf
    return static_cast<uint16_t>((sign << 15) | (static_cast<uint32_t>(exp) << 10)
                                 | (mantissa >> 13));
}

static float f16_to_f32(uint16_t h)
{
    uint32_t sign = static_cast<uint32_t>((h >> 15) & 0x1u);
    uint32_t exp = static_cast<uint32_t>((h >> 10) & 0x1Fu);
    uint32_t mantissa = static_cast<uint32_t>(h & 0x3FFu);
    uint32_t bits;
    if (exp == 0) {
        if (mantissa == 0) {
            bits = sign << 31;
        } else {
            exp = 1;
            while (!(mantissa & 0x400u)) {
                mantissa <<= 1;
                exp--;
            }
            mantissa &= 0x3FFu;
            bits = (sign << 31) | ((exp + 127u - 15u) << 23) | (mantissa << 13);
        }
    } else if (exp == 31u) {
        bits = (sign << 31) | (0xFFu << 23) | (mantissa << 13);
    } else {
        bits = (sign << 31) | ((exp + 127u - 15u) << 23) | (mantissa << 13);
    }
    float result;
    std::memcpy(&result, &bits, 4);
    return result;
}

// ---- helpers ---------------------------------------------------------------

static bool is64(const Value &v)
{
    return v.kind == Value::Kind::Float64 || v.kind == Value::Kind::Int64
        || v.kind == Value::Kind::UInt64;
}

// Apply a unary float function element-wise.
static Value fmap1(const Value &a, float (*f32)(float), double (*f64)(double))
{
    if (a.kind == Value::Kind::Composite) {
        std::vector<Value> out;
        for (auto &e : a.elements) out.push_back(fmap1(e, f32, f64));
        return Value::make_composite(std::move(out));
    }
    if (a.kind == Value::Kind::Float32)
        return Value::make_f32(f32(a.scalar.f32));
    return Value::make_f64(f64(a.scalar.f64));
}

// Apply a binary float function element-wise.
static Value fmap2(const Value &a,
                   const Value &b,
                   float (*f32)(float, float),
                   double (*f64)(double, double))
{
    if (a.kind == Value::Kind::Composite) {
        std::vector<Value> out;
        for (size_t i = 0; i < a.elements.size(); ++i)
            out.push_back(fmap2(a.elements[i], b.elements[i], f32, f64));
        return Value::make_composite(std::move(out));
    }
    if (a.kind == Value::Kind::Float32)
        return Value::make_f32(f32(a.scalar.f32, b.scalar.f32));
    return Value::make_f64(f64(a.scalar.f64, b.scalar.f64));
}

static float vec_dot_f32(const Value &a, const Value &b)
{
    if (a.kind != Value::Kind::Composite)
        return a.scalar.f32 * b.scalar.f32;
    float s = 0;
    for (size_t i = 0; i < a.elements.size(); ++i)
        s += a.elements[i].scalar.f32 * b.elements[i].scalar.f32;
    return s;
}
static double vec_dot_f64(const Value &a, const Value &b)
{
    if (a.kind != Value::Kind::Composite)
        return a.scalar.f64 * b.scalar.f64;
    double s = 0;
    for (size_t i = 0; i < a.elements.size(); ++i)
        s += a.elements[i].scalar.f64 * b.elements[i].scalar.f64;
    return s;
}

static Value vec_length(const Value &v)
{
    if (v.kind == Value::Kind::Float32)
        return Value::make_f32(std::abs(v.scalar.f32));
    if (v.kind == Value::Kind::Float64)
        return Value::make_f64(std::abs(v.scalar.f64));
    bool is_f64 = !v.elements.empty() && v.elements[0].kind == Value::Kind::Float64;
    if (is_f64)
        return Value::make_f64(std::sqrt(vec_dot_f64(v, v)));
    return Value::make_f32(std::sqrt(vec_dot_f32(v, v)));
}

// ---- dispatch --------------------------------------------------------------

Value dispatch_glsl_std_450(uint32_t inst_id,
                            const std::vector<Value> &args,
                            std::vector<std::string> &diagnostics)
{
    auto &a0 = args.size() > 0 ? args[0]
                               : (const Value &) (*(const Value *) nullptr); // will guard below
    auto get = [&](size_t i) -> const Value &
    {
        static Value undef = Value::make_u32(0);
        return i < args.size() ? args[i] : undef;
    };

    switch (inst_id) {
        // --- Unary float ---
        case Round: return fmap1(get(0), std::round, std::round);
        case RoundEven: return fmap1(get(0), std::nearbyintf, std::nearbyint);
        case Trunc: return fmap1(get(0), std::trunc, std::trunc);
        case Floor: return fmap1(get(0), std::floor, std::floor);
        case Ceil: return fmap1(get(0), std::ceil, std::ceil);
        case FAbs: return fmap1(get(0), std::fabs, std::fabs);
        case FSign:
            {
                auto sgn = [](const Value &x) -> Value
                {
                    if (x.kind == Value::Kind::Float32)
                        return Value::make_f32(x.scalar.f32 > 0   ? 1.f
                                               : x.scalar.f32 < 0 ? -1.f
                                                                  : 0.f);
                    return Value::make_f64(x.scalar.f64 > 0 ? 1.0 : x.scalar.f64 < 0 ? -1.0 : 0.0);
                };
                const Value &x = get(0);
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (auto &e : x.elements) out.push_back(sgn(e));
                    return Value::make_composite(std::move(out));
                }
                return sgn(x);
            }
        case Fract:
            return fmap1(
                get(0),
                [](float v) { return v - std::floor(v); },
                [](double v) { return v - std::floor(v); });
        case Radians:
            return fmap1(
                get(0),
                [](float v) { return v * (3.14159265358979f / 180.f); },
                [](double v) { return v * (3.14159265358979323846 / 180.0); });
        case Degrees:
            return fmap1(
                get(0),
                [](float v) { return v * (180.f / 3.14159265358979f); },
                [](double v) { return v * (180.0 / 3.14159265358979323846); });
        case Sin: return fmap1(get(0), std::sin, std::sin);
        case Cos: return fmap1(get(0), std::cos, std::cos);
        case Tan: return fmap1(get(0), std::tan, std::tan);
        case Asin: return fmap1(get(0), std::asin, std::asin);
        case Acos: return fmap1(get(0), std::acos, std::acos);
        case Atan: return fmap1(get(0), std::atan, std::atan);
        case Sinh: return fmap1(get(0), std::sinh, std::sinh);
        case Cosh: return fmap1(get(0), std::cosh, std::cosh);
        case Tanh: return fmap1(get(0), std::tanh, std::tanh);
        case Asinh: return fmap1(get(0), std::asinhf, std::asinh);
        case Acosh: return fmap1(get(0), std::acoshf, std::acosh);
        case Atanh: return fmap1(get(0), std::atanhf, std::atanh);
        case Exp: return fmap1(get(0), std::exp, std::exp);
        case Log: return fmap1(get(0), std::log, std::log);
        case Exp2: return fmap1(get(0), std::exp2, std::exp2);
        case Log2: return fmap1(get(0), std::log2, std::log2);
        case Sqrt: return fmap1(get(0), std::sqrt, std::sqrt);
        case InverseSqrt:
            return fmap1(
                get(0),
                [](float v) { return 1.f / std::sqrt(v); },
                [](double v) { return 1.0 / std::sqrt(v); });

        // --- Binary float ---
        case Atan2: return fmap2(get(0), get(1), std::atan2, std::atan2);
        case Pow: return fmap2(get(0), get(1), std::pow, std::pow);
        case FMin: return fmap2(get(0), get(1), std::fminf, std::fmin);
        case FMax: return fmap2(get(0), get(1), std::fmaxf, std::fmax);
        case NMin:
            return fmap2(get(0), get(1), std::fminf, std::fmin); // NaN-propagating = same on IEEE
        case NMax: return fmap2(get(0), get(1), std::fmaxf, std::fmax);

        // --- Integer ---
        case SAbs:
            {
                const Value &x = get(0);
                auto iabs = [](const Value &v) -> Value
                {
                    if (v.kind == Value::Kind::Int32)
                        return Value::make_i32(std::abs(v.scalar.i32));
                    return Value::make_i64(std::abs(v.scalar.i64));
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (auto &e : x.elements) out.push_back(iabs(e));
                    return Value::make_composite(std::move(out));
                }
                return iabs(x);
            }
        case SSign:
            {
                const Value &x = get(0);
                auto sgn = [](const Value &v) -> Value
                {
                    if (v.kind == Value::Kind::Int32)
                        return Value::make_i32(v.scalar.i32 > 0 ? 1 : v.scalar.i32 < 0 ? -1 : 0);
                    return Value::make_i64(v.scalar.i64 > 0 ? 1 : v.scalar.i64 < 0 ? -1 : 0);
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (auto &e : x.elements) out.push_back(sgn(e));
                    return Value::make_composite(std::move(out));
                }
                return sgn(x);
            }
        case UMin:
            {
                const Value &a = get(0);
                const Value &b = get(1);
                auto op = [](const Value &x, const Value &y) -> Value
                {
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_u32(x.scalar.u32 < y.scalar.u32 ? x.scalar.u32
                                                                           : y.scalar.u32);
                    return Value::make_u64(x.scalar.u64 < y.scalar.u64 ? x.scalar.u64
                                                                       : y.scalar.u64);
                };
                if (a.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (size_t i = 0; i < a.elements.size(); ++i)
                        out.push_back(op(a.elements[i], b.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return op(a, b);
            }
        case UMax:
            {
                const Value &a = get(0);
                const Value &b = get(1);
                auto op = [](const Value &x, const Value &y) -> Value
                {
                    if (x.kind == Value::Kind::UInt32)
                        return Value::make_u32(x.scalar.u32 > y.scalar.u32 ? x.scalar.u32
                                                                           : y.scalar.u32);
                    return Value::make_u64(x.scalar.u64 > y.scalar.u64 ? x.scalar.u64
                                                                       : y.scalar.u64);
                };
                if (a.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (size_t i = 0; i < a.elements.size(); ++i)
                        out.push_back(op(a.elements[i], b.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return op(a, b);
            }
        case SMin:
            {
                const Value &a = get(0);
                const Value &b = get(1);
                auto op = [](const Value &x, const Value &y) -> Value
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_i32(x.scalar.i32 < y.scalar.i32 ? x.scalar.i32
                                                                           : y.scalar.i32);
                    return Value::make_i64(x.scalar.i64 < y.scalar.i64 ? x.scalar.i64
                                                                       : y.scalar.i64);
                };
                if (a.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (size_t i = 0; i < a.elements.size(); ++i)
                        out.push_back(op(a.elements[i], b.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return op(a, b);
            }
        case SMax:
            {
                const Value &a = get(0);
                const Value &b = get(1);
                auto op = [](const Value &x, const Value &y) -> Value
                {
                    if (x.kind == Value::Kind::Int32)
                        return Value::make_i32(x.scalar.i32 > y.scalar.i32 ? x.scalar.i32
                                                                           : y.scalar.i32);
                    return Value::make_i64(x.scalar.i64 > y.scalar.i64 ? x.scalar.i64
                                                                       : y.scalar.i64);
                };
                if (a.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (size_t i = 0; i < a.elements.size(); ++i)
                        out.push_back(op(a.elements[i], b.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return op(a, b);
            }

        // --- Clamp ---
        case FClamp:
            {
                const Value &x = get(0);
                const Value &lo = get(1);
                const Value &hi = get(2);
                auto clamp = [](const Value &v, const Value &l, const Value &h) -> Value
                {
                    if (v.kind == Value::Kind::Float32)
                        return Value::make_f32(
                            std::fminf(std::fmaxf(v.scalar.f32, l.scalar.f32), h.scalar.f32));
                    return Value::make_f64(
                        std::fmin(std::fmax(v.scalar.f64, l.scalar.f64), h.scalar.f64));
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    bool lo_scalar = lo.kind != Value::Kind::Composite;
                    bool hi_scalar = hi.kind != Value::Kind::Composite;
                    for (size_t i = 0; i < x.elements.size(); ++i)
                        out.push_back(clamp(x.elements[i],
                                            lo_scalar ? lo : lo.elements[i],
                                            hi_scalar ? hi : hi.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return clamp(x, lo, hi);
            }
        case NClamp:
            {
                // Same as FClamp but NaN-propagating (implementation identical on IEEE).
                const Value &x = get(0);
                const Value &lo = get(1);
                const Value &hi = get(2);
                auto clamp = [](const Value &v, const Value &l, const Value &h) -> Value
                {
                    if (v.kind == Value::Kind::Float32)
                        return Value::make_f32(
                            std::fminf(std::fmaxf(v.scalar.f32, l.scalar.f32), h.scalar.f32));
                    return Value::make_f64(
                        std::fmin(std::fmax(v.scalar.f64, l.scalar.f64), h.scalar.f64));
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    bool lo_s = lo.kind != Value::Kind::Composite;
                    bool hi_s = hi.kind != Value::Kind::Composite;
                    for (size_t i = 0; i < x.elements.size(); ++i)
                        out.push_back(clamp(x.elements[i],
                                            lo_s ? lo : lo.elements[i],
                                            hi_s ? hi : hi.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return clamp(x, lo, hi);
            }
        case SClamp:
            {
                const Value &x = get(0);
                const Value &lo = get(1);
                const Value &hi = get(2);
                auto clamp = [](const Value &v, const Value &l, const Value &h) -> Value
                {
                    if (v.kind == Value::Kind::Int32) {
                        int32_t r = v.scalar.i32 < l.scalar.i32 ? l.scalar.i32
                                  : v.scalar.i32 > h.scalar.i32 ? h.scalar.i32
                                                                : v.scalar.i32;
                        return Value::make_i32(r);
                    }
                    int64_t r = v.scalar.i64 < l.scalar.i64 ? l.scalar.i64
                              : v.scalar.i64 > h.scalar.i64 ? h.scalar.i64
                                                            : v.scalar.i64;
                    return Value::make_i64(r);
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (size_t i = 0; i < x.elements.size(); ++i)
                        out.push_back(clamp(x.elements[i], lo.elements[i], hi.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return clamp(x, lo, hi);
            }
        case UClamp:
            {
                const Value &x = get(0);
                const Value &lo = get(1);
                const Value &hi = get(2);
                auto clamp = [](const Value &v, const Value &l, const Value &h) -> Value
                {
                    if (v.kind == Value::Kind::UInt32) {
                        uint32_t r = v.scalar.u32 < l.scalar.u32 ? l.scalar.u32
                                   : v.scalar.u32 > h.scalar.u32 ? h.scalar.u32
                                                                 : v.scalar.u32;
                        return Value::make_u32(r);
                    }
                    uint64_t r = v.scalar.u64 < l.scalar.u64 ? l.scalar.u64
                               : v.scalar.u64 > h.scalar.u64 ? h.scalar.u64
                                                             : v.scalar.u64;
                    return Value::make_u64(r);
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (size_t i = 0; i < x.elements.size(); ++i)
                        out.push_back(clamp(x.elements[i], lo.elements[i], hi.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return clamp(x, lo, hi);
            }

        // --- Mix/Step/SmoothStep ---
        case FMix:
            {
                const Value &x = get(0);
                const Value &y = get(1);
                const Value &a = get(2);
                auto mix = [](const Value &xi, const Value &yi, const Value &ai) -> Value
                {
                    if (xi.kind == Value::Kind::Float32)
                        return Value::make_f32(xi.scalar.f32
                                               + ai.scalar.f32 * (yi.scalar.f32 - xi.scalar.f32));
                    return Value::make_f64(xi.scalar.f64
                                           + ai.scalar.f64 * (yi.scalar.f64 - xi.scalar.f64));
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    bool a_scalar = a.kind != Value::Kind::Composite;
                    for (size_t i = 0; i < x.elements.size(); ++i)
                        out.push_back(
                            mix(x.elements[i], y.elements[i], a_scalar ? a : a.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return mix(x, y, a);
            }
        case Step:
            {
                const Value &edge = get(0);
                const Value &x = get(1);
                auto step = [](const Value &e, const Value &xi) -> Value
                {
                    if (xi.kind == Value::Kind::Float32)
                        return Value::make_f32(xi.scalar.f32 < e.scalar.f32 ? 0.f : 1.f);
                    return Value::make_f64(xi.scalar.f64 < e.scalar.f64 ? 0.0 : 1.0);
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    bool e_scalar = edge.kind != Value::Kind::Composite;
                    for (size_t i = 0; i < x.elements.size(); ++i)
                        out.push_back(step(e_scalar ? edge : edge.elements[i], x.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return step(edge, x);
            }
        case SmoothStep:
            {
                const Value &edge0 = get(0);
                const Value &edge1 = get(1);
                const Value &x = get(2);
                auto smooth = [](const Value &e0, const Value &e1, const Value &xi) -> Value
                {
                    if (xi.kind == Value::Kind::Float32) {
                        float t = std::fminf(std::fmaxf((xi.scalar.f32 - e0.scalar.f32)
                                                            / (e1.scalar.f32 - e0.scalar.f32),
                                                        0.f),
                                             1.f);
                        return Value::make_f32(t * t * (3.f - 2.f * t));
                    }
                    double t = std::fmin(std::fmax((xi.scalar.f64 - e0.scalar.f64)
                                                       / (e1.scalar.f64 - e0.scalar.f64),
                                                   0.0),
                                         1.0);
                    return Value::make_f64(t * t * (3.0 - 2.0 * t));
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    bool e_scalar = edge0.kind != Value::Kind::Composite;
                    for (size_t i = 0; i < x.elements.size(); ++i)
                        out.push_back(smooth(e_scalar ? edge0 : edge0.elements[i],
                                             e_scalar ? edge1 : edge1.elements[i],
                                             x.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return smooth(edge0, edge1, x);
            }

        // --- Geometric ---
        case Length: return vec_length(get(0));
        case Distance:
            {
                const Value &a = get(0);
                const Value &b = get(1);
                // diff = a - b
                if (a.kind == Value::Kind::Composite) {
                    std::vector<Value> diff;
                    for (size_t i = 0; i < a.elements.size(); ++i) {
                        if (a.elements[i].kind == Value::Kind::Float32)
                            diff.push_back(Value::make_f32(a.elements[i].scalar.f32
                                                           - b.elements[i].scalar.f32));
                        else
                            diff.push_back(Value::make_f64(a.elements[i].scalar.f64
                                                           - b.elements[i].scalar.f64));
                    }
                    Value d = Value::make_composite(std::move(diff));
                    return vec_length(d);
                }
                if (a.kind == Value::Kind::Float32)
                    return Value::make_f32(std::fabs(a.scalar.f32 - b.scalar.f32));
                return Value::make_f64(std::fabs(a.scalar.f64 - b.scalar.f64));
            }
        case Cross:
            {
                // Only for vec3.
                const Value &a = get(0);
                const Value &b = get(1);
                if (a.kind != Value::Kind::Composite || a.elements.size() < 3)
                    return Value::make_u32(0);
                bool f64 = a.elements[0].kind == Value::Kind::Float64;
                if (!f64) {
                    float ax = a.elements[0].scalar.f32, ay = a.elements[1].scalar.f32,
                          az = a.elements[2].scalar.f32;
                    float bx = b.elements[0].scalar.f32, by = b.elements[1].scalar.f32,
                          bz = b.elements[2].scalar.f32;
                    return Value::make_composite({Value::make_f32(ay * bz - az * by),
                                                  Value::make_f32(az * bx - ax * bz),
                                                  Value::make_f32(ax * by - ay * bx)});
                }
                double ax = a.elements[0].scalar.f64, ay = a.elements[1].scalar.f64,
                       az = a.elements[2].scalar.f64;
                double bx = b.elements[0].scalar.f64, by = b.elements[1].scalar.f64,
                       bz = b.elements[2].scalar.f64;
                return Value::make_composite({Value::make_f64(ay * bz - az * by),
                                              Value::make_f64(az * bx - ax * bz),
                                              Value::make_f64(ax * by - ay * bx)});
            }
        case Normalize:
            {
                const Value &v = get(0);
                Value len = vec_length(v);
                if (v.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (auto &e : v.elements) {
                        if (e.kind == Value::Kind::Float32)
                            out.push_back(Value::make_f32(e.scalar.f32 / len.scalar.f32));
                        else
                            out.push_back(Value::make_f64(e.scalar.f64 / len.scalar.f64));
                    }
                    return Value::make_composite(std::move(out));
                }
                if (v.kind == Value::Kind::Float32)
                    return Value::make_f32(v.scalar.f32 / len.scalar.f32);
                return Value::make_f64(v.scalar.f64 / len.scalar.f64);
            }
        case FaceForward:
            {
                // N if dot(Nref, I) < 0, else -N
                const Value &N = get(0);
                const Value &I = get(1);
                const Value &Nref = get(2);
                bool f64 = N.kind == Value::Kind::Composite && !N.elements.empty()
                        && N.elements[0].kind == Value::Kind::Float64;
                float d32 = vec_dot_f32(Nref, I);
                double d64 = vec_dot_f64(Nref, I);
                bool negate = f64 ? (d64 >= 0) : (d32 >= 0);
                if (!negate)
                    return N;
                if (N.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (auto &e : N.elements) {
                        if (e.kind == Value::Kind::Float32)
                            out.push_back(Value::make_f32(-e.scalar.f32));
                        else
                            out.push_back(Value::make_f64(-e.scalar.f64));
                    }
                    return Value::make_composite(std::move(out));
                }
                return N.kind == Value::Kind::Float32 ? Value::make_f32(-N.scalar.f32)
                                                      : Value::make_f64(-N.scalar.f64);
            }
        case Reflect:
            {
                // I - 2 * dot(N, I) * N
                const Value &I = get(0);
                const Value &N = get(1);
                bool f64 = N.kind == Value::Kind::Composite && !N.elements.empty()
                        && N.elements[0].kind == Value::Kind::Float64;
                if (f64) {
                    double d = 2.0 * vec_dot_f64(N, I);
                    if (I.kind == Value::Kind::Composite) {
                        std::vector<Value> out;
                        for (size_t i = 0; i < I.elements.size(); ++i)
                            out.push_back(Value::make_f64(I.elements[i].scalar.f64
                                                          - d * N.elements[i].scalar.f64));
                        return Value::make_composite(std::move(out));
                    }
                    return Value::make_f64(I.scalar.f64 - d * N.scalar.f64);
                } else {
                    float d = 2.f * vec_dot_f32(N, I);
                    if (I.kind == Value::Kind::Composite) {
                        std::vector<Value> out;
                        for (size_t i = 0; i < I.elements.size(); ++i)
                            out.push_back(Value::make_f32(I.elements[i].scalar.f32
                                                          - d * N.elements[i].scalar.f32));
                        return Value::make_composite(std::move(out));
                    }
                    return Value::make_f32(I.scalar.f32 - d * N.scalar.f32);
                }
            }
        case Refract:
            {
                const Value &I = get(0);
                const Value &N = get(1);
                const Value &eta_val = get(2);
                float eta = eta_val.scalar.f32;
                float dNI = vec_dot_f32(N, I);
                float k = 1.f - eta * eta * (1.f - dNI * dNI);
                if (k < 0.f) {
                    // Total internal reflection — return zero vector.
                    if (I.kind == Value::Kind::Composite) {
                        std::vector<Value> out(I.elements.size(), Value::make_f32(0.f));
                        return Value::make_composite(std::move(out));
                    }
                    return Value::make_f32(0.f);
                }
                float coef = eta * dNI + std::sqrt(k);
                if (I.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (size_t i = 0; i < I.elements.size(); ++i)
                        out.push_back(Value::make_f32(eta * I.elements[i].scalar.f32
                                                      - coef * N.elements[i].scalar.f32));
                    return Value::make_composite(std::move(out));
                }
                return Value::make_f32(eta * I.scalar.f32 - coef * N.scalar.f32);
            }

        // --- Modf ---
        case ModfStruct:
        case Modf:
            {
                const Value &x = get(0);
                auto do_modf = [&](float v) -> std::pair<float, float>
                {
                    float i;
                    float f = std::modf(v, &i);
                    return {f, i};
                };
                auto do_modf64 = [&](double v) -> std::pair<double, double>
                {
                    double i;
                    double f = std::modf(v, &i);
                    return {f, i};
                };
                if (x.kind == Value::Kind::Float32) {
                    auto [frac, intpart] = do_modf(x.scalar.f32);
                    return Value::make_composite({Value::make_f32(frac), Value::make_f32(intpart)});
                }
                if (x.kind == Value::Kind::Float64) {
                    auto [frac, intpart] = do_modf64(x.scalar.f64);
                    return Value::make_composite({Value::make_f64(frac), Value::make_f64(intpart)});
                }
                if (x.kind == Value::Kind::Composite) {
                    // Return struct of {fract_vec, int_vec}
                    std::vector<Value> fv, iv;
                    for (auto &e : x.elements) {
                        if (e.kind == Value::Kind::Float32) {
                            auto [f, i] = do_modf(e.scalar.f32);
                            fv.push_back(Value::make_f32(f));
                            iv.push_back(Value::make_f32(i));
                        } else {
                            auto [f, i] = do_modf64(e.scalar.f64);
                            fv.push_back(Value::make_f64(f));
                            iv.push_back(Value::make_f64(i));
                        }
                    }
                    return Value::make_composite({Value::make_composite(std::move(fv)),
                                                  Value::make_composite(std::move(iv))});
                }
                return Value::make_u32(0);
            }

        // --- Fma (proper) ---
        case Fma:
            {
                const Value &a = get(0);
                const Value &b = get(1);
                const Value &c = get(2);
                auto fma_op = [](const Value &ai, const Value &bi, const Value &ci) -> Value
                {
                    if (ai.kind == Value::Kind::Float32)
                        return Value::make_f32(
                            std::fmaf(ai.scalar.f32, bi.scalar.f32, ci.scalar.f32));
                    return Value::make_f64(std::fma(ai.scalar.f64, bi.scalar.f64, ci.scalar.f64));
                };
                if (a.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (size_t i = 0; i < a.elements.size(); ++i)
                        out.push_back(fma_op(a.elements[i], b.elements[i], c.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return fma_op(a, b, c);
            }

        // --- Ldexp ---
        case Ldexp:
            {
                const Value &x = get(0);
                const Value &exp = get(1);
                auto op = [](const Value &xi, const Value &ei) -> Value
                {
                    if (xi.kind == Value::Kind::Float32)
                        return Value::make_f32(std::ldexp(xi.scalar.f32, ei.scalar.i32));
                    return Value::make_f64(std::ldexp(xi.scalar.f64, ei.scalar.i32));
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (size_t i = 0; i < x.elements.size(); ++i)
                        out.push_back(op(x.elements[i], exp.elements[i]));
                    return Value::make_composite(std::move(out));
                }
                return op(x, exp);
            }

        // --- FindLsb / FindMsb ---
        case FindILsb:
            {
                const Value &x = get(0);
                auto lsb = [](const Value &v) -> Value
                {
                    uint32_t bits = v.scalar.u32;
                    if (bits == 0)
                        return Value::make_i32(-1);
                    int pos = 0;
                    while ((bits & 1) == 0) {
                        bits >>= 1;
                        pos++;
                    }
                    return Value::make_i32(pos);
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (auto &e : x.elements) out.push_back(lsb(e));
                    return Value::make_composite(std::move(out));
                }
                return lsb(x);
            }
        case FindUMsb:
            {
                const Value &x = get(0);
                auto msb = [](const Value &v) -> Value
                {
                    uint32_t bits = v.scalar.u32;
                    if (bits == 0)
                        return Value::make_i32(-1);
                    int pos = 31;
                    while ((bits & 0x80000000u) == 0) {
                        bits <<= 1;
                        pos--;
                    }
                    return Value::make_i32(pos);
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (auto &e : x.elements) out.push_back(msb(e));
                    return Value::make_composite(std::move(out));
                }
                return msb(x);
            }
        case FindSMsb:
            {
                const Value &x = get(0);
                auto msb = [](const Value &v) -> Value
                {
                    int32_t bits = v.scalar.i32;
                    if (bits == 0 || bits == -1)
                        return Value::make_i32(-1);
                    uint32_t ubits = bits < 0 ? ~static_cast<uint32_t>(bits)
                                              : static_cast<uint32_t>(bits);
                    int pos = 30;
                    while ((ubits & 0x40000000u) == 0 && pos > 0) {
                        ubits <<= 1;
                        pos--;
                    }
                    return Value::make_i32(pos);
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> out;
                    for (auto &e : x.elements) out.push_back(msb(e));
                    return Value::make_composite(std::move(out));
                }
                return msb(x);
            }

        // --- FrexpStruct ---
        // (Non-struct Frexp writes through a pointer; handled in exec_ext_inst.)
        case FrexpStruct:
            {
                const Value &x = get(0);
                auto do_frexp = [](const Value &v) -> std::pair<Value, Value>
                {
                    int exp;
                    if (v.kind == Value::Kind::Float32) {
                        float sig = std::frexp(v.scalar.f32, &exp);
                        return {Value::make_f32(sig), Value::make_i32(exp)};
                    }
                    double sig = std::frexp(v.scalar.f64, &exp);
                    return {Value::make_f64(sig), Value::make_i32(exp)};
                };
                if (x.kind == Value::Kind::Composite) {
                    std::vector<Value> sigs, exps;
                    for (auto &e : x.elements) {
                        auto [s, ex] = do_frexp(e);
                        sigs.push_back(s);
                        exps.push_back(ex);
                    }
                    return Value::make_composite({Value::make_composite(std::move(sigs)),
                                                  Value::make_composite(std::move(exps))});
                }
                auto [sig, exp] = do_frexp(x);
                return Value::make_composite({sig, exp});
            }

        // --- Pack ---
        case PackSnorm4x8:
            {
                const Value &v = get(0);
                uint32_t result = 0;
                for (int i = 0; i < 4; ++i) {
                    float clamped = std::fmaxf(-1.f, std::fminf(1.f, v.elements[i].scalar.f32));
                    auto byte = static_cast<uint8_t>(
                        static_cast<int8_t>(std::round(clamped * 127.f)));
                    result |= static_cast<uint32_t>(byte) << (i * 8);
                }
                return Value::make_u32(result);
            }
        case PackUnorm4x8:
            {
                const Value &v = get(0);
                uint32_t result = 0;
                for (int i = 0; i < 4; ++i) {
                    float clamped = std::fmaxf(0.f, std::fminf(1.f, v.elements[i].scalar.f32));
                    result |=
                        static_cast<uint32_t>(static_cast<uint8_t>(std::round(clamped * 255.f)))
                        << (i * 8);
                }
                return Value::make_u32(result);
            }
        case PackSnorm2x16:
            {
                const Value &v = get(0);
                auto pack = [](float f) -> uint16_t
                {
                    float c = std::fmaxf(-1.f, std::fminf(1.f, f));
                    return static_cast<uint16_t>(static_cast<int16_t>(std::round(c * 32767.f)));
                };
                uint32_t lo = pack(v.elements[0].scalar.f32);
                uint32_t hi = pack(v.elements[1].scalar.f32);
                return Value::make_u32(lo | (hi << 16));
            }
        case PackUnorm2x16:
            {
                const Value &v = get(0);
                auto pack = [](float f) -> uint32_t
                {
                    float c = std::fmaxf(0.f, std::fminf(1.f, f));
                    return static_cast<uint32_t>(static_cast<uint16_t>(std::round(c * 65535.f)));
                };
                return Value::make_u32(pack(v.elements[0].scalar.f32)
                                       | (pack(v.elements[1].scalar.f32) << 16));
            }
        case PackHalf2x16:
            {
                const Value &v = get(0);
                uint32_t lo = f32_to_f16(v.elements[0].scalar.f32);
                uint32_t hi = f32_to_f16(v.elements[1].scalar.f32);
                return Value::make_u32(lo | (hi << 16));
            }
        case PackDouble2x32:
            {
                const Value &v = get(0);
                uint64_t bits = static_cast<uint64_t>(v.elements[0].scalar.u32)
                              | (static_cast<uint64_t>(v.elements[1].scalar.u32) << 32);
                double d;
                std::memcpy(&d, &bits, 8);
                return Value::make_f64(d);
            }

        // --- Unpack ---
        case UnpackSnorm2x16:
            {
                uint32_t p = get(0).scalar.u32;
                auto unpack = [](uint16_t bits) -> float
                {
                    int16_t s;
                    std::memcpy(&s, &bits, 2);
                    return std::fmaxf(-1.f, static_cast<float>(s) / 32767.f);
                };
                return Value::make_composite(
                    {Value::make_f32(unpack(p & 0xFFFFu)), Value::make_f32(unpack(p >> 16))});
            }
        case UnpackUnorm2x16:
            {
                uint32_t p = get(0).scalar.u32;
                return Value::make_composite(
                    {Value::make_f32(static_cast<float>(p & 0xFFFFu) / 65535.f),
                     Value::make_f32(static_cast<float>(p >> 16) / 65535.f)});
            }
        case UnpackHalf2x16:
            {
                uint32_t p = get(0).scalar.u32;
                return Value::make_composite({Value::make_f32(f16_to_f32(p & 0xFFFFu)),
                                              Value::make_f32(f16_to_f32(p >> 16))});
            }
        case UnpackSnorm4x8:
            {
                uint32_t p = get(0).scalar.u32;
                auto unpack = [](uint8_t bits) -> float
                {
                    int8_t s = static_cast<int8_t>(bits);
                    return std::fmaxf(-1.f, static_cast<float>(s) / 127.f);
                };
                return Value::make_composite({Value::make_f32(unpack(p & 0xFFu)),
                                              Value::make_f32(unpack((p >> 8) & 0xFFu)),
                                              Value::make_f32(unpack((p >> 16) & 0xFFu)),
                                              Value::make_f32(unpack(p >> 24))});
            }
        case UnpackUnorm4x8:
            {
                uint32_t p = get(0).scalar.u32;
                return Value::make_composite(
                    {Value::make_f32(static_cast<float>(p & 0xFFu) / 255.f),
                     Value::make_f32(static_cast<float>((p >> 8) & 0xFFu) / 255.f),
                     Value::make_f32(static_cast<float>((p >> 16) & 0xFFu) / 255.f),
                     Value::make_f32(static_cast<float>(p >> 24) / 255.f)});
            }
        case UnpackDouble2x32:
            {
                uint64_t bits;
                std::memcpy(&bits, &get(0).scalar.f64, 8);
                return Value::make_composite(
                    {Value::make_u32(static_cast<uint32_t>(bits & 0xFFFFFFFFu)),
                     Value::make_u32(static_cast<uint32_t>(bits >> 32))});
            }

        // --- Determinant / MatrixInverse: stub ---
        case Determinant:
        case MatrixInverse:
            diagnostics.push_back(
                "GLSL.std.450: Determinant/MatrixInverse not yet implemented, returning 0");
            return Value::make_f32(0.f);

        // --- Interpolate: stub ---
        case InterpolateAtCentroid:
        case InterpolateAtSample:
        case InterpolateAtOffset:
            diagnostics.push_back("GLSL.std.450: Interpolate not applicable in Compute shaders");
            return args.empty() ? Value::make_f32(0.f) : args[0];

        default:
            diagnostics.push_back("GLSL.std.450: unknown instruction " + std::to_string(inst_id));
            return Value::make_u32(0);
    }
}

} // namespace spvdb
