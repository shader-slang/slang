// slang-crash-report.cpp
#include "slang-crash-report.h"

#include "slang-crash-report-file.h"
#include "slang-signal.h"

#include <atomic>
#include <string.h>

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

/// Append a signature: `prefix` followed by the eight hexadecimal digits of the hash of
/// `identity`. The digits include leading zeros, so every signature has the same length.
static void _appendSignature(const char* prefix, const char* identity, CrashReportText& ioText)
{
    const uint32_t hash = _hashText(2166136261u, identity);
    ioText.append(prefix);
    for (int shift = 28; shift >= 0; shift -= 4)
        ioText.appendHex((hash >> shift) & 0xf);
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

    _appendSignature("ice-", identity.getBuffer(), ioText);
}

/// Append one `key: value` line of the `[report]` part. A null `value` is printed as `unknown`.
static void _appendField(CrashReportText& ioText, const char* paddedKey, const char* value)
{
    ioText.append(paddedKey);
    ioText.appendSingleLine(value ? value : "unknown", 128);
    ioText.append("\n");
}

/// Append the opening lines of a report and the heading of its `[report]` part.
static void _appendReportHeading(CrashReportText& ioText)
{
    ioText.append("==== Slang crash report (format 1) ====\n"
                  "Slang stopped because of a bug in Slang, not in your code.\n"
                  "This report is plain text. Slang wrote it on this machine and sent it nowhere.\n"
                  "\n"
                  "[report]\n");
}

/// Append the `slang`, `host`, `front-end` and `signature` lines that end every `[report]`
/// part, then the `[stack]` part with one line for each frame of `stackTrace`.
static void _appendCommonFieldsAndStack(
    const char* slangVersion,
    const char* frontEnd,
    const char* signature,
    const StackTrace* stackTrace,
    CrashReportText& ioText)
{
    _appendField(ioText, "slang:       ", slangVersion);
    _appendField(ioText, "host:        ", CrashReportUtil::getHostName());
    _appendField(ioText, "front-end:   ", frontEnd);
    _appendField(ioText, "signature:   ", signature);

    ioText.append("\n[stack]\n");
    if (!stackTrace || stackTrace->frameCount == 0)
    {
        ioText.append("(not available)\n");
        return;
    }
    for (int i = 0; i < stackTrace->frameCount; ++i)
    {
        ioText.append("#");
        ioText.appendDecimal(uint64_t(i));
        ioText.append("  ");

        // An address outside every loaded module cannot be resolved later, so we print a
        // placeholder for it and keep the frame numbers contiguous.
        StackFrameModule module;
        if (StackTraceUtil::findModule(stackTrace->returnAddresses[i], module))
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

/// Append the `[details]` part with `message`, and the `[how to report]` part with a search
/// for `signature`.
static void _appendDetailsAndHowToReport(
    const char* message,
    const char* signature,
    CrashReportText& ioText)
{
    ioText.append("\n[details]\n"
                  "This part can hold names from your code. Read it before you share the report.\n"
                  "message:     ");
    ioText.appendSingleLine(message ? message : "(none)", 2048);
    ioText.append("\n");

    ioText.append("\n[how to report]\n"
                  "Known bug?   https://github.com/shader-slang/slang/issues?q=%22");
    ioText.append(signature);
    ioText.append("%22\n"
                  "Report it:   https://github.com/shader-slang/slang/issues/new"
                  "?template=crash-report.yml\n");
}

/* static */ void CrashReportUtil::appendInternalErrorReport(
    const InternalErrorReportDesc& desc,
    CrashReportText& ioText)
{
    CrashReportText signature;
    appendInternalErrorSignature(desc, signature);

    _appendReportHeading(ioText);
    ioText.append("kind:        internal-error\n");
    _appendField(ioText, "signal-type: ", desc.signalTypeName);
    ioText.append("location:    ");
    _appendLocation(desc, ioText);
    ioText.append("\n");
    _appendCommonFieldsAndStack(
        desc.slangVersion,
        desc.frontEnd,
        signature.getBuffer(),
        desc.stackTrace,
        ioText);
    _appendDetailsAndHowToReport(desc.message, signature.getBuffer(), ioText);
}

/// Return true if `moduleName` is a library of the operating system or of the C and C++
/// runtimes, which a crash passes through but is not the cause of.
static bool _isSystemModule(const char* moduleName)
{
    static const char* const kPrefixes[] = {
        "libc.",      "libc-",      "libm.",     "libpthread", "ld-linux",  "libgcc",   "libstdc++",
        "libc++",     "libsystem_", "libdyld",   "dyld",       "ntdll",     "kernel32", "KERNEL32",
        "KERNELBASE", "ucrtbase",   "vcruntime", "msvcp",      "libunwind",
    };
    for (const char* prefix : kPrefixes)
    {
        if (strncmp(moduleName, prefix, strlen(prefix)) == 0)
            return true;
    }
    return false;
}

/* static */ void CrashReportUtil::appendCrashSignature(
    const CrashReportDesc& desc,
    CrashReportText& ioText)
{
    // A crash has no location in Slang's source, so its identity is the signal and the first
    // frame that is not in a system library, as `module+offset`. An abort faults inside the C
    // library, so without that rule every abort on a platform would share one signature. The
    // offset belongs to one build, so the same bug in another release or on another platform
    // has another signature, and the triage side merges those after it has resolved the frames
    // to names.
    CrashReportText identity;
    identity.append(desc.signalName ? desc.signalName : "unknown");
    identity.append("@");
    const int frameCount = desc.stackTrace ? desc.stackTrace->frameCount : 0;
    for (int i = 0; i < frameCount; ++i)
    {
        StackFrameModule module;
        if (!StackTraceUtil::findModule(desc.stackTrace->returnAddresses[i], module) ||
            _isSystemModule(module.moduleName))
        {
            continue;
        }
        identity.append(module.moduleName);
        identity.append("+");
        identity.appendHex(uint64_t(module.offset));
        break;
    }
    _appendSignature("crash-", identity.getBuffer(), ioText);
}

/* static */ void CrashReportUtil::appendCrashReport(
    const CrashReportDesc& desc,
    CrashReportText& ioText)
{
    CrashReportText signature;
    appendCrashSignature(desc, signature);

    _appendReportHeading(ioText);
    ioText.append("kind:        crash\n");
    _appendField(ioText, "signal:      ", desc.signalName);
    if (desc.hasFaultAddress)
    {
        ioText.append("fault-address: 0x");
        ioText.appendHex(uint64_t(desc.faultAddress));
        ioText.append("\n");
    }
    _appendCommonFieldsAndStack(
        desc.slangVersion,
        desc.frontEnd,
        signature.getBuffer(),
        desc.stackTrace,
        ioText);
    _appendDetailsAndHowToReport(nullptr, signature.getBuffer(), ioText);
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

// The directory that reports are saved in. Like the strings above, it is written at start-up,
// before the flag that publishes it.
static char g_reportDirectory[CrashReportUtil::kMaxReportDirectoryLength + 1];
static std::atomic<bool> g_hasReportDirectory{false};

/* static */ void CrashReportUtil::setReportDirectory(const char* directory)
{
    g_hasReportDirectory.store(false);
    if (!directory)
        return;

    const size_t length = strlen(directory);
    if (length == 0 || length > size_t(kMaxReportDirectoryLength))
        return;
    memcpy(g_reportDirectory, directory, length + 1);
    g_hasReportDirectory.store(true);
}

/// The longest path of a saved report. It holds a directory of the greatest length that
/// `setReportDirectory` accepts, and the file name.
static const size_t kMaxReportPathLength = CrashReportUtil::kMaxReportDirectoryLength + 64;

/// Append `text` to the path in `ioPath`, which holds `ioLength` characters. Return false,
/// and leave the path unchanged, if the result does not fit `kMaxReportPathLength`.
static bool _appendToPath(char* ioPath, size_t& ioLength, const char* text)
{
    const size_t textLength = strlen(text);
    if (ioLength + textLength > kMaxReportPathLength)
        return false;
    memcpy(ioPath + ioLength, text, textLength + 1);
    ioLength += textLength;
    return true;
}

/// Save `report` in `directory` under the name that `signature` gives it, and write the path
/// of the file to `outPath`. Return true if the file exists afterwards: either this call wrote
/// it, or an earlier report of the same bug did.
static bool _saveReportIn(
    const char* directory,
    const char* signature,
    const char* report,
    size_t reportLength,
    char* outPath)
{
    size_t pathLength = 0;
    outPath[0] = 0;
    if (!_appendToPath(outPath, pathLength, directory) ||
        !_appendToPath(outPath, pathLength, "/slang-report-") ||
        !_appendToPath(outPath, pathLength, signature) ||
        !_appendToPath(outPath, pathLength, ".txt"))
    {
        return false;
    }
    return CrashReportFile::writeNew(outPath, report, reportLength) !=
           CrashReportFileResult::Failed;
}

/// Save the report whose signature is `signature`, which is the last `reportLength`
/// characters of `ioText`, and append the line that tells the user where it is.
static void _saveReportAndAppendPath(
    const char* signature,
    size_t reportLength,
    CrashReportText& ioText)
{
    const char* report = ioText.getBuffer() + ioText.getLength() - reportLength;

    char path[kMaxReportPathLength + 1];
    bool isSaved = _saveReportIn(g_reportDirectory, signature, report, reportLength, path);
    if (!isSaved)
    {
        // The directory can be read-only, as a source tree on a build machine often is. The
        // directory for temporary files is the one place that we can expect to write.
        char temporaryDirectory[CrashReportUtil::kMaxReportDirectoryLength + 1];
        if (CrashReportFile::getTemporaryDirectory(temporaryDirectory, sizeof(temporaryDirectory)))
        {
            isSaved = _saveReportIn(temporaryDirectory, signature, report, reportLength, path);
        }
    }

    if (isSaved)
    {
        ioText.append("Saved as:    ");
        ioText.append(path);
        ioText.append("\n");
    }
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

    const int lengthBeforeReport = ioText.getLength();
    appendInternalErrorReport(desc, ioText);
    clearLastSignalRecord();

    if (g_hasReportDirectory.load())
    {
        CrashReportText signature;
        appendInternalErrorSignature(desc, signature);
        _saveReportAndAppendPath(
            signature.getBuffer(),
            size_t(ioText.getLength() - lengthBeforeReport),
            ioText);
    }
    return true;
}

/* static */ bool CrashReportUtil::appendCrashReportOfThisProcess(
    const char* signalName,
    bool hasFaultAddress,
    uintptr_t faultAddress,
    const StackTrace& stackTrace,
    CrashReportText& ioText)
{
    if (!areReportsEnabled())
        return false;

    CrashReportDesc desc;
    desc.signalName = signalName;
    desc.hasFaultAddress = hasFaultAddress;
    desc.faultAddress = faultAddress;
    desc.slangVersion = g_slangVersion;
    desc.frontEnd = g_frontEnd;
    desc.stackTrace = &stackTrace;

    const int lengthBeforeReport = ioText.getLength();
    appendCrashReport(desc, ioText);

    if (g_hasReportDirectory.load())
    {
        CrashReportText signature;
        appendCrashSignature(desc, signature);
        _saveReportAndAppendPath(
            signature.getBuffer(),
            size_t(ioText.getLength() - lengthBeforeReport),
            ioText);
    }
    return true;
}

} // namespace Slang
