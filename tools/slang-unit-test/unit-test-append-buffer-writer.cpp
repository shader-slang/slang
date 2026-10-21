// unit-test-append-buffer-writer.cpp
//
// Tests for the `beginAppendBuffer` / `endAppendBuffer` path of the writers in
// `source/core/slang-writer.h`.
//
// `ISlangWriter` (include/slang.h) lets a caller skip `endAppendBuffer` when it has nothing to
// write, and a writer may be shared between threads (for example the process-wide stdout writer
// handed to every request without its own writer). The tests below check that neither of these,
// nor a `write()` that appends to its own writer, can leave a writer unusable.
//
// A regression in the skipped-end and re-entrant cases shows up as a deadlock, so those bodies
// run on another thread with a timed wait. On a timeout the test fails and the thread is
// detached, instead of hanging the run.

#include "core/slang-io.h"
#include "core/slang-writer.h"
#include "unit-test/slang-unit-test.h"

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <string.h>
#include <thread>

using namespace Slang;

namespace
{

/// An `AppendBufferWriter` that records the text of every `write()` call as one string. The
/// records are guarded by a lock that is held only inside `write()`, so one writer can be shared
/// between threads and each record can be checked for corruption afterwards.
class RecordingWriter : public AppendBufferWriter
{
public:
    RecordingWriter()
        : AppendBufferWriter(0)
    {
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL write(const char* chars, size_t numChars) SLANG_OVERRIDE
    {
        std::lock_guard<std::mutex> lock(m_recordsMutex);
        m_records.add(String(chars, chars + numChars));
        return SLANG_OK;
    }

    List<String> getRecords()
    {
        std::lock_guard<std::mutex> lock(m_recordsMutex);
        return m_records;
    }

private:
    std::mutex m_recordsMutex;
    List<String> m_records;
};

/// A `RecordingWriter` whose first `write()` prints a note to the same writer before recording
/// its own text, the way a writer that logs its own activity would. The note goes through
/// `beginAppendBuffer` / `endAppendBuffer` while the outer `endAppendBuffer` is still running.
class ReentrantWriter : public RecordingWriter
{
public:
    SLANG_NO_THROW SlangResult SLANG_MCALL write(const char* chars, size_t numChars) SLANG_OVERRIDE
    {
        if (!m_hasPrintedNote)
        {
            m_hasPrintedNote = true;
            SLANG_RETURN_ON_FAIL(WriterHelper(this).print("inner note %d", 1));
        }
        return RecordingWriter::write(chars, numChars);
    }

private:
    bool m_hasPrintedNote = false;
};

/// Runs `body` on another thread and waits up to 10 seconds for it. Returns true and sets
/// `outResult` if `body` finished. Otherwise the thread is detached and left blocked until the
/// process exits, so `body` must own every object it touches.
template<typename Func, typename Result>
bool runWithTimeout(Func body, Result& outResult)
{
    auto task = std::make_shared<std::packaged_task<Result()>>(std::move(body));
    std::future<Result> result = task->get_future();
    std::thread thread([task]() { (*task)(); });
    if (result.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
    {
        thread.detach();
        return false;
    }
    thread.join();
    outResult = result.get();
    return true;
}

/// Begins an append buffer and abandons it without writing anything, as `ISlangWriter` allows
/// ("an endAppendBuffer is not necessary if there are no characters to write"), then appends
/// `text` with a complete begin/end pair.
SlangResult appendAfterSkippedEnd(ISlangWriter* writer, const char* text)
{
    const size_t length = ::strlen(text);
    writer->beginAppendBuffer(length);

    char* buffer = writer->beginAppendBuffer(length);
    ::memcpy(buffer, text, length);
    return writer->endAppendBuffer(buffer, length);
}

void recordMessage(const char* message, void* userData)
{
    static_cast<List<String>*>(userData)->add(String(message));
}

} // namespace

SLANG_UNIT_TEST(appendBufferWriterCallbackWriter)
{
    List<String> messages;
    ComPtr<ISlangWriter> writer(new CallbackWriter(&recordMessage, &messages, 0));

    SLANG_CHECK(SLANG_SUCCEEDED(WriterHelper(writer).print("first %d\n", 1)));
    SLANG_CHECK(SLANG_SUCCEEDED(WriterHelper(writer).print("second %s\n", "line")));
    SLANG_CHECK(SLANG_SUCCEEDED(writer->write("third", 5)));

    char* buffer = writer->beginAppendBuffer(6);
    ::memcpy(buffer, "fourth", 6);
    SLANG_CHECK(SLANG_SUCCEEDED(writer->endAppendBuffer(buffer, 6)));

    SLANG_CHECK_ABORT(messages.getCount() == 4);
    SLANG_CHECK(messages[0] == "first 1\n");
    SLANG_CHECK(messages[1] == "second line\n");
    SLANG_CHECK(messages[2] == "third");
    SLANG_CHECK(messages[3] == "fourth");
}

SLANG_UNIT_TEST(appendBufferWriterSkippedEndNullWriter)
{
    SlangResult result = SLANG_FAIL;
    const bool finished = runWithTimeout(
        []()
        {
            ComPtr<ISlangWriter> writer(new NullWriter(0));
            return appendAfterSkippedEnd(writer, "hello");
        },
        result);

    SLANG_CHECK_ABORT(finished);
    SLANG_CHECK(SLANG_SUCCEEDED(result));
}

SLANG_UNIT_TEST(appendBufferWriterSkippedEndFileWriter)
{
    String path;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-append-buffer-writer"), path)));

    // The writer is created and released inside the body, so the file is closed by the time the
    // result is ready and the test thread can read it back.
    SlangResult result = SLANG_FAIL;
    const bool finished = runWithTimeout(
        [path]()
        {
            ComPtr<ISlangWriter> writer;
            SLANG_RETURN_ON_FAIL(FileWriter::createBinary(path.getBuffer(), 0, writer));
            return appendAfterSkippedEnd(writer, "hello");
        },
        result);

    String contents;
    const SlangResult readResult = finished ? File::readAllText(path, contents) : SLANG_FAIL;
    File::remove(path);

    SLANG_CHECK_ABORT(finished);
    SLANG_CHECK(SLANG_SUCCEEDED(result));
    SLANG_CHECK(SLANG_SUCCEEDED(readResult));
    SLANG_CHECK(contents == "hello");
}

SLANG_UNIT_TEST(appendBufferWriterReentrantWrite)
{
    List<String> records;
    const bool finished = runWithTimeout(
        []()
        {
            RefPtr<ReentrantWriter> writer = new ReentrantWriter();
            const char text[] = "outer text, longer than the inner note";
            const size_t length = sizeof(text) - 1;

            char* buffer = writer->beginAppendBuffer(length);
            ::memcpy(buffer, text, length);
            writer->endAppendBuffer(buffer, length);
            return writer->getRecords();
        },
        records);

    SLANG_CHECK_ABORT(finished);

    // The outer text is recorded after the nested note was appended, so it is intact only if the
    // nested append did not reuse the buffer that was being written.
    SLANG_CHECK_ABORT(records.getCount() == 2);
    SLANG_CHECK(records[0] == "inner note 1");
    SLANG_CHECK(records[1] == "outer text, longer than the inner note");
}

SLANG_UNIT_TEST(appendBufferWriterInterleavedWriters)
{
    RefPtr<RecordingWriter> first = new RecordingWriter();
    RefPtr<RecordingWriter> second = new RecordingWriter();

    // Each writer allows one active append buffer, so one thread may have a buffer open on each
    // of two writers at once. The second buffer is larger, so sharing storage would reallocate
    // the first.
    char* firstBuffer = first->beginAppendBuffer(5);
    char* secondBuffer = second->beginAppendBuffer(32);
    ::memcpy(firstBuffer, "first", 5);
    ::memcpy(secondBuffer, "the second writer's longer text", 31);
    second->endAppendBuffer(secondBuffer, 31);
    first->endAppendBuffer(firstBuffer, 5);

    List<String> firstRecords = first->getRecords();
    List<String> secondRecords = second->getRecords();
    SLANG_CHECK_ABORT(firstRecords.getCount() == 1 && secondRecords.getCount() == 1);
    SLANG_CHECK(firstRecords[0] == "first");
    SLANG_CHECK(secondRecords[0] == "the second writer's longer text");
}

SLANG_UNIT_TEST(appendBufferWriterSharedBetweenThreads)
{
    const int kThreadCount = 8;
    const int kAppendsPerThread = 200;

    RefPtr<RecordingWriter> writer = new RecordingWriter();
    std::atomic<bool> go{false};
    std::thread threads[kThreadCount];
    for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex)
    {
        threads[threadIndex] = std::thread(
            [&, threadIndex]()
            {
                while (!go.load())
                    std::this_thread::yield();

                for (int appendIndex = 0; appendIndex < kAppendsPerThread; ++appendIndex)
                {
                    char text[64];
                    const size_t length = size_t(::snprintf(
                        text,
                        sizeof(text),
                        "thread %d append %d",
                        threadIndex,
                        appendIndex));

                    // Filling the buffer in two halves with a yield between them gives the other
                    // threads a chance to begin and end their own appends meanwhile.
                    char* buffer = writer->beginAppendBuffer(length);
                    ::memcpy(buffer, text, length / 2);
                    std::this_thread::yield();
                    ::memcpy(buffer + length / 2, text + length / 2, length - length / 2);
                    writer->endAppendBuffer(buffer, length);
                }
            });
    }
    go = true;
    for (auto& thread : threads)
        thread.join();

    List<String> records = writer->getRecords();
    SLANG_CHECK_ABORT(records.getCount() == kThreadCount * kAppendsPerThread);

    List<String> expected;
    for (int threadIndex = 0; threadIndex < kThreadCount; ++threadIndex)
    {
        for (int appendIndex = 0; appendIndex < kAppendsPerThread; ++appendIndex)
        {
            char text[64];
            ::snprintf(text, sizeof(text), "thread %d append %d", threadIndex, appendIndex);
            expected.add(String(text));
        }
    }
    records.sort();
    expected.sort();

    Index mismatchCount = 0;
    for (Index i = 0; i < records.getCount(); ++i)
    {
        if (!(records[i] == expected[i]))
            ++mismatchCount;
    }
    SLANG_CHECK(mismatchCount == 0);
}
