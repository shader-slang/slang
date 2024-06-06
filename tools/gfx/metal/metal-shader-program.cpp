// metal-shader-program.cpp
#include "metal-shader-program.h"
#include "metal-device.h"
#include "metal-util.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

ShaderProgramImpl::ShaderProgramImpl(DeviceImpl* device)
    : m_device(device)
{
}

ShaderProgramImpl::~ShaderProgramImpl()
{
}

Result ShaderProgramImpl::createShaderModule(slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode)
{
    m_codeBlobs.add(kernelCode);
    const char* realEntryPointName = entryPointInfo->getNameOverride();
    NS::SharedPtr<NS::String> source = MetalUtil::createStringView((void*)kernelCode->getBufferPointer(), kernelCode->getBufferSize());;
    NS::Error* error;
    NS::SharedPtr<MTL::Library> library = NS::TransferPtr(m_device->m_device->newLibrary(source.get(), nullptr, &error));
    if (!library)
    {
        std::cout << error->localizedDescription()->utf8String() << std::endl;
        return SLANG_E_INVALID_ARG;
    }
    m_entryPointNames.add(realEntryPointName);
    m_modules.add(library);
    return SLANG_OK;
}

} // namespace metal
} // namespace gfx
