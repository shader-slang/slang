#ifndef CPU_COMPUTE_UTIL_H
#define CPU_COMPUTE_UTIL_H

#include "cpu-memory-binding.h"
#include "slang-support.h"
#include "options.h"

#include "../../source/core/slang-smart-pointer.h"

namespace renderer_test {

struct CPUComputeUtil
{
    enum class ExecuteStyle
    {
        Unknown,
        Thread,
        Group,
        GroupRange,
    };

    struct Resource : public RefObject
    {
        void* getInterface() const { return m_interface; }
        void* m_interface;
    };

    struct Context
    {
            /// Holds the binding information
        CPUMemoryBinding binding;
            /// Buffers are held in same order as entries in layout (useful for dumping out bindings)
        List<CPUMemoryBinding::Buffer> buffers;
            /// Holds the resources created (in calcBindings)
        List<RefPtr<Resource> > m_resources;
    };

    struct ExecuteInfo
    {
        typedef void (*Func)();

        ExecuteStyle m_style;
        Func m_func;
        uint32_t m_dispatchSize[3];
        uint32_t m_numThreadsPerAxis[3];

        void* m_uniformState;
        void* m_uniformEntryPointParams;
    };

    static SlangResult calcBindings(const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& outContext);

    static SlangResult calcExecuteInfo(ExecuteStyle style, const uint32_t dispatchSize[3], const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& context, ExecuteInfo& out);

    static SlangResult execute(const ExecuteInfo& info);

    static SlangResult writeBindings(const ShaderInputLayout& layout, const List<CPUMemoryBinding::Buffer>& buffers, const Slang::String& fileName);
};


} // renderer_test

#endif //CPU_MEMORY_BINDING_H
