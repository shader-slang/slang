//why macros and not templated generics? -- macros are faster to iterate (development) on

//#define test_if_double_does_not_crash

#ifdef test_if_double_does_not_crash
#define MAYBE_DOUBLE double
#else
#define MAYBE_DOUBLE float
#endif

#define testFunctionIn1Param(func, val, testv1)         \
            func(float(testv1)) == float(val) &&        \
            func(vec2(testv1)) == vec2(val) &&          \
            func(ivec3(testv1)) == ivec3(val) &&        \
            func(uvec4(testv1)) == uvec4(val) &&        \
            func(uint(testv1)) == uint(val) &&          \
            func(int(testv1)) == int(val) &&            \
            func(MAYBE_DOUBLE(testv1)) == MAYBE_DOUBLE(val)

            //func(half(testv1)) == half(val) &&        \
            //func(uint16_t(testv1)) == uint16_t(val);

#define testFunctionIn1ParamBool(func, val, testv1)     \
            func(float(testv1)) == bool(val) &&         \
            func(vec2(testv1)) == bool(val) &&          \
            func(ivec3(testv1)) == bool(val) &&         \
            func(uvec4(testv1)) == bool(val) &&         \
            func(uint(testv1)) == bool(val) &&          \
            func(int(testv1)) == bool(val)              \

            //func(half(testv1)) == half(val) &&        \
            //func(uint16_t(testv1)) == uint16_t(val);

#define testFunctionIn2ParamPlus(func, val, testv1, ...)             \
            func(float(testv1), __VA_ARGS__) == float(val) &&        \
            func(vec2(testv1), __VA_ARGS__) == vec2(val) &&          \
            func(ivec3(testv1), __VA_ARGS__) == ivec3(val) &&        \
            func(uvec4(testv1), __VA_ARGS__) == uvec4(val) &&        \
            func(uint(testv1), __VA_ARGS__) == uint(val) &&          \
            func(int(testv1), __VA_ARGS__) == int(val) &&            \
            func(MAYBE_DOUBLE(testv1), __VA_ARGS__) == MAYBE_DOUBLE(val)

            //func(half(testv1), testv2) == half(val) &&        \
            //func(uint16_t(testv1), testv2) == uint16_t(val);

#define testFunctionInInt1Param(func, val, testv) \
            func(uint(testv)) == uint(val) &&     \
            func(uvec2(testv)) == uvec2(val) &&   \
            func(ivec3(testv)) == ivec3(val) &&   \
            func(int(testv)) == int(val)

#define testFunctionInInt2ParamPlus(func, val, testv1, ...) \
        func(uint(testv1), __VA_ARGS__) == uint(val) &&         \
        func(uvec2(testv1), __VA_ARGS__) == uvec2(val) &&       \
        func(ivec3(testv1), __VA_ARGS__) == ivec3(val) &&       \
        func(int(testv1), __VA_ARGS__) == int(val) 