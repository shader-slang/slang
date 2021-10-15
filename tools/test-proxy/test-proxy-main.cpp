// main.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../source/core/slang-secure-crt.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-list.h"
#include "../../source/core/slang-string.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-string-slice-pool.h"
#include "../../source/core/slang-writer.h"
#include "../../source/core/slang-file-system.h"
#include "../../source/core/slang-shared-library.h"

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-test-tool-util.h"

namespace TestProxy
{
using namespace Slang;

static SlangResult execute(int argc, const char* const* argv)
{
    typedef Slang::TestToolUtil::InnerMainFunc InnerMainFunc;

    if (argc < 2)
    {
        return SLANG_FAIL;
    }

    String exePath = Path::getParentDirectory(argv[0]);

    // Assume we will used the shared session
    ComPtr<slang::IGlobalSession> session;

    // The 'exeName' of the tool. 
    const String toolName = argv[1];

    // The sharedSession always has a pre-loaded stdlib, is sharedSession is not nullptr.
    // This differed test checks if the command line has an option to setup the stdlib.
    // If so we *don't* use the sharedSession, and create a new stdlib-less session just for this compilation. 
    if (TestToolUtil::hasDeferredStdLib(Index(argc - 2), argv + 2))
    {
        SLANG_RETURN_ON_FAIL(slang_createGlobalSessionWithoutStdLib(SLANG_API_VERSION, session.writeRef()));
        TestToolUtil::setSessionDefaultPreludeFromExePath(argv[0], session);
    }
    else
    {
        // Just create the global session in the regular way if there isn't one set
        SLANG_RETURN_ON_FAIL(slang_createGlobalSession(SLANG_API_VERSION, session.writeRef()));
        TestToolUtil::setSessionDefaultPreludeFromExePath(argv[0], session);
    }

    StringBuilder sharedLibToolBuilder;
    sharedLibToolBuilder.append(toolName);
    sharedLibToolBuilder.append("-tool");

    auto toolPath = Path::combine(exePath, sharedLibToolBuilder);

    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SLANG_RETURN_ON_FAIL(DefaultSharedLibraryLoader::getSingleton()->loadSharedLibrary(toolPath.getBuffer(), sharedLibrary.writeRef()));

    auto func = (InnerMainFunc)sharedLibrary->findFuncByName("innerMain");
    if (!func)
    {
        return SLANG_FAIL;
    }

    // Work out the args sent to the shared library
    List<const char*> args;
    args.add(argv[0]);
    args.addRange(argv + 2, Index(argc - 2));

    RefPtr<StdWriters> stdWriters = StdWriters::createDefault();

    const SlangResult res = func(stdWriters, session, int(args.getCount()), args.begin());

    return res;
}

} // namespace CppExtract

int main(int argc, const char* const* argv)
{
    using namespace TestProxy;
    SlangResult res = execute(argc, argv);
    return (int)TestToolUtil::getReturnCode(res);
}
