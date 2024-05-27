// metal-shader-program.cpp
#include "metal-shader-program.h"

#include "metal-device.h"

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

void ShaderProgramImpl::comFree() { }

Result ShaderProgramImpl::createShaderModule(
    slang::EntryPointReflection* entryPointInfo, ComPtr<ISlangBlob> kernelCode)
{
    if (entryPointInfo == nullptr || kernelCode == nullptr || kernelCode->getBufferSize() == 0)
    {
        return SLANG_E_INVALID_ARG;
    }

    auto realEntryPointName = entryPointInfo->getNameOverride();
    std::string sourceStr(static_cast<const char*>(kernelCode->getBufferPointer()), kernelCode->getBufferSize());
    NS::String *nsSourceString = NS::String::alloc()->init(sourceStr.c_str(), NS::UTF8StringEncoding);
    NS::Error* error;
    MTL::Library* library = m_device->m_device->newLibrary(nsSourceString, nullptr, &error);
    if (library == nullptr)
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
