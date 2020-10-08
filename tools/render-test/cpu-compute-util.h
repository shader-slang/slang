#ifndef CPU_COMPUTE_UTIL_H
#define CPU_COMPUTE_UTIL_H

#include "slang-support.h"
#include "options.h"

#include "bind-location.h"

#include "../../source/core/slang-basic.h"

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
        BindSet m_bindSet;
        CPULikeBindRoot m_bindRoot;

            /// Buffers are held in same order as entries in layout (useful for dumping out bindings)
        List<BindSet::Value*> m_buffers;

            /// Bindless resource objects
        Slang::OrderedDictionary<Slang::String, RefPtr<Resource>> m_bindlessResources;
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

        /// True if this feature is available on CPU
    static bool hasFeature(const Slang::UnownedStringSlice& feature);

        /// Runs code across run styles and makes sure output buffers match
    static SlangResult checkStyleConsistency(ISlangSharedLibrary* sharedLib, const uint32_t dispatchSize[3], const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout);

    static SlangResult createBindlessResources(ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& context);

        /// Query and fill in the RTTI pointer and runtime resource handle values in data buffers.
    static SlangResult fillRuntimeHandleInBuffers(
        ShaderCompilerUtil::OutputAndLayout& compilationAndLayout,
        Context& context,
        ISlangSharedLibrary* sharedLib);

    static SlangResult calcBindings(const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& outContext);
    
    static SlangResult calcExecuteInfo(ExecuteStyle style, ISlangSharedLibrary* sharedLib, const uint32_t dispatchSize[3], const ShaderCompilerUtil::OutputAndLayout& compilationAndLayout, Context& context, ExecuteInfo& out);

    static SlangResult execute(const ExecuteInfo& info);
};


} // renderer_test

#endif //CPU_COMPUTE_UTIL_H
