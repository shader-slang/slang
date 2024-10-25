#pragma once

#include <slang.h>
#include <unordered_map>
#include <emscripten/val.h>

namespace slang
{
namespace wgsl
{

class Error
{
public:
    // Can be
    // "USER": User did not call the API correctly
    // "INTERNAL": Slang failed due to a bug
    std::string type;
    std::string message;
    SlangResult result;
};

Error getLastError();

class CompileTargets
{
public:
    CompileTargets();
    int findCompileTarget(const std::string& name);
private:
    std::unordered_map<std::string, SlangCompileTarget> m_compileTargetMap;
};

CompileTargets* getCompileTargets();

class ComponentType
{
public:

    ComponentType(slang::IComponentType* interface) :
        m_interface(interface) {}

    ComponentType* link();

    std::string getEntryPointCode(int entryPointIndex, int targetIndex);
    emscripten::val getEntryPointCodeSpirv(int entryPointIndex, int targetIndex);

    slang::IComponentType* interface() const {return m_interface;}

    virtual ~ComponentType() = default;

private:

    Slang::ComPtr<slang::IComponentType> m_interface;
};

class EntryPoint : public ComponentType
{
public:

    EntryPoint(slang::IEntryPoint* interface) : ComponentType(interface) {}

private:

    slang::IEntryPoint* entryPointInterface() const {
        return static_cast<slang::IEntryPoint*>(interface());
    }
};

class Module : public ComponentType
{
public:

    Module(slang::IModule* interface) : ComponentType(interface) {}

    EntryPoint* findEntryPointByName(const std::string& name);
    EntryPoint* findAndCheckEntryPoint(const std::string& name, int stage);

    slang::IModule* moduleInterface() const {
        return static_cast<slang::IModule*>(interface());
    }
};

class Session
{
public:

    Session(slang::ISession* interface)
        : m_interface(interface) {}

    Module* loadModuleFromSource(const std::string& slangCode);

    ComponentType* createCompositeComponentType(
        const std::vector<ComponentType*>& components);

    slang::ISession* interface() const {return m_interface;}

private:

    Slang::ComPtr<slang::ISession> m_interface;
};

class GlobalSession
{
public:

    GlobalSession(slang::IGlobalSession* interface)
        : m_interface(interface) {}

    Session* createSession(int compileTarget);

    slang::IGlobalSession* interface() const {return m_interface;}

private:

    Slang::ComPtr<slang::IGlobalSession> m_interface;
};

GlobalSession* createGlobalSession();

} // namespace wgsl
} // namespace slang
