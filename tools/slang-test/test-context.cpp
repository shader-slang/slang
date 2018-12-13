// test-context.cpp
#include "test-context.h"

#include "os.h"
#include "../../source/core/slang-string-util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

TestContext::TestContext() 
{
    m_session = nullptr;
}

Result TestContext::init()
{
    m_session = spCreateSession(nullptr);
    if (!m_session)
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

TestContext::~TestContext()
{
    if (m_session)
    {
        spDestroySession(m_session);
    }
}

TestContext::InnerMainFunc TestContext::getInnerMainFunc(const String& dirPath, const String& name)
{
    {
        SharedLibraryTool* tool = m_sharedLibTools.TryGetValue(name);
        if (tool)
        {
            return tool->m_func;
        }
    }

    StringBuilder sharedLibToolBuilder;
    sharedLibToolBuilder.append(name);
    sharedLibToolBuilder.append("-shared-library");

    StringBuilder builder;
    SharedLibrary::appendPlatformFileName(sharedLibToolBuilder.getUnownedSlice(), builder);
    String path = Path::Combine(dirPath, builder);

    SharedLibraryTool tool = {};

    if (SLANG_SUCCEEDED(SharedLibrary::loadWithPlatformFilename(path.begin(), tool.m_sharedLibrary)))
    {
        tool.m_func = (InnerMainFunc)SharedLibrary::findFuncByName(tool.m_sharedLibrary, "innerMain");
    }

    m_sharedLibTools.Add(name, tool);
    return tool.m_func;
}
