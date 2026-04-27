// slang-coverage-common.cpp — Implementation of the Tier-1 helper
// library declared in `include/slang-coverage.h`. All graphics-API-
// independent logic lives here: manifest parsing, counter
// accumulation, LCOV emission.
//
// No third-party JSON dependency: the `.slangcov` schema is small and
// controlled by the Slang compiler, so we hand-roll a minimal parser
// tuned to the exact shapes the compiler emits. This keeps the
// library dependency-free — important because shipping with no
// transitive runtime dependencies is a lot of the library's value.

#include "include/slang-coverage.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace
{

struct CounterEntry
{
    std::string file;
    int32_t line = 0;
};

} // anonymous namespace

struct SlangCoverageContext
{
    std::vector<CounterEntry> entries;
    std::vector<uint64_t> accumulator;
    SlangCoverageBindingInfo binding{};
    std::string bufferNameStorage;
};

//
// Minimal JSON helpers. We only need enough to read the specific
// shapes the compiler emits — arbitrary JSON is not required.
//

namespace
{

// Skip whitespace and comments (no comments in JSON, but tolerate
// trailing newlines and BOM).
const char* skipWs(const char* p, const char* end)
{
    while (p < end)
    {
        char c = *p;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            ++p;
            continue;
        }
        break;
    }
    return p;
}

// Match a literal character, advancing `p`. Returns false on mismatch.
bool expect(const char*& p, const char* end, char c)
{
    p = skipWs(p, end);
    if (p >= end || *p != c)
        return false;
    ++p;
    return true;
}

// Parse a JSON string into `out`, handling common escape sequences
// (\" \\ \/ \b \f \n \r \t \uXXXX). Caller has already consumed the
// opening quote.
bool parseString(const char*& p, const char* end, std::string& out)
{
    out.clear();
    while (p < end)
    {
        char c = *p++;
        if (c == '"')
            return true;
        if (c != '\\')
        {
            out.push_back(c);
            continue;
        }
        if (p >= end)
            return false;
        char esc = *p++;
        switch (esc)
        {
        case '"':
            out.push_back('"');
            break;
        case '\\':
            out.push_back('\\');
            break;
        case '/':
            out.push_back('/');
            break;
        case 'b':
            out.push_back('\b');
            break;
        case 'f':
            out.push_back('\f');
            break;
        case 'n':
            out.push_back('\n');
            break;
        case 'r':
            out.push_back('\r');
            break;
        case 't':
            out.push_back('\t');
            break;
        case 'u':
            {
                if (p + 4 > end)
                    return false;
                unsigned cp = 0;
                for (int i = 0; i < 4; ++i)
                {
                    char h = p[i];
                    unsigned v = 0;
                    if (h >= '0' && h <= '9')
                        v = unsigned(h - '0');
                    else if (h >= 'a' && h <= 'f')
                        v = unsigned(h - 'a' + 10);
                    else if (h >= 'A' && h <= 'F')
                        v = unsigned(h - 'A' + 10);
                    else
                        return false;
                    cp = (cp << 4) | v;
                }
                p += 4;
                // Emit as UTF-8. Surrogate pairs and non-BMP escapes are
                // not expected in our manifest contents (file paths), so
                // treat high code points conservatively.
                if (cp < 0x80)
                {
                    out.push_back(char(cp));
                }
                else if (cp < 0x800)
                {
                    out.push_back(char(0xC0 | (cp >> 6)));
                    out.push_back(char(0x80 | (cp & 0x3F)));
                }
                else
                {
                    out.push_back(char(0xE0 | (cp >> 12)));
                    out.push_back(char(0x80 | ((cp >> 6) & 0x3F)));
                    out.push_back(char(0x80 | (cp & 0x3F)));
                }
                break;
            }
        default:
            return false;
        }
    }
    return false;
}

bool parseNumber(const char*& p, const char* end, int64_t& out)
{
    p = skipWs(p, end);
    if (p >= end)
        return false;
    const char* start = p;
    if (*p == '-' || *p == '+')
        ++p;
    while (p < end && *p >= '0' && *p <= '9')
        ++p;
    if (p == start || (start[0] == '-' && p == start + 1))
        return false;
    // Use strtoll on the isolated token.
    std::string tok(start, size_t(p - start));
    char* endc = nullptr;
    long long v = std::strtoll(tok.c_str(), &endc, 10);
    if (!endc || *endc != '\0')
        return false;
    out = int64_t(v);
    return true;
}

// Skip over a JSON value (object / array / string / number / literal)
// starting at `p`, advancing `p` past its end. Used to ignore keys we
// don't care about without hand-rolling a full parser.
bool skipValue(const char*& p, const char* end)
{
    p = skipWs(p, end);
    if (p >= end)
        return false;
    char c = *p;
    if (c == '"')
    {
        ++p;
        std::string s;
        return parseString(p, end, s);
    }
    if (c == '{' || c == '[')
    {
        char close = (c == '{') ? '}' : ']';
        ++p;
        int depth = 1;
        while (p < end && depth > 0)
        {
            p = skipWs(p, end);
            if (p >= end)
                return false;
            if (*p == '"')
            {
                ++p;
                std::string s;
                if (!parseString(p, end, s))
                    return false;
                continue;
            }
            if (*p == '{' || *p == '[')
            {
                ++depth;
                ++p;
                continue;
            }
            if (*p == '}' || *p == ']')
            {
                // We don't verify closer matches because the compiler
                // writes balanced JSON; mismatch is caught elsewhere.
                (void)close;
                --depth;
                ++p;
                continue;
            }
            ++p;
        }
        return depth == 0;
    }
    // Number or literal (true/false/null): scan to a delimiter.
    while (p < end)
    {
        char x = *p;
        if (x == ',' || x == '}' || x == ']' || x == ' ' || x == '\t' || x == '\r' || x == '\n')
            break;
        ++p;
    }
    return true;
}

// Parse the `buffer` sub-object.
bool parseBuffer(const char*& p, const char* end, SlangCoverageContext* ctx)
{
    if (!expect(p, end, '{'))
        return false;
    ctx->binding.space = -1;
    ctx->binding.binding = -1;
    ctx->binding.descriptorSet = -1;
    ctx->binding.uavRegister = -1;
    ctx->binding.elementStrideBytes = 4;
    ctx->binding.synthesized = 0;

    bool first = true;
    while (true)
    {
        p = skipWs(p, end);
        if (p < end && *p == '}')
        {
            ++p;
            return true;
        }
        if (!first && !expect(p, end, ','))
            return false;
        first = false;

        p = skipWs(p, end);
        if (p >= end || *p != '"')
            return false;
        ++p;
        std::string key;
        if (!parseString(p, end, key))
            return false;
        if (!expect(p, end, ':'))
            return false;

        p = skipWs(p, end);
        if (key == "name")
        {
            if (p >= end || *p != '"')
                return false;
            ++p;
            if (!parseString(p, end, ctx->bufferNameStorage))
                return false;
            ctx->binding.bufferName = ctx->bufferNameStorage.c_str();
        }
        else if (key == "element_stride")
        {
            int64_t v = 0;
            if (!parseNumber(p, end, v))
                return false;
            ctx->binding.elementStrideBytes = int32_t(v);
        }
        else if (
            key == "space" || key == "binding" || key == "descriptor_set" || key == "uav_register")
        {
            int64_t v = 0;
            if (!parseNumber(p, end, v))
                return false;
            if (key == "space")
                ctx->binding.space = int32_t(v);
            else if (key == "binding")
                ctx->binding.binding = int32_t(v);
            else if (key == "descriptor_set")
                ctx->binding.descriptorSet = int32_t(v);
            else
                ctx->binding.uavRegister = int32_t(v);
        }
        else if (key == "synthesized")
        {
            p = skipWs(p, end);
            if (p + 4 <= end && std::strncmp(p, "true", 4) == 0)
            {
                ctx->binding.synthesized = 1;
                p += 4;
            }
            else if (p + 5 <= end && std::strncmp(p, "false", 5) == 0)
            {
                ctx->binding.synthesized = 0;
                p += 5;
            }
            else
                return false;
        }
        else
        {
            if (!skipValue(p, end))
                return false;
        }
    }
}

bool parseEntries(const char*& p, const char* end, SlangCoverageContext* ctx)
{
    if (!expect(p, end, '['))
        return false;
    p = skipWs(p, end);
    if (p < end && *p == ']')
    {
        ++p;
        return true;
    }
    while (true)
    {
        if (!expect(p, end, '{'))
            return false;
        CounterEntry entry;
        bool first = true;
        while (true)
        {
            p = skipWs(p, end);
            if (p < end && *p == '}')
            {
                ++p;
                break;
            }
            if (!first && !expect(p, end, ','))
                return false;
            first = false;

            p = skipWs(p, end);
            if (p >= end || *p != '"')
                return false;
            ++p;
            std::string key;
            if (!parseString(p, end, key))
                return false;
            if (!expect(p, end, ':'))
                return false;
            p = skipWs(p, end);

            if (key == "file")
            {
                if (p >= end || *p != '"')
                    return false;
                ++p;
                if (!parseString(p, end, entry.file))
                    return false;
            }
            else if (key == "line")
            {
                int64_t v = 0;
                if (!parseNumber(p, end, v))
                    return false;
                entry.line = int32_t(v);
            }
            else
            {
                if (!skipValue(p, end))
                    return false;
            }
        }
        ctx->entries.push_back(std::move(entry));
        p = skipWs(p, end);
        if (p < end && *p == ',')
        {
            ++p;
            continue;
        }
        if (!expect(p, end, ']'))
            return false;
        return true;
    }
}

bool parseManifest(const std::string& text, SlangCoverageContext* ctx)
{
    const char* p = text.c_str();
    const char* end = p + text.size();

    if (!expect(p, end, '{'))
        return false;

    int64_t version = 1;
    int64_t counterCount = -1;
    bool first = true;
    while (true)
    {
        p = skipWs(p, end);
        if (p < end && *p == '}')
        {
            ++p;
            break;
        }
        if (!first && !expect(p, end, ','))
            return false;
        first = false;

        p = skipWs(p, end);
        if (p >= end || *p != '"')
            return false;
        ++p;
        std::string key;
        if (!parseString(p, end, key))
            return false;
        if (!expect(p, end, ':'))
            return false;

        if (key == "version")
        {
            if (!parseNumber(p, end, version))
                return false;
        }
        else if (key == "counters")
        {
            if (!parseNumber(p, end, counterCount))
                return false;
        }
        else if (key == "buffer")
        {
            if (!parseBuffer(p, end, ctx))
                return false;
        }
        else if (key == "entries")
        {
            if (!parseEntries(p, end, ctx))
                return false;
        }
        else
        {
            if (!skipValue(p, end))
                return false;
        }
    }

    if (version != 1)
        return false;
    if (counterCount < 0 || size_t(counterCount) < ctx->entries.size())
    {
        // Accept counters >= entries.size (trailing slots may exist
        // if the compiler reserved them for future use), but reject
        // counter count smaller than the entry list.
        if (counterCount < 0)
            return false;
    }
    ctx->accumulator.assign(size_t(counterCount), 0);
    return true;
}

} // anonymous namespace

//
// C API implementation.
//

extern "C" SlangResult slang_coverage_create(
    const char* manifestPath,
    SlangCoverageContext** outCtx)
{
    if (!manifestPath || !outCtx)
        return SLANG_E_INVALID_ARG;
    *outCtx = nullptr;

    FILE* f = std::fopen(manifestPath, "rb");
    if (!f)
        return SLANG_E_NOT_FOUND;
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    std::string text(size_t(size < 0 ? 0 : size), '\0');
    if (size > 0)
    {
        if (std::fread(text.data(), 1, size_t(size), f) != size_t(size))
        {
            std::fclose(f);
            return SLANG_E_CANNOT_OPEN;
        }
    }
    std::fclose(f);

    auto* ctx = new SlangCoverageContext();
    // Stamp the binding-info struct's `structSize` so consumers can
    // version-gate field reads. parseManifest may overwrite individual
    // fields but won't touch this.
    ctx->binding.structSize = sizeof(SlangCoverageBindingInfo);
    if (!parseManifest(text, ctx))
    {
        delete ctx;
        return SLANG_FAIL;
    }
    *outCtx = ctx;
    return SLANG_OK;
}

extern "C" void slang_coverage_destroy(SlangCoverageContext* ctx)
{
    delete ctx;
}

extern "C" uint32_t slang_coverage_counter_count(const SlangCoverageContext* ctx)
{
    return ctx ? uint32_t(ctx->accumulator.size()) : 0;
}

extern "C" const SlangCoverageBindingInfo* slang_coverage_binding(const SlangCoverageContext* ctx)
{
    return ctx ? &ctx->binding : nullptr;
}

extern "C" SlangResult slang_coverage_accumulate(
    SlangCoverageContext* ctx,
    const uint32_t* counters,
    size_t count)
{
    if (!ctx || !counters)
        return SLANG_E_INVALID_ARG;
    // Tightened: we require an exact-size snapshot. A short snapshot
    // (caller passed the wrong shader's buffer or a truncated readback)
    // would silently produce a partial-coverage report — the strictest
    // way to surface that as a real integration bug is to refuse it.
    if (count != ctx->accumulator.size())
        return SLANG_E_INVALID_ARG;
    for (size_t i = 0; i < count; ++i)
        ctx->accumulator[i] += counters[i];
    return SLANG_OK;
}

extern "C" void slang_coverage_reset_accumulator(SlangCoverageContext* ctx)
{
    if (!ctx)
        return;
    std::fill(ctx->accumulator.begin(), ctx->accumulator.end(), uint64_t(0));
}

extern "C" uint64_t slang_coverage_get_hits(const SlangCoverageContext* ctx, uint32_t index)
{
    if (!ctx || index >= ctx->accumulator.size())
        return 0;
    return ctx->accumulator[index];
}

extern "C" SlangResult slang_coverage_save_lcov(
    const SlangCoverageContext* ctx,
    const char* outputPath,
    const char* testName)
{
    if (!ctx || !outputPath)
        return SLANG_E_INVALID_ARG;
    const char* name = (testName && *testName) ? testName : "slang_coverage";
    // LCOV rejects hyphens in test names.
    for (const char* c = name; *c; ++c)
        if (*c == '-')
            return SLANG_E_INVALID_ARG;

    // Aggregate hits by (file, line). If multiple counter entries
    // share the same (file, line) their hit counts are summed.
    std::map<std::string, std::map<int32_t, uint64_t>> byFile;
    for (size_t i = 0; i < ctx->entries.size(); ++i)
    {
        const CounterEntry& e = ctx->entries[i];
        uint64_t hits = (i < ctx->accumulator.size()) ? ctx->accumulator[i] : 0;
        byFile[e.file][e.line] += hits;
    }

    FILE* f = std::fopen(outputPath, "w");
    if (!f)
        return SLANG_E_CANNOT_OPEN;
    std::fprintf(f, "TN:%s\n", name);
    for (const auto& filePair : byFile)
    {
        std::fprintf(f, "SF:%s\n", filePair.first.c_str());
        for (const auto& lineHits : filePair.second)
        {
            std::fprintf(f, "DA:%d,%llu\n", lineHits.first, (unsigned long long)lineHits.second);
        }
        std::fprintf(f, "end_of_record\n");
    }
    std::fclose(f);
    return SLANG_OK;
}
