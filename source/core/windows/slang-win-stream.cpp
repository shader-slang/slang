// slang-win-stream.cpp
#include "slang-win-stream.h"

#include <stdio.h>
#include <stdlib.h>

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WinThreadReadStream !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

DWORD WinThreadReadStream::_read()
{
    for (;;)
    {
        WinCriticalSection::Scope scope(&m_criticalSection);
        if (m_streamHandle.isNull())
        {
            // If there is no stream, then we are done
            break;
        }

        const size_t readSize = 1024;

        const Index currentSize = m_buffer.getCount();
        m_buffer.growToCount(currentSize + readSize);

        DWORD bytesRead = 0;
        BOOL readResult = ReadFile(m_streamHandle, m_buffer.getBuffer() + currentSize, readSize, &bytesRead, nullptr);
        m_buffer.setCount(Index(currentSize + bytesRead));

        const DWORD lastError = GetLastError();
        if (lastError == ERROR_BROKEN_PIPE || !readResult)
        {
            break;
        }
        if (m_requestThreadTerminate)
        {
            break;
        }
    }

    // Reset the thread handle
    {
        WinCriticalSection::Scope scope(&m_criticalSection);
        m_streamHandle.setNull();
    }

    return 0;
}

void WinThreadReadStream::close()
{
    if (!m_threadHandle.isNull())
    {
        m_requestThreadTerminate = true;
        // Wait for the thread to complete
        WaitForSingleObject(m_threadHandle, INFINITE);
    }

    // Close any handles. We need to close the stream, because if it's reset it's used to indicate 'closed'
    m_threadHandle.setNull();
    m_streamHandle.setNull();
}

SlangResult WinThreadReadStream::read(void* buffer, size_t length, size_t& outReadBytes)
{
    // If it's closed we are done
    if (m_streamHandle.isNull())
    {
        outReadBytes = 0;
        return SLANG_OK;
    }

    {
        WinCriticalSection::Scope scope(&m_criticalSection);
        const size_t bufferSize = size_t(m_buffer.getCount());

        const size_t readSize = length > bufferSize ? bufferSize : length;

        ::memcpy(buffer, m_buffer.getBuffer(), readSize);
        outReadBytes = readSize;

        if (readSize == bufferSize)
        {
            m_buffer.clear();
        }
        else
        {
            m_buffer.removeRange(0, Index(readSize));
        }
    }
    return SLANG_OK;
}

SlangResult WinThreadReadStream::init(HANDLE handle)
{
    // Set the stream handle
    m_streamHandle = handle;

    // Create a thread to read from the child's stdout.
    m_threadHandle = CreateThread(nullptr, 0, &_readProc, (LPVOID)this, 0, nullptr);
    return SLANG_OK;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

WinFileStream::WinFileStream(HANDLE handle, FileAccess access):
    m_streamHandle(handle),
    m_access(access)
{
}

SlangResult WinFileStream::read(void* buffer, size_t length, size_t& outReadBytes)
{
    if ((Index(m_access) & Index(FileAccess::Read)) == 0 || m_streamHandle.isNull())
    {
        return SLANG_E_NOT_AVAILABLE;
    }

    DWORD bytesRead = 0;
    BOOL readResult = ReadFile(m_streamHandle, buffer, DWORD(length), &bytesRead, nullptr);

    outReadBytes = bytesRead;

    if (!readResult)
    {
        const auto err = GetLastError();

        if (err == ERROR_BROKEN_PIPE)
        {
            m_streamHandle.setNull();
            return SLANG_OK;
        }

        SLANG_UNUSED(err);
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult WinFileStream::write(const void* buffer, size_t length)
{
    if ((Index(m_access) & Index(FileAccess::Write)) == 0 || m_streamHandle.isNull())
    {
        return SLANG_E_NOT_AVAILABLE;
    }

    DWORD numWritten = 0;
    BOOL writeResult = WriteFile(m_streamHandle, buffer, DWORD(length), &numWritten, nullptr);

    if (!writeResult || numWritten != length)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

void WinFileStream::close()
{
    m_streamHandle.setNull();
}

} 
