//TEST:COMPARE_COMPUTE(targets=vk+glsl+cuda+cpu, filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Every test of these commands defines the macro SLANG_TEST_<TARGET> of its target. The line above
// runs for the targets vk, glsl, cuda and cpu, and each run defines the macro of its own target.
#if !defined(SLANG_TEST_VK) && !defined(SLANG_TEST_GLSL) && !defined(SLANG_TEST_CPU) && \
    !defined(SLANG_TEST_CUDA)
#error "The test does not define the macro of its target."
#endif

// Returns 1 if the macro SLANG_TEST_<TARGET> of the test fits the target of the compiler.
int macroFitsTarget()
{
    int result = 1;
#if defined(SLANG_TEST_VK)
    __target_switch
    {
    case spirv:
        break;
    default:
        result = 0;
        break;
    }
#elif defined(SLANG_TEST_GLSL)
    // The target glsl emits GLSL, and glslang then compiles it to SPIR-V.
    __target_switch
    {
    case glsl:
        break;
    default:
        result = 0;
        break;
    }
#elif defined(SLANG_TEST_CPU)
    __target_switch
    {
    case cpp:
        break;
    default:
        result = 0;
        break;
    }
#elif defined(SLANG_TEST_CUDA)
    __target_switch
    {
    case cuda:
        break;
    default:
        result = 0;
        break;
    }
#endif
    return result;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = int(dispatchThreadID.x) * 3 + macroFitsTarget() - 1;
}

// CHECK:      0
// CHECK-NEXT: 3
// CHECK-NEXT: 6
// CHECK-NEXT: 9
