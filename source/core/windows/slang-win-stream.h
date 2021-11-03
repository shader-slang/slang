#ifndef SLANG_CORE_WIN_STREAM_H
#define SLANG_CORE_WIN_STREAM_H

#include "../../../slang.h"

#include "../slang-stream.h"

#ifdef _WIN32
// Include Windows header in a way that minimized namespace pollution.
// TODO: We could try to avoid including this at all, but it would
// mean trying to hide certain struct layouts, which would add
// more dynamic allocation.
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <Windows.h>
#   undef WIN32_LEAN_AND_MEAN
#   undef NOMINMAX
#endif

namespace Slang {

// Has behavior very similar to unique_ptr - assignment is a move.
class WinHandle
{
public:
    /// Detach the encapsulated handle. Returns the handle (which now must be externally handled) 
    HANDLE detach() { HANDLE handle = m_handle; m_handle = nullptr; return handle; }

    /// Return as a handle
    operator HANDLE() const { return m_handle; }

    /// Assign
    void operator=(HANDLE handle) { setNull(); m_handle = handle; }
    void operator=(WinHandle&& rhs) { HANDLE handle = m_handle; m_handle = rhs.m_handle; rhs.m_handle = handle; }

    /// Get ready for writing 
    SLANG_FORCE_INLINE HANDLE* writeRef() { setNull(); return &m_handle; }
    /// Get for read access
    SLANG_FORCE_INLINE const HANDLE* readRef() const { return &m_handle; }

    void setNull()
    {
        if (m_handle)
        {
            CloseHandle(m_handle);
            m_handle = nullptr;
        }
    }
    bool isNull() const { return m_handle == nullptr; }

    /// Ctor
    WinHandle(HANDLE handle = nullptr) :m_handle(handle) {}
    WinHandle(WinHandle&& rhs) :m_handle(rhs.m_handle) { rhs.m_handle = nullptr; }

    /// Dtor
    ~WinHandle() { setNull(); }

private:

    WinHandle(const WinHandle&) = delete;
    void operator=(const WinHandle& rhs) = delete;

    HANDLE m_handle;
};

class WinCriticalSection
{
public:
    struct Scope
    {
        Scope(WinCriticalSection* criticalSection) :
            m_criticalSection(criticalSection)
        {
            EnterCriticalSection(&criticalSection->m_criticalSection);
        }
        ~Scope()
        {
            LeaveCriticalSection(&m_criticalSection->m_criticalSection);
        }
        WinCriticalSection* m_criticalSection;
    };

    WinCriticalSection()
    {
        InitializeCriticalSection(&m_criticalSection);
    }

protected:
    CRITICAL_SECTION m_criticalSection;
};

/* This stream, uses a thread to copy data from the file handle */
class WinThreadReadStream : public Stream
{
public:
    typedef WinThreadReadStream ThisType;

    // Stream
    virtual Int64 getPosition() SLANG_OVERRIDE { return 0; }
    virtual SlangResult seek(SeekOrigin origin, Int64 offset) SLANG_OVERRIDE { SLANG_UNUSED(origin); SLANG_UNUSED(offset); return SLANG_E_NOT_AVAILABLE; }
    virtual SlangResult read(void* buffer, size_t length, size_t& outReadBytes) SLANG_OVERRIDE;
    virtual SlangResult write(const void* buffer, size_t length) SLANG_OVERRIDE { SLANG_UNUSED(buffer); SLANG_UNUSED(length); return SLANG_E_NOT_AVAILABLE; }
    virtual bool isEnd() SLANG_OVERRIDE { return m_streamHandle.isNull(); }
    virtual bool canRead() SLANG_OVERRIDE { return !m_streamHandle.isNull(); }
    virtual bool canWrite() SLANG_OVERRIDE { return false; }
    virtual void close() SLANG_OVERRIDE;

    SlangResult init(HANDLE handle);

    ~WinThreadReadStream() { close(); }

protected:
    DWORD _read();

    static DWORD WINAPI _readProc(LPVOID threadParam) { return reinterpret_cast<ThisType*>(threadParam)->_read(); }

    // This would perhaps be better as a queue. We could implement on top of an array. Here we just use a
    // list, and always copy back for simplicity.
    List<Byte> m_buffer;

    WinHandle m_streamHandle;
    WinHandle m_threadHandle;

    bool m_requestThreadTerminate = false;         ///< Set when necessary to indicate to the reading thread it needs to close.

    WinCriticalSection m_criticalSection;
};

/* A simple Stream implementation of a File HANDLE (or Pipe). Note that currently does not allow getPosition/seek/atEnd */
class WinFileStream : public Stream
{
public:
    typedef WinFileStream ThisType;

    // Stream
    virtual Int64 getPosition() SLANG_OVERRIDE { return 0; }
    virtual SlangResult seek(SeekOrigin origin, Int64 offset) SLANG_OVERRIDE { SLANG_UNUSED(origin); SLANG_UNUSED(offset); return SLANG_E_NOT_AVAILABLE; }
    virtual SlangResult read(void* buffer, size_t length, size_t& outReadBytes) SLANG_OVERRIDE;
    virtual SlangResult write(const void* buffer, size_t length) SLANG_OVERRIDE;
    virtual bool isEnd() SLANG_OVERRIDE { return m_streamHandle.isNull(); }
    virtual bool canRead() SLANG_OVERRIDE { return (Index(m_access) & Index(FileAccess::Read)) && !m_streamHandle.isNull(); }
    virtual bool canWrite() SLANG_OVERRIDE { return (Index(m_access) & Index(FileAccess::Write)) && !m_streamHandle.isNull(); }
    virtual void close() SLANG_OVERRIDE;

    WinFileStream(HANDLE handle, FileAccess access);

    ~WinFileStream() { close(); }

protected:

    FileAccess m_access = FileAccess::None;
    WinHandle m_streamHandle;
};

} // namespace Slang

#endif // SLANG_CORE_WIN_STREAM_H
