#include <vector>
#include <string>
#include <slang.h>
#include <slang-com-ptr.h>
#include "slang-wasm.h"
#include "../core/slang-blob.h"
#include "../core/slang-exception.h"

using namespace slang;

namespace slang
{
namespace wgsl
{

Error g_error;

Error getLastError()
{
    Error currentError = g_error;
    g_error = {};
    return currentError;
}

GlobalSession* createGlobalSession()
{
    IGlobalSession* globalSession = nullptr;
    {
        SlangResult result = slang::createGlobalSession(&globalSession);
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return nullptr;
        }
    }

    return new GlobalSession(globalSession);
}

Session* GlobalSession::createSession()
{
    ISession* session = nullptr;
    {
        SessionDesc sessionDesc = {};
        sessionDesc.structureSize = sizeof(sessionDesc);
        constexpr SlangInt targetCount = 1;
        TargetDesc targets[targetCount] = {
            {.structureSize = sizeof(TargetDesc), .format = SLANG_WGSL}
        };
        sessionDesc.targets = targets;
        sessionDesc.targetCount = targetCount;
        SlangResult result = m_interface->createSession(sessionDesc, &session);
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return nullptr;
        }
    }

    return new Session(session);
}

Module* Session::loadModuleFromSource(const std::string& slangCode)
{
    Slang::ComPtr<IModule> module;
    {
        const char * name = "";
        const char * path = "";
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        Slang::ComPtr<ISlangBlob> slangCodeBlob = Slang::RawBlob::create(
            slangCode.c_str(), slangCode.size());
        module = m_interface->loadModuleFromSource(
            name, path, slangCodeBlob, diagnosticsBlob.writeRef());
        if (!module)
        {
            g_error.type = std::string("USER");
            g_error.message = std::string(
                (char*)diagnosticsBlob->getBufferPointer(),
                (char*)diagnosticsBlob->getBufferPointer() +
                diagnosticsBlob->getBufferSize());
            return nullptr;
        }
    }

    return new Module(module);
}

EntryPoint* Module::findEntryPointByName(const std::string& name)
{
    Slang::ComPtr<IEntryPoint> entryPoint;
    {
        SlangResult result = moduleInterface()->findEntryPointByName(
            name.c_str(), entryPoint.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return nullptr;
        }
    }

    return new EntryPoint(entryPoint);
}

ComponentType* Session::createCompositeComponentType(
    const std::vector<ComponentType*>& components)
{
    Slang::ComPtr<IComponentType> composite;
    {
        std::vector<IComponentType*> nativeComponents(components.size());
        for (size_t i = 0U; i < components.size(); i++)
            nativeComponents[i] = components[i]->interface();
        SlangResult result = m_interface->createCompositeComponentType(
            nativeComponents.data(),
            (SlangInt)nativeComponents.size(),
            composite.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            return nullptr;
        }
    }

    return new ComponentType(composite);
}

ComponentType* ComponentType::link()
{
    Slang::ComPtr<IComponentType> linkedProgram;
    {
        Slang::ComPtr<ISlangBlob> diagnosticBlob;
        SlangResult result = interface()->link(
            linkedProgram.writeRef(), diagnosticBlob.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            g_error.message = std::string(
                (char*)diagnosticBlob->getBufferPointer(),
                (char*)diagnosticBlob->getBufferPointer() +
                diagnosticBlob->getBufferSize());
            return nullptr;
        }
    }

    return new ComponentType(linkedProgram);
}

std::string ComponentType::getEntryPointCode(int entryPointIndex, int targetIndex)
{
    {
        Slang::ComPtr<IBlob> kernelBlob;
        Slang::ComPtr<ISlangBlob> diagnosticBlob;
        SlangResult result = interface()->getEntryPointCode(
            entryPointIndex,
            targetIndex,
            kernelBlob.writeRef(),
            diagnosticBlob.writeRef());
        if (result != SLANG_OK)
        {
            g_error.type = std::string("USER");
            g_error.result = result;
            g_error.message = std::string(
                (char*)diagnosticBlob->getBufferPointer(),
                (char*)diagnosticBlob->getBufferPointer() +
                diagnosticBlob->getBufferSize());
            return "";
        }
        std::string wgslCode = std::string(
            (char*)kernelBlob->getBufferPointer(),
            (char*)kernelBlob->getBufferPointer() + kernelBlob->getBufferSize());
        return wgslCode;
    }

    return {};
}

} // namespace wgsl
} // namespace slang
