#include "slang-performance-profiler.h"
#include "slang-dictionary.h"
#include <atomic>

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
                auto milliseconds = std::chrono::duration_cast< std::chrono::milliseconds >(func.value.duration);
                out << func.value.invocationCount << "\t" << milliseconds.count() << "ms\n";
            }
        }
    };

    class SerialPerformaceProfilerImpl : public SerialPerformaceProfiler
    {
    public:
       void accumulateResults(StringBuilder& out, PerformanceProfiler* profile)
       {
           PerformanceProfilerImpl* profilerImpl = static_cast<PerformanceProfilerImpl*>(profile);
            for (auto func : profilerImpl->data)
            {
                auto milliseconds = std::chrono::duration_cast< std::chrono::milliseconds >(func.value.duration);
                long ms = milliseconds.count();

                if (!strcmp(func.key, "checkAllTranslationUnits"))
                {
                    m_semanticsCheckTime += ms;
                }
                else if (!strcmp(func.key, "generateIRForTranslationUnit"))
                {
                    m_IRGenTime += ms;
                }
                else if (!strcmp(func.key, "performMandatoryEarlyInlining"))
                {
                    m_earlyInlneTime += ms;
                }
                else if (!strcmp(func.key, "linkAndOptimizeIR"))
                {
                    m_linkIRTime += ms;
                }
                else if (!strcmp(func.key, "linkIR"))
                {
                    m_optimizeIRTime += ms;
                }
            }
            out << "Semantics Check Time: " << SerialPerformaceProfilerImpl::m_semanticsCheckTime.load() << "ms\n";
            out << "IR Generation Time: " << SerialPerformaceProfilerImpl::m_IRGenTime.load() << "ms\n";
            out << "Early Inline Time: " << SerialPerformaceProfilerImpl::m_earlyInlneTime.load() << "ms\n";
            out << "Link IR Time: " << SerialPerformaceProfilerImpl::m_linkIRTime.load() << "ms\n";
            out << "Optimize IR Time: " << SerialPerformaceProfilerImpl::m_optimizeIRTime.load() << "ms\n";
       }
    private:
        // Those are supposed to accumulate the time spent in each thread
        std::atomic<long> m_semanticsCheckTime;
        std::atomic<long> m_IRGenTime;
        std::atomic<long> m_earlyInlneTime;
        std::atomic<long> m_linkIRTime;
        std::atomic<long> m_optimizeIRTime;
    };

    // SerialPerformaceProfilerImpl profiler is intent to be shared by all threads, it's thread safe object
    // because the only member function is re-entrant function, all the member variables are atomic.
    SerialPerformaceProfiler* Slang::SerialPerformaceProfiler::getProfiler()
    {
        static SerialPerformaceProfilerImpl profiler = SerialPerformaceProfilerImpl();
        return &profiler;
    }

    PerformanceProfiler* Slang::PerformanceProfiler::getProfiler()
    {
        thread_local static PerformanceProfilerImpl profiler = PerformanceProfilerImpl();
        return &profiler;
    }
}
