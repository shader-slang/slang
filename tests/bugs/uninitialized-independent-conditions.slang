//TEST:SIMPLE: -target spirv -no-codegen -warnings-as-errors all

struct Data { float value; };

// Check that independent flags can each guard a variable's initialization and use.
void independentFlags(bool a, bool b, bool c, bool d, float* output)
{
    Data da, db, dc, dd;
    if (a) da.value = 1;
    if (b) db.value = 1;
    if (c) dc.value = 1;
    if (d) dd.value = 1;
    if (a) output[0] = da.value;
    if (b) output[1] = db.value;
    if (c) output[2] = dc.value;
    if (d) output[3] = dd.value;
}

// Check that eight independent variable/guard pairs produce no warnings.
void eightIndependentFlags(bool a, bool b, bool c, bool d, bool e, bool f, bool g, bool h, float* output)
{
    Data da, db, dc, dd, de, df, dg, dh;
    if (a) da.value = 1;
    if (b) db.value = 1;
    if (c) dc.value = 1;
    if (d) dd.value = 1;
    if (e) de.value = 1;
    if (f) df.value = 1;
    if (g) dg.value = 1;
    if (h) dh.value = 1;
    if (a) output[0] = da.value;
    if (b) output[1] = db.value;
    if (c) output[2] = dc.value;
    if (d) output[3] = dd.value;
    if (e) output[4] = de.value;
    if (f) output[5] = df.value;
    if (g) output[6] = dg.value;
    if (h) output[7] = dh.value;
}

// Check that unrelated repeated flags do not cause a warning for an initialized read.
void usefulLateCondition(uint flags, bool initialized, float* output)
{
    Data data;
    if (initialized) data.value = 1;
#define DECLARE_FLAG(N) bool flag##N = (flags & (1u << N)) != 0;
#define USE_FLAG(N) if (flag##N) output[N] += 1;
    DECLARE_FLAG(0) DECLARE_FLAG(1) DECLARE_FLAG(2) DECLARE_FLAG(3)
    DECLARE_FLAG(4) DECLARE_FLAG(5) DECLARE_FLAG(6) DECLARE_FLAG(7)
#undef DECLARE_FLAG
    USE_FLAG(0) USE_FLAG(1) USE_FLAG(2) USE_FLAG(3)
    USE_FLAG(4) USE_FLAG(5) USE_FLAG(6) USE_FLAG(7)
    USE_FLAG(0) USE_FLAG(1) USE_FLAG(2) USE_FLAG(3)
    USE_FLAG(4) USE_FLAG(5) USE_FLAG(6) USE_FLAG(7)
#undef USE_FLAG
    if (initialized) output[8] = data.value;
}

// Check that unrelated repeated flags do not starve an early-exit guard proof.
float usefulLateEarlyExit(uint flags, bool initialized, float* output)
{
    Data data;
    if (initialized) data.value = 1;
#define DECLARE_FLAG(N) bool flag##N = (flags & (1u << N)) != 0;
#define USE_FLAG(N) if (flag##N) output[N] += 1;
    DECLARE_FLAG(0) DECLARE_FLAG(1) DECLARE_FLAG(2) DECLARE_FLAG(3)
    DECLARE_FLAG(4) DECLARE_FLAG(5) DECLARE_FLAG(6) DECLARE_FLAG(7)
#undef DECLARE_FLAG
    USE_FLAG(0) USE_FLAG(1) USE_FLAG(2) USE_FLAG(3)
    USE_FLAG(4) USE_FLAG(5) USE_FLAG(6) USE_FLAG(7)
    USE_FLAG(0) USE_FLAG(1) USE_FLAG(2) USE_FLAG(3)
    USE_FLAG(4) USE_FLAG(5) USE_FLAG(6) USE_FLAG(7)
#undef USE_FLAG
    if (initialized) {} else return 0;
    return data.value;
}

[shader("compute")]
void main() {}
