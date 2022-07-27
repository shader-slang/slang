// debug-helper-functions.h
#pragma once
#include "debug-base.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

#ifdef __FUNCSIG__
#    define SLANG_FUNC_SIG __FUNCSIG__
#elif defined(__PRETTY_FUNCTION__)
#    define SLANG_FUNC_SIG __FUNCSIG__
#elif defined(__FUNCTION__)
#    define SLANG_FUNC_SIG __FUNCTION__
#else
#    define SLANG_FUNC_SIG "UnknownFunction"
#endif

thread_local const char* _currentFunctionName = nullptr;
struct SetCurrentFuncRAII
{
    SetCurrentFuncRAII(const char* funcName) { _currentFunctionName = funcName; }
    ~SetCurrentFuncRAII() { _currentFunctionName = nullptr; }
};
#define SLANG_GFX_API_FUNC SetCurrentFuncRAII setFuncNameRAII(SLANG_FUNC_SIG)
#define SLANG_GFX_API_FUNC_NAME(x) SetCurrentFuncRAII setFuncNameRAII(x)

/// Returns the public API function name from a `SLANG_FUNC_SIG` string.
String _gfxGetFuncName(const char* input);

template <typename... TArgs>
static char* _gfxDiagnoseFormat(
    char* buffer, // Initial buffer to output formatted string.
    size_t shortBufferSize, // Size of the initial buffer.
    List<char>& bufferArray, // A list for allocating a large buffer if needed.
    const char* format, // The format string.
    TArgs... args);

template <typename... TArgs>
static void _gfxDiagnoseImpl(DebugMessageType type, const char* format, TArgs... args);

#define GFX_DIAGNOSE_ERROR(message)                                                                \
    _gfxDiagnoseImpl(                                                                              \
        DebugMessageType::Error,                                                                   \
        "%s: %s",                                                                                  \
        _gfxGetFuncName(_currentFunctionName ? _currentFunctionName : SLANG_FUNC_SIG).getBuffer(), \
        message)
#define GFX_DIAGNOSE_WARNING(message)                                                              \
    _gfxDiagnoseImpl(                                                                              \
        DebugMessageType::Warning,                                                                 \
        "%s: %s",                                                                                  \
        _gfxGetFuncName(_currentFunctionName ? _currentFunctionName : SLANG_FUNC_SIG).getBuffer(), \
        message)
#define GFX_DIAGNOSE_INFO(message)                                                                 \
    _gfxDiagnoseImpl(                                                                              \
        DebugMessageType::Info,                                                                    \
        "%s: %s",                                                                                  \
        _gfxGetFuncName(_currentFunctionName ? _currentFunctionName : SLANG_FUNC_SIG).getBuffer(), \
        message)
#define GFX_DIAGNOSE_FORMAT(type, format, ...)                                            \
    {                                                                                     \
        char shortBuffer[256];                                                            \
        List<char> bufferArray;                                                           \
        auto message = _gfxDiagnoseFormat(                                                \
            shortBuffer, sizeof(shortBuffer), bufferArray, format, __VA_ARGS__);          \
        _gfxDiagnoseImpl(                                                                 \
            type,                                                                         \
            "%s: %s",                                                                     \
            _gfxGetFuncName(_currentFunctionName ? _currentFunctionName : SLANG_FUNC_SIG) \
                .getBuffer(),                                                             \
            message);                                                                     \
    }
#define GFX_DIAGNOSE_ERROR_FORMAT(...) GFX_DIAGNOSE_FORMAT(DebugMessageType::Error, __VA_ARGS__)

#define SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(typeName)                                    \
    I##typeName* Debug##typeName::getInterface(const Slang::Guid& guid)                 \
    {                                                                                   \
        return (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_I##typeName) \
                    ? static_cast<I##typeName*>(this)                                   \
                    : nullptr;                                                          \
    }
#define SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(typeName, parentType)                   \
    I##typeName* Debug##typeName::getInterface(const Slang::Guid& guid)                   \
    {                                                                                     \
        return (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_I##typeName || \
                guid == GfxGUID::IID_I##parentType)                                       \
                    ? static_cast<I##typeName*>(this)                                     \
                    : nullptr;                                                            \
    }

SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(Device)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(BufferResource, Resource)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(TextureResource, Resource)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(CommandBuffer)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(CommandQueue)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(Framebuffer)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(FramebufferLayout)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(InputLayout)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(RenderPassLayout)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(PipelineState)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(ResourceView)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(SamplerState)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(ShaderObject)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(ShaderProgram)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(Swapchain)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(TransientResourceHeap)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(QueryPool)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT(AccelerationStructure, ResourceView)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(Fence)
SLANG_GFX_DEBUG_GET_INTERFACE_IMPL(ShaderTable)

#undef SLANG_GFX_DEBUG_GET_INTERFACE_IMPL
#undef SLANG_GFX_DEBUG_GET_INTERFACE_IMPL_PARENT

// Utility conversion functions to get Debug* object or the inner object from a user provided
// pointer.
#define SLANG_GFX_DEBUG_GET_OBJ_IMPL(type)                                                   \
    static Debug##type* getDebugObj(I##type* ptr) { return static_cast<Debug##type*>(ptr); } \
    static I##type* getInnerObj(I##type* ptr)                                                \
    {                                                                                        \
        if (!ptr) return nullptr;                                                            \
        auto debugObj = getDebugObj(ptr);                                                    \
        return debugObj->baseObject;                                                         \
    }

SLANG_GFX_DEBUG_GET_OBJ_IMPL(Device)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(BufferResource)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(TextureResource)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(CommandBuffer)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(CommandQueue)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ComputeCommandEncoder)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(RenderCommandEncoder)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ResourceCommandEncoder)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(RayTracingCommandEncoder)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(Framebuffer)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(FramebufferLayout)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(InputLayout)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(RenderPassLayout)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(PipelineState)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ResourceView)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(SamplerState)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ShaderObject)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ShaderProgram)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(Swapchain)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(TransientResourceHeap)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(QueryPool)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(AccelerationStructure)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(Fence)
SLANG_GFX_DEBUG_GET_OBJ_IMPL(ShaderTable)

#undef SLANG_GFX_DEBUG_GET_OBJ_IMPL

void validateAccelerationStructureBuildInputs(
    const IAccelerationStructure::BuildInputs& buildInputs);

} // namespace debug
} // namespace gfx
