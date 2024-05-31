#include "slang-performance-profiler.h"
#include "slang-dictionary.h"
#include <unordered_map>

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
            const std::lock_guard<std::mutex> scopeLock(m_mutex);

            for (auto func : profilerImpl->data)
            {
                std::chrono::nanoseconds ns = func.value.duration;

                auto entry = m_compileTimeProfiler.find(func.key);
                if (entry == m_compileTimeProfiler.end())
                {
                    m_compileTimeProfiler.emplace(std::make_pair(func.key, FuncProfileInfo()));
                    entry = m_compileTimeProfiler.find(func.key);
                }
                entry->second.invocationCount += func.value.invocationCount;
                entry->second.duration += ns;
            }

            for (auto entry : m_compileTimeProfiler)
            {
                auto milliseconds = std::chrono::duration_cast< std::chrono::milliseconds >(entry.second.duration);
                out << entry.first.c_str() << ": \t" << entry.second.invocationCount << "\t" << milliseconds.count() << " ms\n";
            }
       }
    private:
        // Those are supposed to accumulate the time spent in each thread
        std::unordered_map<std::string, FuncProfileInfo> m_compileTimeProfiler;
        std::mutex m_mutex;
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
