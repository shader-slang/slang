// unit-test-com-host-callable.cpp

#include "../../source/core/slang-byte-encode-util.h"

#include <stdio.h>
#include <stdlib.h>

#include "tools/unit-test/slang-unit-test.h"

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../../source/core/slang-list.h"

namespace { // anonymous

// Slang namespace is used for elements support code (like core) which we use here
// for ComPtr<> and TestToolUtil
using namespace Slang;

// For the moment we have to explicitly write the Slang COM interface in C++ code. It *MUST* match 
// the interface in the slang source
// As it stands all interfaces need to derive from ISlangUnknown (or IUnknown). 
class IDoThings : public ISlangUnknown
{
public:
    virtual int SLANG_MCALL doThing(int a, int b) = 0;
    virtual int SLANG_MCALL calcHash(const char* in) = 0;
};

class ICountGood : public ISlangUnknown
{
public:
    virtual int SLANG_MCALL nextCount() = 0;
};

static int _calcHash(const char* in)
{
    int hash = 0;
    for (; *in; ++in)
    {
        // A very poor hash function
        hash = hash * 13 + *in;
    }
    return hash;
}

class DoThings : public IDoThings
{
public:
    // We don't need queryInterface for this impl, or ref counting
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE { return SLANG_E_NOT_IMPLEMENTED; }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }

    // IDoThings
    virtual int SLANG_MCALL doThing(int a, int b) SLANG_OVERRIDE { return a + b + 1; }
    virtual int SLANG_MCALL calcHash(const char* in) SLANG_OVERRIDE { return (int)_calcHash(in); }
};

class CountGood : public ICountGood
{
public:
    // We don't need queryInterface for this impl, or ref counting
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE { return SLANG_E_NOT_IMPLEMENTED; }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }

    // ICountGood
    virtual int SLANG_MCALL nextCount() SLANG_OVERRIDE { return m_count++; }

    int m_count = 0;
};

static SlangResult _testComHostCallable(UnitTestContext* context)
{
    using namespace Slang;

    slang::IGlobalSession* slangSession = context->slangGlobalSession;

    // Create a compile request
    Slang::ComPtr<slang::ICompileRequest> request;
    SLANG_RETURN_ON_FAIL(slangSession->createCompileRequest(request.writeRef()));

    // We want to compile to 'HOST_CALLABLE' here such that we can execute the Slang code.
    // 
    // Note that it is possible to use HOST_HOST_CALLABLE, but this currently only works with 'regular' C++ compilers
    // not with `slang-llvm`.
    const int targetIndex = request->addCodeGenTarget(SLANG_SHADER_HOST_CALLABLE);

    // Set the target flag to indicate that we want to compile all into a library.
    request->setTargetFlags(targetIndex, SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);

    // Add the translation unit
    const int translationUnitIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

    // Set the source file for the translation unit
    request->addTranslationUnitSourceFile(translationUnitIndex, "tools/slang-unit-test/unit-test-com-host-callable.slang");

    const SlangResult compileRes = request->compile();

    // Even if there were no errors that forced compilation to fail, the
    // compiler may have produced "diagnostic" output such as warnings.
    // We will go ahead and print that output here.
    //
    if (auto diagnostics = request->getDiagnosticOutput())
    {
        printf("%s", diagnostics);
    }

    // Get the 'shared library' (note that this doesn't necessarily have to be implemented as a shared library
    // it's just an interface to executable code).
    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SLANG_RETURN_ON_FAIL(request->getTargetHostCallable(0, sharedLibrary.writeRef()));

    {
        typedef const char* (*Func)(const char*);
        Func func = (Func)sharedLibrary->findFuncByName("getString");

        if (!func)
        {
            return SLANG_FAIL;
        }

        String text = "Hello World!";
        String returnedText = func(text.getBuffer());

        SLANG_CHECK(text == returnedText);
    }
    {
        typedef int (*Func)(const char* text, IDoThings* doThings);

        Func func = (Func)sharedLibrary->findFuncByName("calcHash");

        if (!func)
        {
            return SLANG_FAIL;
        }

        DoThings doThings;

        String text("Hello");

        const int hash = func(text.getBuffer(), &doThings);

        SLANG_CHECK(hash == _calcHash(text.getBuffer()));
    }

    // Check accessing a global
    {
        typedef void (*SetFunc)(int v);
        typedef int (*GetFunc)();

        const auto setGlobal = (SetFunc)sharedLibrary->findFuncByName("setGlobal");
        const auto getGlobal = (GetFunc)sharedLibrary->findFuncByName("getGlobal");

        if (setGlobal == nullptr || getGlobal == nullptr)
        {
            return SLANG_FAIL;
        }

        // In the slang source it is set a default value
        SLANG_CHECK(getGlobal() == 10);

        for (Index i = 0; i < 10; ++i)
        {
            setGlobal(int(i));
            SLANG_CHECK(getGlobal() == i);
        }
    }

    // Check using a global interface
    {

        typedef void (*SetCounterFunc)(ICountGood* counter);
        typedef int (*NextCountFunc)();

        const auto setCounter = (SetCounterFunc)sharedLibrary->findFuncByName("setCounter");
        const auto nextCount = (NextCountFunc)sharedLibrary->findFuncByName("nextCount");

        if (setCounter == nullptr || nextCount == nullptr)
        {
            return SLANG_FAIL;
        }

        CountGood counter;

        setCounter(&counter);

        for (Index i = 0; i < 10; ++i)
        {
            const auto v = nextCount();
            SLANG_CHECK(v == i);
        }

        auto counterPtr = (ICountGood**)sharedLibrary->findSymbolAddressByName("globalCounter");

        if (counterPtr)
        {
            SLANG_CHECK(*counterPtr == &counter);
        }
    }

    return SLANG_OK;
}

} // anonymous

SLANG_UNIT_TEST(comHostCallable)
{
    auto testResult = _testComHostCallable(unitTestContext);
    SLANG_CHECK(SLANG_SUCCEEDED(testResult));
}
