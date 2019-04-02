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
    for (auto& pair : m_sharedLibTools)
    {
        auto& tool = pair.Value;
        tool.m_testTool.setNull();

        if (tool.m_sharedLibrary)
        {
            SharedLibrary::unload(tool.m_sharedLibrary);
        }
    }

    if (m_session)
    {
        spDestroySession(m_session);
    }
}

static const Guid IID_ITestTool = SLANG_UUID_Slang_ITestTool;

ITestTool* TestContext::getTestTool(const String& dirPath, const String& name)
{
    {
        SharedLibraryTool* tool = m_sharedLibTools.TryGetValue(name);
        if (tool)
        {
            return tool->m_testTool;
        }
    }

    StringBuilder sharedLibToolBuilder;
    sharedLibToolBuilder.append(name);
    sharedLibToolBuilder.append("-tool");

    StringBuilder builder;
    SharedLibrary::appendPlatformFileName(sharedLibToolBuilder.getUnownedSlice(), builder);
    String path = Path::Combine(dirPath, builder);

    SharedLibraryTool tool = {};

    if (SLANG_SUCCEEDED(SharedLibrary::loadWithPlatformFilename(path.begin(), tool.m_sharedLibrary)))
    {
        auto getTestToolFunc = (TestToolUtil::getTestToolFunc)SharedLibrary::findFuncByName(tool.m_sharedLibrary, "getTestTool");

        if (getTestToolFunc)
        {
            ComPtr<ITestTool> testTool;

            if (SLANG_SUCCEEDED(getTestToolFunc(IID_ITestTool, (ISlangUnknown**)testTool.writeRef())))
            {
                tool.m_testTool = testTool;
            }
        }
    }

    m_sharedLibTools.Add(name, tool);
    return tool.m_testTool;
}

void TestContext::setTestTool(const String& name, ITestTool* testTool)
{
    SharedLibraryTool* tool = m_sharedLibTools.TryGetValue(name);
    if (tool)
    {
        if (tool->m_sharedLibrary)
        {
            SharedLibrary::unload(tool->m_sharedLibrary);
            tool->m_sharedLibrary = nullptr;
        }

        tool->m_testTool = testTool;
    }
    else
    {
        SharedLibraryTool tool = {};
        tool.m_testTool = testTool;
        m_sharedLibTools.Add(name, tool);
    }
}
