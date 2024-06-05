#include "slang-performance-profiler.h"
#include "slang-dictionary.h"
#include <unordered_map>
#include <mutex>

namespace Slang
{
    class PerformanceProfilerImpl : public PerformanceProfiler
    {
    public:
        OrderedDictionary<const char*, FuncProfileInfo> data;

        virtual FuncProfileContext enterFunction(const char* funcName) override
        {
            auto entry = data.TryGetValue(funcName);
            if (!entry)
            {
                data.Add(funcName, FuncProfileInfo());
                entry = data.TryGetValue(funcName);
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
            auto entry = data.TryGetValue(ctx.funcName);
            entry->duration += duration;
        }
        virtual void getResult(StringBuilder& out) override
        {
            for (auto func : data)
            {
                out << func.Key << ": \t";
                auto milliseconds = std::chrono::duration_cast< std::chrono::milliseconds >(func.Value.duration);
                out << func.Value.invocationCount << "\t" << milliseconds.count() << "ms\n";
            }
        }

        virtual void clear() override
        {
            data.Clear();
        }
    };

    PerformanceProfiler* Slang::PerformanceProfiler::getProfiler()
    {
        thread_local static PerformanceProfilerImpl profiler = PerformanceProfilerImpl();
        return &profiler;
    }


    SlangProfiler::SlangProfiler(PerformanceProfiler* profiler)
    {
        PerformanceProfilerImpl* profilerImpl = static_cast<PerformanceProfilerImpl*>(profiler);
        size_t entryCount = profilerImpl->data.Count();

        m_profilEntries.reserve(entryCount);

        int index = 0;
        for (auto func : profilerImpl->data)
        {
            ProfileInfo profileEntry {};
            size_t strSize = std::min(sizeof(profileEntry.funcName) - 1, strlen(func.Key));

            if (strSize > 0)
            {
                memcpy(profileEntry.funcName, func.Key, strSize);
            }
            profileEntry.invocationCount = func.Value.invocationCount;
            profileEntry.duration = func.Value.duration;

            m_profilEntries.insert(index, profileEntry);
            index++;
        }
    }

    ISlangUnknown* SlangProfiler::getInterface(const Guid& guid)
    {
        if(guid == SlangProfiler::getTypeGuid())
            return static_cast<ISlangUnknown*>(this);
        else
            return nullptr;
    }

    size_t SlangProfiler::getEntryCount()
    {
        return m_profilEntries.getCount();
    }

    const char* SlangProfiler::getEntryName(uint32_t index)
    {
        if (index >= (uint32_t)m_profilEntries.getCount())
            return nullptr;

        return m_profilEntries[index].funcName;
    }

    long SlangProfiler::getEntryTimeMS(uint32_t index)
    {
        if (index >= (uint32_t)m_profilEntries.getCount())
            return 0;

        auto milliseconds = std::chrono::duration_cast< std::chrono::milliseconds >(m_profilEntries[index].duration);
        return (long)milliseconds.count();
    }

    uint32_t SlangProfiler::getEntryInvocationTimes(uint32_t index)
    {
        if (index >= (uint32_t)m_profilEntries.getCount())
            return 0;

        return m_profilEntries[index].invocationCount;
    }
}
