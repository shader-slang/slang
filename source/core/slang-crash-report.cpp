// slang-crash-report.cpp
#include "slang-crash-report.h"

#include "slang-signal.h"

#include <atomic>

// This file uses only C library calls and fixed storage. Slang's own containers allocate and
// assert, and both are off limits on the paths that produce a crash report.

namespace Slang
{

void CrashReportText::_appendChar(char c)
{
    if (m_length >= kCapacity)
        return;
    m_chars[m_length++] = c;
    m_chars[m_length] = 0;
}

void CrashReportText::append(const char* text)
{
    if (!text)
        return;
    for (const char* cursor = text; *cursor; ++cursor)
        _appendChar(*cursor);
}

void CrashReportText::appendSingleLine(const char* text, int maxLength)
{
    if (!text)
        return;
    int count = 0;
    for (const char* cursor = text; *cursor && count < maxLength; ++cursor, ++count)
    {
        const char c = *cursor;
        _appendChar((c == '\n' || c == '\r') ? ' ' : c);
    }
}

void CrashReportText::appendHex(uint64_t value)
{
    static const char kDigits[] = "0123456789abcdef";

    // A 64-bit value has at most 16 hexadecimal digits. We produce them from the last one.
    char digits[16];
    int digitCount = 0;
    do
    {
        digits[digitCount++] = kDigits[value & 0xf];
        value >>= 4;
    } while (value);

    while (digitCount > 0)
        _appendChar(digits[--digitCount]);
}

void CrashReportText::appendDecimal(uint64_t value)
{
    // A 64-bit value has at most 20 decimal digits. We produce them from the last one.
    char digits[20];
    int digitCount = 0;
    do
    {
        digits[digitCount++] = char('0' + value % 10);
        value /= 10;
    } while (value);

    while (digitCount > 0)
        _appendChar(digits[--digitCount]);
}

/* static */ const char* CrashReportUtil::getHostName()
{
#if SLANG_WASM
    return "wasm";
#else

#if SLANG_WINDOWS_FAMILY
#define SLANG_CRASH_REPORT_OS "windows"
#elif SLANG_OSX
#define SLANG_CRASH_REPORT_OS "macos"
#elif SLANG_IOS
#define SLANG_CRASH_REPORT_OS "ios"
#elif SLANG_ANDROID
#define SLANG_CRASH_REPORT_OS "android"
#elif SLANG_LINUX
#define SLANG_CRASH_REPORT_OS "linux"
#else
#define SLANG_CRASH_REPORT_OS "unknown"
#endif

// A 64-bit ARM build also sets `SLANG_PROCESSOR_ARM`, so the 64-bit test comes first.
#if SLANG_PROCESSOR_X86_64
#define SLANG_CRASH_REPORT_ARCH "x86_64"
#elif SLANG_PROCESSOR_ARM_64
#define SLANG_CRASH_REPORT_ARCH "aarch64"
#elif SLANG_PROCESSOR_X86
#define SLANG_CRASH_REPORT_ARCH "x86"
#elif SLANG_PROCESSOR_ARM
#define SLANG_CRASH_REPORT_ARCH "arm"
#else
#define SLANG_CRASH_REPORT_ARCH "unknown"
#endif

    return SLANG_CRASH_REPORT_OS "-" SLANG_CRASH_REPORT_ARCH;

#undef SLANG_CRASH_REPORT_OS
#undef SLANG_CRASH_REPORT_ARCH

#endif
}

/// Return the part of `path` that follows its last directory separator.
static const char* _getFileName(const char* path)
{
    const char* name = path;
    for (const char* cursor = path; *cursor; ++cursor)
    {
        if (*cursor == '/' || *cursor == '\\')
            name = cursor + 1;
    }
    return name;
}

/// Fold the characters of `text` into the 32-bit FNV-1a hash `hash`.
static uint32_t _hashText(uint32_t hash, const char* text)
{
    for (const char* cursor = text; *cursor; ++cursor)
    {
        hash ^= uint8_t(*cursor);
        hash *= 16777619u;
    }
    return hash;
}

/// Append `location` of the internal error as `file:line`, or `unknown` if the error carries
/// no source file.
static void _appendLocation(const InternalErrorReportDesc& desc, CrashReportText& ioText)
{
    if (!desc.sourceFile)
    {
        ioText.append("unknown");
        return;
    }
    ioText.append(_getFileName(desc.sourceFile));
    ioText.append(":");
    ioText.appendDecimal(desc.sourceLine > 0 ? uint64_t(desc.sourceLine) : 0);
}

/* static */ void CrashReportUtil::appendInternalErrorSignature(
    const InternalErrorReportDesc& desc,
    CrashReportText& ioText)
{
    // We hash the same `signal-type` and `location` text that the report prints, so a reader
    // can check a signature against those two lines.
    CrashReportText identity;
    identity.append(desc.signalTypeName ? desc.signalTypeName : "unknown");
    identity.append("@");
    _appendLocation(desc, identity);

    const uint32_t hash = _hashText(2166136261u, identity.getBuffer());

    // The signature always has eight digits, so we print the leading zeros that `appendHex`
    // leaves out.
    ioText.append("ice-");
    for (int shift = 28; shift >= 0; shift -= 4)
        ioText.appendHex((hash >> shift) & 0xf);
}

/// Append one `key: value` line of the `[report]` part. A null `value` is printed as `unknown`.
static void _appendField(CrashReportText& ioText, const char* paddedKey, const char* value)
{
    ioText.append(paddedKey);
    ioText.appendSingleLine(value ? value : "unknown", 128);
    ioText.append("\n");
}

/* static */ void CrashReportUtil::appendInternalErrorReport(
    const InternalErrorReportDesc& desc,
    CrashReportText& ioText)
{
    ioText.append("==== Slang crash report (format 1) ====\n"
                  "Slang stopped because of a bug in Slang, not in your code.\n"
                  "This report is plain text. Slang wrote it on this machine and sent it nowhere.\n"
                  "\n"
                  "[report]\n"
                  "kind:        internal-error\n");
    _appendField(ioText, "signal-type: ", desc.signalTypeName);
    ioText.append("location:    ");
    _appendLocation(desc, ioText);
    ioText.append("\n");
    _appendField(ioText, "slang:       ", desc.slangVersion);
    _appendField(ioText, "host:        ", getHostName());
    _appendField(ioText, "front-end:   ", desc.frontEnd);
    ioText.append("signature:   ");
    appendInternalErrorSignature(desc, ioText);
    ioText.append("\n");

    ioText.append("\n[stack]\n");
    if (!desc.stackTrace || desc.stackTrace->frameCount == 0)
    {
        ioText.append("(not available)\n");
    }
    else
    {
        for (int i = 0; i < desc.stackTrace->frameCount; ++i)
        {
            ioText.append("#");
            ioText.appendDecimal(uint64_t(i));
            ioText.append("  ");

            // An address outside every loaded module cannot be resolved later, so we print a
            // placeholder for it and keep the frame numbers contiguous.
            StackFrameModule module;
            if (StackTraceUtil::findModule(desc.stackTrace->returnAddresses[i], module))
            {
                ioText.append(module.moduleName[0] ? module.moduleName : "?");
                ioText.append("+0x");
                ioText.appendHex(uint64_t(module.offset));
            }
            else
            {
                ioText.append("?");
            }
            ioText.append("\n");
        }
    }

    ioText.append("\n[details]\n"
                  "This part can hold names from your code. Read it before you share the report.\n"
                  "message:     ");
    ioText.appendSingleLine(desc.message ? desc.message : "(none)", 2048);
    ioText.append("\n");

    ioText.append("\n[how to report]\n"
                  "Known bug?   https://github.com/shader-slang/slang/issues?q=%22");
    appendInternalErrorSignature(desc, ioText);
    ioText.append("%22\n"
                  "Report it:   https://github.com/shader-slang/slang/issues/new"
                  "?template=crash-report.yml\n");
}

// The process-wide settings of crash reports. The strings are written once at start-up, before
// the flag that publishes them, and the flag is atomic, so a thread that sees reports enabled
// also sees both strings.
static const char* g_frontEnd = nullptr;
static const char* g_slangVersion = nullptr;
static std::atomic<bool> g_areReportsEnabled{false};

/* static */ void CrashReportUtil::enableReports(const char* frontEnd, const char* slangVersion)
{
    if (!frontEnd)
    {
        g_areReportsEnabled.store(false);
        return;
    }
    g_frontEnd = frontEnd;
    g_slangVersion = slangVersion;
    g_areReportsEnabled.store(true);
}

/* static */ bool CrashReportUtil::areReportsEnabled()
{
    return g_areReportsEnabled.load();
}

/* static */ bool CrashReportUtil::appendReportOfLastInternalError(
    const char* message,
    CrashReportText& ioText)
{
    if (!areReportsEnabled())
        return false;

    InternalErrorReportDesc desc;
    desc.slangVersion = g_slangVersion;
    desc.frontEnd = g_frontEnd;
    desc.message = message;

    // An error that did not pass through `handleSignal`, such as an exception from the C++
    // library, leaves no record. Its report still states the version and the host.
    SignalRecord record;
    if (getLastSignalRecord(record))
    {
        desc.signalTypeName = getSignalTypeName(record.type);
        desc.sourceFile = record.file;
        desc.sourceLine = record.line;
        desc.stackTrace = record.stackTrace;
    }

    appendInternalErrorReport(desc, ioText);
    clearLastSignalRecord();
    return true;
}

} // namespace Slang
