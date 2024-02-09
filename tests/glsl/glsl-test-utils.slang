

#define testFunctionIn1Param(func, testv1, val)         \
            func(float(testv1)) == float(val) &&        \
            func(vec2(testv1)) == vec2(val) &&          \
            func(ivec3(testv1)) == ivec3(val) &&        \
            func(uvec4(testv1)) == uvec4(val) &&        \
            func(uint(testv1)) == uint(val) &&          \
            func(int(testv1)) == int(val)               \

            //func(half(testv1)) == half(val) &&        \
            //func(uint16_t(testv1)) == uint16_t(val);  

#define testFunctionIn1ParamBool(func, testv1, val)     \
            func(float(testv1)) == bool(val) &&         \
            func(vec2(testv1)) == bool(val) &&          \
            func(ivec3(testv1)) == bool(val) &&         \
            func(uvec4(testv1)) == bool(val) &&         \
            func(uint(testv1)) == bool(val) &&          \
            func(int(testv1)) == bool(val)              \

            //func(half(testv1)) == half(val) &&        \
            //func(uint16_t(testv1)) == uint16_t(val);  

#define testFunctionIn2Param(func, testv1, testv2, val)         \
            func(float(testv1), testv2) == float(val) &&        \
            func(vec2(testv1), testv2) == vec2(val) &&          \
            func(ivec3(testv1), testv2) == ivec3(val) &&        \
            func(uvec4(testv1), testv2) == uvec4(val) &&        \
            func(uint(testv1), testv2) == uint(val) &&          \
            func(int(testv1), testv2) == int(val)               \

            //func(half(testv1), testv2) == half(val) &&        \
            //func(uint16_t(testv1), testv2) == uint16_t(val);

#define testFunctionInArithmetic1Param(func, testv1, val) testFunctionIn1Param(func, testv1, val);

#define testFunctionInArithmetic2Param(func, testv1, testv2, val) testFunctionIn2Param(func, testv1, testv2, val);

#define testFunctionInInt1Param(func, testv, val) \
            func(uint(testv)) == uint(val) &&     \
            func(uvec2(testv)) == uvec2(val) &&   \
            func(ivec3(testv)) == ivec3(val) &&   \
            func(int(testv)) == int(val) 

#define testFunctionInInt2Param(func, testv1, testv2, val) \
        func(uint(testv1), testv2) == uint(val) &&         \
        func(uvec2(testv1), testv2) == uvec2(val) &&       \
        func(ivec3(testv1), testv2) == ivec3(val) &&       \
        func(int(testv1), testv2) == int(val) 