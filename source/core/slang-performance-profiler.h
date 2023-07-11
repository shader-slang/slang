#ifndef SLANG_CORE_PERFORMANCE_PROFILER_H
#define SLANG_CORE_PERFORMANCE_PROFILER_H

#include "slang-string.h"
#include <chrono>

namespace Slang
{

struct FuncProfileContext
{
    const char* funcName = nullptr;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

class PerformanceProfiler
{
public:
    virtual FuncProfileContext enterFunction(const char* funcName) = 0;
    virtual void exitFunction(FuncProfileContext context) = 0;
    virtual void getResult(StringBuilder& out) = 0;
public:
    static PerformanceProfiler* getProfiler();
};

struct PerformanceProfilerFuncRAIIContext
{
    FuncProfileContext context;
    PerformanceProfilerFuncRAIIContext(const char* funcName)
    {
        context = PerformanceProfiler::getProfiler()->enterFunction(funcName);
    }
    ~PerformanceProfilerFuncRAIIContext()
    {
        PerformanceProfiler::getProfiler()->exitFunction(context);
    }
};

#define SLANG_PROFILE PerformanceProfilerFuncRAIIContext _profileContext(__func__)
}

#endif
