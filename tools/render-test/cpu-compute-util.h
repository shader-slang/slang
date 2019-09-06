#ifndef CPU_COMPUTE_UTIL_H
#define CPU_COMPUTE_UTIL_H

#include "cpu-memory-binding.h"
#include "slang-support.h"
#include "options.h"

#include "../../source/core/slang-smart-pointer.h"

namespace renderer_test {

struct CPUComputeUtil
{
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

    static SlangResult calcBindings(const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& outContext);

    static SlangResult execute(const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& outContext);

    static SlangResult writeBindings(const ShaderInputLayout& layout, const List<CPUMemoryBinding::Buffer>& buffers, const Slang::String& fileName);
};


} // renderer_test

#endif //CPU_MEMORY_BINDING_H
