// test-context.cpp
#include "test-context.h"

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-string-util.h"

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
        const auto& tool = pair.Value;
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
    sharedLibToolBuilder.append("-tool");

    StringBuilder builder;
    SharedLibrary::appendPlatformFileName(sharedLibToolBuilder.getUnownedSlice(), builder);
    String path = Path::combine(dirPath, builder);

    SharedLibraryTool tool = {};

    if (SLANG_SUCCEEDED(SharedLibrary::loadWithPlatformPath(path.begin(), tool.m_sharedLibrary)))
    {
        tool.m_func = (InnerMainFunc)SharedLibrary::findFuncByName(tool.m_sharedLibrary, "innerMain");
    }

    m_sharedLibTools.Add(name, tool);
    return tool.m_func;
}

void TestContext::setInnerMainFunc(const String& name, InnerMainFunc func)
{
    SharedLibraryTool* tool = m_sharedLibTools.TryGetValue(name);
    if (tool)
    {
        if (tool->m_sharedLibrary)
        {
            SharedLibrary::unload(tool->m_sharedLibrary);
            tool->m_sharedLibrary = nullptr;
        }

        tool->m_func = func;
    }
    else
    {
        SharedLibraryTool tool = {};
        tool.m_func = func;
        m_sharedLibTools.Add(name, tool);
    }
}

CPPCompilerSet* TestContext::getCPPCompilerSet()
{
    if (!cppCompilerSet)
    {
        cppCompilerSet = new CPPCompilerSet;

        CPPCompilerUtil::InitializeSetDesc desc;
        CPPCompilerUtil::initializeSet(desc, cppCompilerSet);
    }
    return cppCompilerSet;
}

Slang::CPPCompiler* TestContext::getDefaultCPPCompiler()
{
    CPPCompilerSet* set = getCPPCompilerSet();
    return set ? set->getDefaultCompiler() : nullptr;
}

