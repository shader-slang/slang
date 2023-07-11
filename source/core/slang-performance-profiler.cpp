#include "slang-performance-profiler.h"
#include "slang-dictionary.h"

namespace Slang
{
    struct FuncProfileInfo
    {
        int invocationCount = 0;
        std::chrono::nanoseconds duration = std::chrono::nanoseconds::zero();
    };
    class PerformanceProfilerImpl : public PerformanceProfiler
    {
    public:
        OrderedDictionary<const char*, FuncProfileInfo> data;

        virtual FuncProfileContext enterFunction(const char* funcName) override
        {
            auto entry = data.tryGetValue(funcName);
            if (!entry)
            {
                data.add(funcName, FuncProfileInfo());
                entry = data.tryGetValue(funcName);
            }
            entry->invocationCount++;
            FuncProfileContext ctx;
            ctx.funcName = funcName;
            ctx.startTime = std::chrono::high_resolution_clock::now();
            return ctx;
        }
        virtual void exitFunction(FuncProfileContext ctx) override
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = endTime - ctx.startTime;
            auto entry = data.tryGetValue(ctx.funcName);
            entry->duration += duration;
        }
        virtual void getResult(StringBuilder& out) override
        {
            for (auto func : data)
            {
                out << func.key << ": \t";
                out << func.value.invocationCount << "\t" << func.value.duration.count()/1000000 << "\n";
            }
        }
    };

    PerformanceProfiler* Slang::PerformanceProfiler::getProfiler()
    {
        static PerformanceProfilerImpl profiler = PerformanceProfilerImpl();
        return &profiler;
    }
}
