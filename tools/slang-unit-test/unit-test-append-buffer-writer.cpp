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
// Each test protects one part of the design:
//   * CallbackWriter: a CallbackWriter delivers append buffers to its callback.
//   * SkippedEnd*, SkippedEndOnOtherThread: an abandoned append buffer blocks no later append,
//     on the same thread or another one.
//   * ReentrantWrite: a write() that appends to its own writer gets a separate buffer.
//   * InterleavedWriters: one thread can have a buffer open on several writers at once.
//   * SharedBetweenThreads: threads sharing one writer fill separate buffers.
//   * EndWithoutBegin: an end with no begin on the calling thread fails without writing.
//   * PrintFromThreadExit: a writer is still usable from a destructor run at thread exit.
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
/// records are guarded by a lock that is held only while they are accessed, never between
/// `beginAppendBuffer` and `endAppendBuffer`, so one writer can be shared between threads and
/// each record can be checked for corruption afterwards.
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

String makeSharedAppendText(int threadIndex, int appendIndex)
{
    char text[64];
    ::snprintf(text, sizeof(text), "thread %d append %d", threadIndex, appendIndex);
    return String(text);
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

SLANG_UNIT_TEST(appendBufferWriterSkippedEndOnOtherThread)
{
    // The other thread abandons its append buffer and exits. A lock held from begin to end would
    // stay locked for every thread after that.
    List<String> records;
    const bool finished = runWithTimeout(
        []()
        {
            RefPtr<RecordingWriter> writer = new RecordingWriter();
            std::thread([&]() { writer->beginAppendBuffer(5); }).join();

            char* buffer = writer->beginAppendBuffer(5);
            ::memcpy(buffer, "hello", 5);
            writer->endAppendBuffer(buffer, 5);
            return writer->getRecords();
        },
        records);

    SLANG_CHECK_ABORT(finished);
    SLANG_CHECK_ABORT(records.getCount() == 1);
    SLANG_CHECK(records[0] == "hello");
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
    const char* const texts[] = {"first", "the second writer's longer text", "third"};
    const Index kWriterCount = SLANG_COUNT_OF(texts);

    // Each writer allows one active append buffer, so one thread may have a buffer open on each
    // of several writers at once. Later buffers are larger, so sharing storage would reallocate
    // an earlier one. Ending them in the order they were begun makes each removal move a live
    // entry into the removed one's place.
    List<RefPtr<RecordingWriter>> writers;
    List<char*> buffers;
    for (Index i = 0; i < kWriterCount; ++i)
    {
        writers.add(new RecordingWriter());
        buffers.add(writers[i]->beginAppendBuffer(::strlen(texts[i])));
    }
    for (Index i = 0; i < kWriterCount; ++i)
        ::memcpy(buffers[i], texts[i], ::strlen(texts[i]));
    for (Index i = 0; i < kWriterCount; ++i)
        SLANG_CHECK(SLANG_SUCCEEDED(writers[i]->endAppendBuffer(buffers[i], ::strlen(texts[i]))));

    for (Index i = 0; i < kWriterCount; ++i)
    {
        List<String> records = writers[i]->getRecords();
        SLANG_CHECK_ABORT(records.getCount() == 1);
        SLANG_CHECK(records[0] == texts[i]);
    }
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
                    const String text = makeSharedAppendText(threadIndex, appendIndex);
                    const char* chars = text.getBuffer();
                    const size_t length = size_t(text.getLength());

                    // Filling the buffer in two halves with a yield between them gives the other
                    // threads a chance to begin and end their own appends meanwhile.
                    char* buffer = writer->beginAppendBuffer(length);
                    ::memcpy(buffer, chars, length / 2);
                    std::this_thread::yield();
                    ::memcpy(buffer + length / 2, chars + length / 2, length - length / 2);
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
            expected.add(makeSharedAppendText(threadIndex, appendIndex));
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

SLANG_UNIT_TEST(appendBufferWriterEndWithoutBegin)
{
    RefPtr<RecordingWriter> writer = new RecordingWriter();

    char text[] = "abcd";
    SLANG_CHECK(writer->endAppendBuffer(text, 4) == SLANG_E_INVALID_ARG);

    // An append buffer belongs to the thread that began it, so ending it on another thread fails
    // and leaves it open for the thread that owns it.
    char* buffer = writer->beginAppendBuffer(4);
    ::memcpy(buffer, "abcd", 4);
    SlangResult otherThreadResult = SLANG_OK;
    std::thread([&]() { otherThreadResult = writer->endAppendBuffer(buffer, 4); }).join();
    SLANG_CHECK(otherThreadResult == SLANG_E_INVALID_ARG);
    SLANG_CHECK(SLANG_SUCCEEDED(writer->endAppendBuffer(buffer, 4)));

    List<String> records = writer->getRecords();
    SLANG_CHECK_ABORT(records.getCount() == 1);
    SLANG_CHECK(records[0] == "abcd");
}

namespace
{
/// Prints to `writer` from its destructor. As a thread_local constructed before the thread's
/// first append buffer, it is destroyed after the writers' per-thread state at thread exit.
struct PrintsOnThreadExit
{
    RecordingWriter* writer = nullptr;
    ~PrintsOnThreadExit()
    {
        if (writer)
            WriterHelper(writer).print("thread exit %d", 1);
    }
};
} // namespace

SLANG_UNIT_TEST(appendBufferWriterPrintFromThreadExit)
{
    RefPtr<RecordingWriter> writer = new RecordingWriter();
    std::thread thread(
        [&]()
        {
            thread_local PrintsOnThreadExit printsOnExit;
            printsOnExit.writer = writer;
            WriterHelper(writer).print("in thread %d", 1);
        });
    thread.join();

    List<String> records = writer->getRecords();
    SLANG_CHECK_ABORT(records.getCount() == 2);
    SLANG_CHECK(records[0] == "in thread 1");
    SLANG_CHECK(records[1] == "thread exit 1");
}
