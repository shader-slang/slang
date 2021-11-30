// test-context.cpp
#include "test-context.h"

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-shared-library.h"

#include "../../source/core/slang-test-tool-util.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

TestContext::TestContext() 
{
    m_session = nullptr;

    /// if we are testing on arm, debug, we may want to increase the connection timeout
#if (SLANG_PROCESSOR_ARM || SLANG_PROCESSOR_ARM_64) && defined(_DEBUG)
    // 5 mins(!). This seems to be the order of time needed for timeout on a CI ARM test system on debug
    connectionTimeOutInMs = 1000 * 60 * 5;
#endif
}

Result TestContext::init(const char* exePath)
{
    m_session = spCreateSession(nullptr);
    if (!m_session)
    {
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(TestToolUtil::getExeDirectoryPath(exePath, exeDirectoryPath));
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
    sharedLibToolBuilder.append("-tool");

    StringBuilder builder;
    SharedLibrary::appendPlatformFileName(sharedLibToolBuilder.getUnownedSlice(), builder);
    String path = Path::combine(dirPath, builder);

    DefaultSharedLibraryLoader* loader = DefaultSharedLibraryLoader::getSingleton();

    SharedLibraryTool tool = {};

    if (SLANG_SUCCEEDED(loader->loadPlatformSharedLibrary(path.begin(), tool.m_sharedLibrary.writeRef())))
    {
        tool.m_func = (InnerMainFunc)tool.m_sharedLibrary->findFuncByName("innerMain");
    }

    m_sharedLibTools.Add(name, tool);
    return tool.m_func;
}

void TestContext::setInnerMainFunc(const String& name, InnerMainFunc func)
{
    SharedLibraryTool* tool = m_sharedLibTools.TryGetValue(name);
    if (tool)
    {
        tool->m_sharedLibrary.setNull();
        tool->m_func = func;
    }
    else
    {
        SharedLibraryTool tool = {};
        tool.m_func = func;
        m_sharedLibTools.Add(name, tool);
    }
}

DownstreamCompilerSet* TestContext::getCompilerSet()
{
    if (!compilerSet)
    {
        compilerSet = new DownstreamCompilerSet;

        DownstreamCompilerLocatorFunc locators[int(SLANG_PASS_THROUGH_COUNT_OF)] = { nullptr };

        DownstreamCompilerUtil::setDefaultLocators(locators);
        for (Index i = 0; i < Index(SLANG_PASS_THROUGH_COUNT_OF); ++i)
        {
            auto locator = locators[i];
            if (locator)
            {
                locator(String(), DefaultSharedLibraryLoader::getSingleton(), compilerSet);
            }
        }

        DownstreamCompilerUtil::updateDefaults(compilerSet);
    }
    return compilerSet;
}

SlangResult TestContext::_createJSONRPCConnection(RefPtr<JSONRPCConnection>& out)
{
    RefPtr<Process> process;

    {
        CommandLine cmdLine;
        cmdLine.setExecutable(exeDirectoryPath.getBuffer(), "test-server");
        SLANG_RETURN_ON_FAIL(Process::create(cmdLine, Process::Flag::AttachDebugger, process));
    }

    Stream* writeStream = process->getStream(Process::StreamType::StdIn);
    RefPtr<BufferedReadStream> readStream(new BufferedReadStream(process->getStream(Process::StreamType::StdOut)));

    RefPtr<HTTPPacketConnection> connection = new HTTPPacketConnection(readStream, writeStream);
    RefPtr<JSONRPCConnection> rpcConnection = new JSONRPCConnection;

    SLANG_RETURN_ON_FAIL(rpcConnection->init(connection, process));

    out = rpcConnection;

    return SLANG_OK;
}


void TestContext::destroyRPCConnection()
{
    if (m_jsonRpcConnection)
    {
        m_jsonRpcConnection->disconnect();
        m_jsonRpcConnection.setNull();
    }
}

Slang::JSONRPCConnection* TestContext::getOrCreateJSONRPCConnection()
{
    if (!m_jsonRpcConnection)
    {
        if (SLANG_FAILED(_createJSONRPCConnection(m_jsonRpcConnection)))
        {
            return nullptr;
        }
    }

    return m_jsonRpcConnection;
}


Slang::DownstreamCompiler* TestContext::getDefaultCompiler(SlangSourceLanguage sourceLanguage)
{
    DownstreamCompilerSet* set = getCompilerSet();
    return set ? set->getDefaultCompiler(sourceLanguage) : nullptr;
}

bool TestContext::canRunTestWithRenderApiFlags(Slang::RenderApiFlags requiredFlags)
{
    // If only allow tests that use API - then the requiredFlags must be 0
    if (options.apiOnly && requiredFlags == 0)
    {
        return false;
    }
    // Are the required rendering APIs enabled from the -api command line switch
    return (requiredFlags & options.enabledApis) == requiredFlags;
}

SpawnType TestContext::getFinalSpawnType(SpawnType spawnType)
{
    if (spawnType == SpawnType::Default)
    {
        if (options.outputMode == TestOutputMode::Default)
        {
            return SpawnType::UseSharedLibrary;
        }
        else
        {
            return SpawnType::UseTestServer;
        }
    }

    // Just return whatever spawnType was passed in
    return spawnType;
}

SpawnType TestContext::getFinalSpawnType()
{
    return getFinalSpawnType(options.defaultSpawnType);
}
