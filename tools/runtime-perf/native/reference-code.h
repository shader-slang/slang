// A single explicitly selected reference binary, supplied through RHI's cache API.
// Reflection still comes from the matching Slang module. No persistent cache is modified.
#include <atomic>
#include <mutex>

class ReferenceBlob : public ISlangBlob
{
public:
    std::vector<char> bytes;
    std::atomic<uint32_t> refs{1};
    SLANG_NO_THROW const void* SLANG_MCALL getBufferPointer() override { return bytes.data(); }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() override { return bytes.size(); }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return ++refs; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override
    {
        uint32_t n = --refs;
        if (!n)
            delete this;
        return n;
    }
    SLANG_NO_THROW Result SLANG_MCALL queryInterface(const SlangUUID& id, void** out) override
    {
        *out = nullptr;
        if (id != ISlangBlob::getTypeGuid() && id != ISlangUnknown::getTypeGuid())
            return SLANG_E_NO_INTERFACE;
        *out = this;
        addRef();
        return SLANG_OK;
    }
};

class ReferenceCode : public IPersistentCache
{
public:
    // Owned by Benchmark, which destroys the device before this member.
    ComPtr<ISlangBlob> key, code;
    std::atomic<uint32_t> hits{0}, misses{0};
    std::mutex mutex;
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return 2; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override { return 2; }
    SLANG_NO_THROW Result SLANG_MCALL queryInterface(const SlangUUID& id, void** out) override
    {
        *out = nullptr;
        if (id != IPersistentCache::getTypeGuid() && id != ISlangUnknown::getTypeGuid())
            return SLANG_E_NO_INTERFACE;
        *out = this;
        return SLANG_OK;
    }
    SLANG_NO_THROW Result SLANG_MCALL writeCache(ISlangBlob*, ISlangBlob*) override
    {
        return SLANG_E_NOT_IMPLEMENTED;
    }
    SLANG_NO_THROW Result SLANG_MCALL queryCache(ISlangBlob* requested, ISlangBlob** out) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        *out = nullptr;
        if (!key || requested->getBufferSize() != key->getBufferSize() ||
            memcmp(requested->getBufferPointer(), key->getBufferPointer(), key->getBufferSize()))
        {
            ++misses;
            return SLANG_E_NOT_FOUND;
        }
        ++hits;
        *out = code.get();
        code->addRef();
        return SLANG_OK;
    }
    bool load(const std::string& path)
    {
        FILE* file = fopen(path.c_str(), "rb");
        if (!file)
            return false;
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        rewind(file);
        if (size <= 0)
        {
            fclose(file);
            return false;
        }
        auto blob = new ReferenceBlob;
        blob->bytes.resize(size);
        bool ok = fread(blob->bytes.data(), 1, size, file) == size;
        fclose(file);
        code.attach(blob);
        return ok;
    }
};
