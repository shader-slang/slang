// slang-downstream-dep1.cpp
#include "slang-downstream-dep1.h"


namespace Slang
{

// A temporary class that adapts `ISlangSharedLibrary_Dep1` to ISlangSharedLibrary
class SharedLibraryDep1Adapter : public ComBaseObject, public ISlangSharedLibrary
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE;

    // ISlangSharedLibrary
    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name) SLANG_OVERRIDE { return m_contained->findSymbolAddressByName(name); }

    SharedLibraryDep1Adapter(ISlangSharedLibrary_Dep1* dep1) :
        m_contained(dep1)
    {
    }

protected:
    void* getInterface(const Guid& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid() ||
            guid == ICastable::getTypeGuid() ||
            guid == ISlangSharedLibrary::getTypeGuid())
        {
            return static_cast<ISlangSharedLibrary*>(this);
        }
        return nullptr;
    }
    void* getObject(const Guid& guid)
    {
        SLANG_UNUSED(guid);
        return nullptr;
    }

    ComPtr<ISlangSharedLibrary_Dep1> m_contained;
};

void* SharedLibraryDep1Adapter::castAs(const SlangUUID& guid)
{
	if (auto intf = getInterface(guid))
	{
		return intf;
	}
	return getObject(guid);
}

/* Hack to take into account downstream compilers shared library interface might need an adapter */
/* static */SlangResult DownstreamUtil_Dep1::getDownstreamSharedLibrary(DownstreamCompileResult* downstreamResult, ComPtr<ISlangSharedLibrary>& outSharedLibrary)
{
	ComPtr<ISlangSharedLibrary> lib;
	SLANG_RETURN_ON_FAIL(downstreamResult->getHostCallableSharedLibrary(lib));

	if (SLANG_SUCCEEDED(lib->queryInterface(ISlangSharedLibrary::getTypeGuid(), (void**)outSharedLibrary.writeRef())))
	{
		return SLANG_OK;
	}

	ComPtr<ISlangSharedLibrary_Dep1> libDep1;
	if (SLANG_SUCCEEDED(lib->queryInterface(ISlangSharedLibrary_Dep1::getTypeGuid(), (void**)libDep1.writeRef())))
	{
		// Okay, we need to adapt for now
		outSharedLibrary = new SharedLibraryDep1Adapter(libDep1);
		return SLANG_OK;
	}
	return SLANG_E_NOT_FOUND;
}

}
