// unit-test-crash-report.cpp

#include "core/slang-crash-report-file.h"
#include "core/slang-crash-report.h"
#include "core/slang-io.h"
#include "core/slang-string.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

SLANG_UNIT_TEST(crashReportTextNumbers, NoRenderApi)
{
    CrashReportText text;
    text.appendHex(0);
    text.append(" ");
    text.appendHex(0x3a1786);
    text.append(" ");
    text.appendHex(0xffffffffffffffffull);
    text.append(" ");
    text.appendDecimal(0);
    text.append(" ");
    text.appendDecimal(18446744073709551615ull);
    SLANG_CHECK(strcmp(text.getBuffer(), "0 3a1786 ffffffffffffffff 0 18446744073709551615") == 0);
}

SLANG_UNIT_TEST(crashReportTextSingleLine, NoRenderApi)
{
    CrashReportText text;
    text.append(nullptr);
    text.appendSingleLine(nullptr, 8);
    SLANG_CHECK(text.getLength() == 0);

    // A value keeps to one line, so that it cannot forge a `key: value` line of its own, and
    // it stops at the requested length.
    text.appendSingleLine("first\nsignature: forged\r\nrest", 23);
    SLANG_CHECK(strcmp(text.getBuffer(), "first signature: forged") == 0);
}

SLANG_UNIT_TEST(crashReportTextIsBounded, NoRenderApi)
{
    // Text past the capacity is dropped, and the buffer stays terminated.
    CrashReportText text;
    for (int i = 0; i < CrashReportText::kCapacity; ++i)
        text.append("ab");
    SLANG_CHECK(text.getLength() == CrashReportText::kCapacity);
    SLANG_CHECK(strlen(text.getBuffer()) == size_t(CrashReportText::kCapacity));
}

SLANG_UNIT_TEST(crashReportHostName, NoRenderApi)
{
    // The host is named like our release packages, as `os-arch`, with `wasm` as the one
    // exception. A platform that we ship must not fall back to `unknown`.
    const char* host = CrashReportUtil::getHostName();
#if SLANG_WASM
    SLANG_CHECK(strcmp(host, "wasm") == 0);
#else
    SLANG_CHECK(strchr(host, '-') != nullptr);
#endif
#if SLANG_WINDOWS_FAMILY || SLANG_OSX || SLANG_LINUX
    SLANG_CHECK(strstr(host, "unknown") == nullptr);
#endif
}

static String _getSignature(const InternalErrorReportDesc& desc)
{
    CrashReportText text;
    CrashReportUtil::appendInternalErrorSignature(desc, text);
    return String(text.getBuffer());
}

SLANG_UNIT_TEST(crashReportInternalErrorSignature, NoRenderApi)
{
    InternalErrorReportDesc desc;
    desc.signalTypeName = "unexpected";
    desc.sourceFile = "slang-example.cpp";
    desc.sourceLine = 42;
    const String signature = _getSignature(desc);

    // The signature is `ice-` followed by eight hexadecimal digits.
    SLANG_CHECK(signature.getLength() == 12);
    SLANG_CHECK(signature.startsWith("ice-"));

    // The same error gives the same signature whatever the message, the version, or the
    // directory that the build recorded for the source file.
    InternalErrorReportDesc same = desc;
    same.message = "unexpected: a different message";
    same.slangVersion = "2099.1";
    same.sourceFile = "C:\\build\\source\\slang/slang-example.cpp";
    SLANG_CHECK(_getSignature(same) == signature);

    // Another line, another file or another kind of error is another signature.
    InternalErrorReportDesc otherLine = desc;
    otherLine.sourceLine = 43;
    SLANG_CHECK(_getSignature(otherLine) != signature);

    InternalErrorReportDesc otherFile = desc;
    otherFile.sourceFile = "slang-other.cpp";
    SLANG_CHECK(_getSignature(otherFile) != signature);

    InternalErrorReportDesc otherKind = desc;
    otherKind.signalTypeName = "unreachable";
    SLANG_CHECK(_getSignature(otherKind) != signature);
}

SLANG_UNIT_TEST(crashReportInternalErrorFormat, NoRenderApi)
{
    InternalErrorReportDesc desc;
    desc.signalTypeName = "unexpected";
    desc.sourceFile = "source/slang/slang-example.cpp";
    desc.sourceLine = 42;
    desc.slangVersion = "2026.22.1";
    desc.frontEnd = "slangc";
    desc.message = "unexpected: unhandled case for 'MyMaterial'\nsecond line";

    CrashReportText text;
    CrashReportUtil::appendInternalErrorReport(desc, text);

    const String signature = _getSignature(desc);
    StringBuilder expected;
    expected << "==== Slang crash report (format 1) ====\n"
                "Slang stopped because of a bug in Slang, not in your code.\n"
                "This report is plain text. Slang wrote it on this machine and sent it nowhere.\n"
                "\n"
                "[report]\n"
                "kind:        internal-error\n"
                "signal-type: unexpected\n"
                "location:    slang-example.cpp:42\n"
                "phase:       none\n"
                "pass:        none\n"
                "target:      none\n"
                "stage:       none\n"
                "slang:       2026.22.1\n"
                "host:        "
             << CrashReportUtil::getHostName()
             << "\n"
                "front-end:   slangc\n"
                "signature:   "
             << signature
             << "\n"
                "\n"
                "[stack]\n"
                "(not available)\n"
                "\n"
                "[details]\n"
                "This part can hold names from your code. Read it before you share the report.\n"
                "message:     unexpected: unhandled case for 'MyMaterial' second line\n"
                "\n"
                "[how to report]\n"
                "Known bug?   https://github.com/shader-slang/slang/issues?q=%22"
             << signature
             << "%22\n"
                "Report it:   https://github.com/shader-slang/slang/issues/new"
                "?template=crash-report.yml\n";

    SLANG_CHECK(expected.produceString() == String(text.getBuffer()));
}

SLANG_UNIT_TEST(crashReportInternalErrorMissingFacts, NoRenderApi)
{
    // A description with no facts still gives every line of the `[report]` part, so a reader
    // of the report never has to handle a missing key.
    InternalErrorReportDesc desc;
    CrashReportText text;
    CrashReportUtil::appendInternalErrorReport(desc, text);

    const char* report = text.getBuffer();
    SLANG_CHECK(strstr(report, "signal-type: unknown\n") != nullptr);
    SLANG_CHECK(strstr(report, "location:    unknown\n") != nullptr);
    SLANG_CHECK(strstr(report, "slang:       unknown\n") != nullptr);
    SLANG_CHECK(strstr(report, "front-end:   unknown\n") != nullptr);
    SLANG_CHECK(strstr(report, "message:     (none)\n") != nullptr);
}

SLANG_UNIT_TEST(crashReportInternalErrorStack, NoRenderApi)
{
    if (!StackTraceUtil::isSupported())
        return;

    StackTrace trace;
    StackTraceUtil::capture(0, trace);
    SLANG_CHECK_ABORT(trace.frameCount >= 1);

    InternalErrorReportDesc desc;
    desc.stackTrace = &trace;
    CrashReportText text;
    CrashReportUtil::appendInternalErrorReport(desc, text);

    // The `[stack]` part has one numbered line for each captured frame, and the innermost
    // frame, which is this test, is printed as `module+0xoffset`.
    const char* stack = strstr(text.getBuffer(), "\n[stack]\n");
    SLANG_CHECK_ABORT(stack != nullptr);
    const char* details = strstr(stack, "\n[details]\n");
    SLANG_CHECK_ABORT(details != nullptr);

    int frameLineCount = 0;
    for (const char* cursor = stack; cursor < details; ++cursor)
    {
        if (cursor[0] == '\n' && cursor[1] == '#')
            frameLineCount++;
    }
    SLANG_CHECK(frameLineCount == trace.frameCount);

    StackFrameModule module;
    SLANG_CHECK_ABORT(StackTraceUtil::findModule(trace.returnAddresses[0], module));
    StringBuilder firstFrame;
    firstFrame << "\n#0  " << module.moduleName << "+0x";
    SLANG_CHECK(strstr(stack, firstFrame.getBuffer()) != nullptr);
}

SLANG_UNIT_TEST(crashReportFileIsWrittenOnce, NoRenderApi)
{
    // `generateTemporary` gives a name that no other test uses. We remove the file that it
    // creates, so that the name is free.
    String path;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::generateTemporary(toSlice("slang-report-"), path)));
    File::remove(path);

    static const char kFirst[] = "the first report\n";
    static const char kSecond[] = "another report of the same bug\n";

    SLANG_CHECK(
        CrashReportFile::writeNew(path.getBuffer(), kFirst, strlen(kFirst)) ==
        CrashReportFileResult::Written);

    // A second report of the same bug finds the file there and leaves it alone, which is what
    // makes one file per signature safe when many compiles fail at once.
    SLANG_CHECK(
        CrashReportFile::writeNew(path.getBuffer(), kSecond, strlen(kSecond)) ==
        CrashReportFileResult::AlreadyExists);

    String content;
    SLANG_CHECK(SLANG_SUCCEEDED(File::readAllText(path, content)));
    SLANG_CHECK(content == kFirst);
    File::remove(path);

    // A file in a directory that does not exist cannot be created.
    const String missing = Path::combine(path + ".missing", "report.txt");
    SLANG_CHECK(
        CrashReportFile::writeNew(missing.getBuffer(), kFirst, strlen(kFirst)) ==
        CrashReportFileResult::Failed);
}

SLANG_UNIT_TEST(crashReportTemporaryDirectory, NoRenderApi)
{
    // The fallback directory exists, and it is given without a separator at its end, so that
    // a file name joins to it in the same way as to any other directory.
    char directory[512];
    SLANG_CHECK_ABORT(CrashReportFile::getTemporaryDirectory(directory, sizeof(directory)));
    SLANG_CHECK(File::exists(directory));
    const size_t length = strlen(directory);
    SLANG_CHECK_ABORT(length > 0);
    SLANG_CHECK(length == 1 || (directory[length - 1] != '/' && directory[length - 1] != '\\'));

    char tooSmall[2];
    SLANG_CHECK(!CrashReportFile::getTemporaryDirectory(tooSmall, sizeof(tooSmall)));
}

SLANG_UNIT_TEST(crashReportCrashFormat, NoRenderApi)
{
    CrashReportDesc desc;
    desc.signalName = "SIGSEGV";
    desc.hasFaultAddress = true;
    desc.faultAddress = 0x18;
    desc.slangVersion = "2026.22.1";
    desc.frontEnd = "slangc";

    CrashReportText text;
    CrashReportUtil::appendCrashReport(desc, text);
    const char* report = text.getBuffer();

    // A crash report has the parts of an internal-error report, with the signal in place of
    // the kind of error and its location.
    SLANG_CHECK(
        strstr(report, "\n[report]\nkind:        crash\nsignal:      SIGSEGV\n") != nullptr);
    SLANG_CHECK(strstr(report, "\nfault-address: 0x18\n") != nullptr);
    SLANG_CHECK(strstr(report, "\nfront-end:   slangc\n") != nullptr);
    SLANG_CHECK(strstr(report, "\nsignature:   crash-") != nullptr);
    SLANG_CHECK(strstr(report, "\n[stack]\n(not available)\n") != nullptr);
    SLANG_CHECK(strstr(report, "\n[details]\n") != nullptr);
    SLANG_CHECK(strstr(report, "\n[how to report]\n") != nullptr);

    // Without a memory fault there is no fault address.
    CrashReportDesc abortDesc = desc;
    abortDesc.signalName = "SIGABRT";
    abortDesc.hasFaultAddress = false;
    CrashReportText abortText;
    CrashReportUtil::appendCrashReport(abortDesc, abortText);
    SLANG_CHECK(strstr(abortText.getBuffer(), "fault-address") == nullptr);

    // The signal is part of the signature.
    CrashReportText signature;
    CrashReportText abortSignature;
    CrashReportUtil::appendCrashSignature(desc, signature);
    CrashReportUtil::appendCrashSignature(abortDesc, abortSignature);
    SLANG_CHECK(strcmp(signature.getBuffer(), abortSignature.getBuffer()) != 0);
    SLANG_CHECK(strlen(signature.getBuffer()) == strlen("crash-") + 8);
}

SLANG_UNIT_TEST(crashReportBreadcrumbs, NoRenderApi)
{
    // Breadcrumbs print as names, and a breadcrumb whose text is a slice of a longer table
    // entry prints only the slice.
    static const char kTargets[] = "spirv,spirv-asm";
    CrashBreadcrumbs breadcrumbs;
    breadcrumbs.phase = {"emit", 4};
    breadcrumbs.pass = {"legalizeVaryingParams", 21};
    breadcrumbs.target = {kTargets, 5};

    InternalErrorReportDesc desc;
    desc.breadcrumbs = &breadcrumbs;
    CrashReportText text;
    CrashReportUtil::appendInternalErrorReport(desc, text);

    const char* report = text.getBuffer();
    SLANG_CHECK(
        strstr(
            report,
            "\nphase:       emit\npass:        legalizeVaryingParams\ntarget:      spirv\n"
            "stage:       none\n") != nullptr);

    // Breadcrumbs are not part of the signature: the same bug, reached from another pass,
    // is the same bug.
    InternalErrorReportDesc withoutBreadcrumbs = desc;
    withoutBreadcrumbs.breadcrumbs = nullptr;
    CrashReportText signature;
    CrashReportText signatureWithout;
    CrashReportUtil::appendInternalErrorSignature(desc, signature);
    CrashReportUtil::appendInternalErrorSignature(withoutBreadcrumbs, signatureWithout);
    SLANG_CHECK(strcmp(signature.getBuffer(), signatureWithout.getBuffer()) == 0);
}
