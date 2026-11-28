// slang-crash-report.h
#ifndef SLANG_CORE_CRASH_REPORT_H
#define SLANG_CORE_CRASH_REPORT_H

#include "slang-common.h"
#include "slang-stack-trace.h"

#include <stdint.h>

namespace Slang
{

/// Text of bounded size that is built without the heap.
///
/// A crash report is produced after the compiler has failed, and the same code will run inside
/// a fatal-signal handler. We therefore build the text in a fixed array: the heap may be the
/// thing that is corrupted, and a signal handler must not allocate. Text that does not fit is
/// dropped, because an assert on this path would start another report.
struct CrashReportText
{
    static const int kCapacity = 16 * 1024;

    CrashReportText() { m_chars[0] = 0; }

    /// Append `text`. A null `text` appends nothing.
    void append(const char* text);

    /// Append at most `maxLength` characters of `text`, with every line break replaced by a
    /// space. The value of a `key: value` line uses this, so that a value cannot start a line
    /// that a reader would take for another key.
    void appendSingleLine(const char* text, int maxLength);

    /// Append `value` in lowercase hexadecimal, without a prefix and without leading zeros.
    void appendHex(uint64_t value);

    /// Append `value` in decimal.
    void appendDecimal(uint64_t value);

    /// The text so far. It is always terminated by a NUL.
    const char* getBuffer() const { return m_chars; }
    int getLength() const { return m_length; }

private:
    void _appendChar(char c);

    char m_chars[kCapacity + 1];
    int m_length = 0;
};

/// The facts that the report of an internal compiler error states.
///
/// Every string is borrowed for the duration of the call that takes the description. A null
/// string in the `[report]` part is printed as `unknown`, so that the part always has the same
/// lines.
struct InternalErrorReportDesc
{
    /// The kind of internal error, as one of the fixed names `unexpected`, `unimplemented`,
    /// `unreachable`, `assert`, `invalid-operation` and `abort-compilation`.
    const char* signalTypeName = nullptr;

    /// The Slang source file and line that raised the error. Any directory in `sourceFile` is
    /// left out of the report.
    const char* sourceFile = nullptr;
    int sourceLine = 0;

    /// The version of Slang, and the program that ran it, for example `slangc`.
    const char* slangVersion = nullptr;
    const char* frontEnd = nullptr;

    /// The call stack at the point that raised the error, or null if none was captured.
    const StackTrace* stackTrace = nullptr;

    /// The message of the error. It can contain names from the user's code, so the report
    /// prints it in the `[details]` part, which the user is told to read before sharing.
    const char* message = nullptr;
};

struct CrashReportUtil
{
    /// Return the platform that this binary was built for, named like our release packages:
    /// for example `linux-x86_64`, `macos-aarch64` or `windows-x86_64`.
    static const char* getHostName();

    /// Append the signature of the internal error that `desc` describes, such as `ice-51b0c3aa`.
    ///
    /// The signature identifies one bug: two reports with the same kind of error raised at the
    /// same place in Slang's source have the same signature, on every platform, so reports can
    /// be counted and duplicates merged. It does not depend on the message or on the stack.
    static void appendInternalErrorSignature(
        const InternalErrorReportDesc& desc,
        CrashReportText& ioText);

    /// Append the complete report of the internal error that `desc` describes.
    ///
    /// The report has a `[report]` part that holds only values from Slang's own vocabulary, a
    /// `[stack]` part that holds return addresses as `module+0xoffset`, a `[details]` part for
    /// text that can come from the user's code, and a `[how to report]` part.
    static void appendInternalErrorReport(
        const InternalErrorReportDesc& desc,
        CrashReportText& ioText);
};

} // namespace Slang

#endif
