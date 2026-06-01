// unit-test-llvm-version-skew.cpp

// Regression test for shader-slang/slang#11388.
//
// When the loaded `slang-llvm` library predates the `createLLVMBuilder_V3`
// builder ABI (i.e. the prebuilt is older than the compiler), the builder
// symbol lookup in slang-emit-llvm.cpp returns null. Previously that path did a
// silent `return SLANG_FAIL`, surfacing only as a cryptic downstream link error
// ("cannot find shader.o"). It now emits an actionable diagnostic naming the
// missing symbol.
//
// The skew can't be reproduced from a from-source build (the locally built/
// fetched libslang-llvm exports the matching symbol). So this test installs a
// shared-library loader that wraps the real `slang-llvm` and hides just the
// builder symbol, making the test independent of which slang-llvm is present.

#include "../../source/core/slang-shared-library.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

namespace
{ // anonymous

using namespace Slang;

// Wraps a real ISlangSharedLibrary but reports one symbol as absent.
class SymbolHidingLibrary : public ComBaseObject, public ISlangSharedLibrary
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ICastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        if (auto intf = getInterface(guid))
            return intf;
        // Delegate any other representation queries to the real library.
        return m_inner->castAs(guid);
    }

    // ISlangSharedLibrary
    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name)
        SLANG_OVERRIDE
    {
        if (m_hiddenSymbol.getLength() && m_hiddenSymbol == UnownedStringSlice(name))
            return nullptr;
        return m_inner->findSymbolAddressByName(name);
    }

    SymbolHidingLibrary(ISlangSharedLibrary* inner, const char* hiddenSymbol)
        : m_inner(inner), m_hiddenSymbol(hiddenSymbol)
    {
    }

protected:
    void* getInterface(const Guid& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid() || guid == ISlangCastable::getTypeGuid() ||
            guid == ISlangSharedLibrary::getTypeGuid())
        {
            return static_cast<ISlangSharedLibrary*>(this);
        }
        return nullptr;
    }

    ComPtr<ISlangSharedLibrary> m_inner;
    String m_hiddenSymbol;
};

// Delegates to a base loader, but wraps a named library so a symbol is hidden.
class SymbolHidingLoader : public ComBaseObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_COM_BASE_IUNKNOWN_ALL

    // ISlangSharedLibraryLoader
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outSharedLibrary) SLANG_OVERRIDE
    {
        ComPtr<ISlangSharedLibrary> inner;
        SLANG_RETURN_ON_FAIL(m_base->loadSharedLibrary(path, inner.writeRef()));

        if (m_libraryToWrap == UnownedStringSlice(path))
        {
            ComPtr<ISlangSharedLibrary> wrapper(
                new SymbolHidingLibrary(inner, m_hiddenSymbol.getBuffer()));
            *outSharedLibrary = wrapper.detach();
            return SLANG_OK;
        }

        *outSharedLibrary = inner.detach();
        return SLANG_OK;
    }

    SymbolHidingLoader(
        ISlangSharedLibraryLoader* base,
        const char* libraryToWrap,
        const char* hiddenSymbol)
        : m_base(base), m_libraryToWrap(libraryToWrap), m_hiddenSymbol(hiddenSymbol)
    {
    }

protected:
    void* getInterface(const Guid& guid)
    {
        if (guid == ISlangUnknown::getTypeGuid() ||
            guid == ISlangSharedLibraryLoader::getTypeGuid())
        {
            return static_cast<ISlangSharedLibraryLoader*>(this);
        }
        return nullptr;
    }

    ComPtr<ISlangSharedLibraryLoader> m_base;
    String m_libraryToWrap;
    String m_hiddenSymbol;
};

} // namespace

SLANG_UNIT_TEST(llvmBuilderVersionSkewDiagnostic)
{
    // Use a fresh global session so the (uncached) slang-llvm load goes through
    // our loader rather than a library a previous test already cached.
    ComPtr<slang::IGlobalSession> session;
    if (SLANG_FAILED(slang::createGlobalSession(session.writeRef())))
    {
        SLANG_CHECK(false);
        return;
    }

    // Only meaningful when slang-llvm is actually available to load.
    if (SLANG_FAILED(session->checkPassThroughSupport(SLANG_PASS_THROUGH_LLVM)))
        return;

    // Hide the builder symbol the compiler needs, simulating an older prebuilt.
    ComPtr<ISlangSharedLibraryLoader> loader(new SymbolHidingLoader(
        DefaultSharedLibraryLoader::getSingleton(),
        "slang-llvm",
        "createLLVMBuilder_V3"));
    session->setSharedLibraryLoader(loader);

    Slang::ComPtr<slang::ICompileRequest> request;
    SLANG_ALLOW_DEPRECATED_BEGIN
    if (SLANG_FAILED(session->createCompileRequest(request.writeRef())))
    {
        SLANG_CHECK(false);
        return;
    }
    SLANG_ALLOW_DEPRECATED_END

    // Direct LLVM-IR target routes through slang-emit-llvm.cpp's createLLVMBuilder lookup.
    const int targetIndex = request->addCodeGenTarget(SLANG_SHADER_LLVM_IR);
    request->setTargetFlags(targetIndex, SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);

    const int translationUnitIndex =
        request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    request->addTranslationUnitSourceFile(
        translationUnitIndex,
        "tools/slang-unit-test/unit-test-llvm-version-skew.slang");

    const SlangResult compileRes = request->compile();
    const char* diagnostics = request->getDiagnosticOutput();
    if (diagnostics)
        printf("%s", diagnostics);

    // The compile must fail with the actionable diagnostic that names the missing
    // builder symbol -- not the previous silent SLANG_FAIL.
    SLANG_CHECK(SLANG_FAILED(compileRes));
    SLANG_CHECK(diagnostics != nullptr);
    SLANG_CHECK(diagnostics && strstr(diagnostics, "createLLVMBuilder_V3") != nullptr);
    SLANG_CHECK(diagnostics && strstr(diagnostics, "slang-llvm") != nullptr);
}
