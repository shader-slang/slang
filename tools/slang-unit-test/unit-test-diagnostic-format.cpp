#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-rich-diagnostics-render.h"
#include "core/slang-string-util.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
// Exercise the public session option at both module loading and target code generation. Target
// diagnostics pass through additional, empty component option sets that must preserve the format.
String getFormatDiagnostics(
    slang::IGlobalSession* globalSession,
    bool targetStage,
    int format,
    bool warningsAsErrors = false,
    bool machineReadable = false)
{
    slang::CompilerOptionEntry options[4] = {};
    int optionCount = 0;
    if (format >= 0)
    {
        auto& option = options[optionCount++];
        option.name = slang::CompilerOptionName::DiagnosticFormat;
        option.value.kind = slang::CompilerOptionValueKind::Int;
        option.value.intValue0 = format;
    }
    if (format == SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO)
    {
        auto& option = options[optionCount++];
        option.name = slang::CompilerOptionName::DiagnosticColor;
        option.value.kind = slang::CompilerOptionValueKind::Int;
        option.value.intValue0 = SLANG_DIAGNOSTIC_COLOR_ALWAYS;
    }
    if (warningsAsErrors)
    {
        auto& option = options[optionCount++];
        option.name = slang::CompilerOptionName::WarningsAsErrors;
        option.value.kind = slang::CompilerOptionValueKind::String;
        option.value.stringValue0 = "all";
    }
    if (machineReadable)
    {
        auto& option = options[optionCount++];
        option.name = slang::CompilerOptionName::EnableMachineReadableDiagnostics;
        option.value.kind = slang::CompilerOptionValueKind::Int;
        option.value.intValue0 = 1;
    }

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.compilerOptionEntries = options;
    sessionDesc.compilerOptionEntryCount = optionCount;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    const char* source =
        targetStage ? "int a;\n[shader(\"compute\")]\n[numthreads(1,1,1)]\nvoid main() {}\n"
                    : "int value = unknownName;\n";
    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "formatTest",
        "format-test.slang",
        source,
        diagnostics.writeRef());
    if (targetStage)
    {
        SLANG_CHECK_ABORT(module != nullptr);
        ComPtr<slang::IComponentType> linkedProgram;
        ComPtr<slang::IBlob> linkDiagnostics;
        module->link(linkedProgram.writeRef(), linkDiagnostics.writeRef());
        SLANG_CHECK_ABORT(linkedProgram != nullptr);
        ComPtr<slang::IBlob> code;
        diagnostics.setNull();
        linkedProgram->getTargetCode(0, code.writeRef(), diagnostics.writeRef());
    }
    else
    {
        SLANG_CHECK(module == nullptr);
    }
    SLANG_CHECK_ABORT(diagnostics && diagnostics->getBufferSize() > 0);
    return String(UnownedStringSlice(
        (const char*)diagnostics->getBufferPointer(),
        diagnostics->getBufferSize()));
}
} // namespace

SLANG_UNIT_TEST(diagnosticFormatFrontEnd)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    const String implicitDefault = getFormatDiagnostics(globalSession, false, -1);
    const String explicitDefault =
        getFormatDiagnostics(globalSession, false, SLANG_DIAGNOSTIC_FORMAT_DEFAULT);
    SLANG_CHECK(implicitDefault == explicitDefault);
    SLANG_CHECK(implicitDefault.indexOf(toSlice("error[E30015]:")) != -1);

    const String visualStudio =
        getFormatDiagnostics(globalSession, false, SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO);
    SLANG_CHECK(visualStudio.indexOf(toSlice("format-test.slang(1,13): error E30015:")) != -1);
    SLANG_CHECK(visualStudio.indexOf(toSlice("unknownName")) != -1);
    SLANG_CHECK(visualStudio.indexOf(toSlice("\x1B[")) == -1);

    const String machineReadable = getFormatDiagnostics(
        globalSession,
        false,
        SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO,
        false,
        true);
    SLANG_CHECK(machineReadable.indexOf(toSlice("E30015\terror\t")) != -1);
    SLANG_CHECK(machineReadable.indexOf(toSlice(": error E30015:")) == -1);
    SLANG_CHECK(machineReadable.indexOf(toSlice("\x1B[")) == -1);
}

SLANG_UNIT_TEST(diagnosticFormatTargetStage)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // A global uniform produces E39019 during parameter binding in getTargetCode().
    const String visualStudio =
        getFormatDiagnostics(globalSession, true, SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO);
    SLANG_CHECK(visualStudio.indexOf(toSlice("format-test.slang(1,5): warning E39019:")) != -1);
    SLANG_CHECK(visualStudio.indexOf(toSlice("\x1B[")) == -1);

    const String promoted =
        getFormatDiagnostics(globalSession, true, SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO, true);
    SLANG_CHECK(promoted.indexOf(toSlice("format-test.slang(1,5): error E39019:")) != -1);
    SLANG_CHECK(promoted.indexOf(toSlice(": warning E39019:")) == -1);

    const String defaultFormat = getFormatDiagnostics(globalSession, true, -1);
    SLANG_CHECK(defaultFormat.indexOf(toSlice("warning[E39019]:")) != -1);
}

SLANG_UNIT_TEST(diagnosticFormatLegacySink)
{
    DiagnosticSink parent(nullptr, nullptr);
    parent.setDiagnosticFormat(SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO);
    DiagnosticSink sink(nullptr, nullptr, &parent);
    SLANG_CHECK(sink.getDiagnosticFormat() == SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO);
    sink.setFlag(DiagnosticSink::Flag::TreatWarningsAsErrors);
    const DiagnosticInfo
        info{7, Severity::Warning, "format-test", "first line\nsecond line", WarningLevel::Default};
    SLANG_CHECK(sink.diagnose(SourceLoc(), info));
    const String text = sink.outputBuffer.produceString();
    SLANG_CHECK(text == "error E00007: first line\n    second line\n");
    SLANG_CHECK(sink.getErrorCount() == 1);
}


SLANG_UNIT_TEST(diagnosticFormatStructuredSpans)
{
    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);
    const String source = "#line 40 \"mapped path.slang\"\nint first;\nint second;\n";
    SourceFile* file = sourceManager.createSourceFileWithString(
        PathInfo::makeFromString("original.slang"),
        source);
    SourceView* view = sourceManager.createSourceView(file, nullptr, SourceLoc());
    const SourceLoc start = view->getRange().begin;
    // This is the mapping installed by the preprocessor for the directive in the source above.
    view->addLineDirective(start, String("mapped path.slang"), 40);
    const SourceLoc first = start + source.indexOf(toSlice("first"));
    const SourceLoc second = start + source.indexOf(toSlice("second"));

    GenericDiagnostic diagnostic = {};
    diagnostic.code = 42;
    diagnostic.severity = Severity::Error;
    diagnostic.message = "main message\nmessage continuation";
    diagnostic.primarySpan = {SourceRange(first, first + 5), "primary label\nprimary continuation"};
    diagnostic.secondarySpans.add(
        {SourceRange(second, second + 6), "secondary label\nsecondary continuation"});
    DiagnosticNote note;
    note.message = "related declaration\nnote continuation";
    note.span = {SourceRange(second, second + 6), "note primary label"};
    note.secondarySpans.add({SourceRange(first, first + 5), "note secondary label"});
    diagnostic.notes.add(note);

    const DiagnosticRenderOptions options = {
        .enableTerminalColors = true,
        .enableUnicode = true,
        .format = SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO};
    const String text = renderDiagnostic(nullptr, &sourceManager, options, diagnostic);
    SLANG_CHECK(
        text.indexOf(toSlice("mapped path.slang(40,5): error E00042: main message\n")) == 0);
    SLANG_CHECK(
        text.indexOf(toSlice("mapped path.slang(41,5): note: related declaration\n")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("original.slang")) == -1);
    SLANG_CHECK(text.indexOf(toSlice("40 | int first;")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("41 | int second;")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("primary label")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("secondary label")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("note primary label")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("note secondary label")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("\n    message continuation\n")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("\n    primary continuation\n")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("\n    secondary continuation\n")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("\n    note continuation\n")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("\x1B[")) == -1);

    // Only actual diagnostic headers may begin in column one. Multiline labels and source
    // snippets must remain continuation lines, even when the caller requests Unicode and color.
    int headerCount = 0;
    for (auto line : LineParser(text.getUnownedSlice()))
    {
        if (!line.getLength())
            continue;
        if (line.startsWith(toSlice("mapped path.slang(")))
            headerCount++;
        else
            SLANG_CHECK(line.startsWith(toSlice("    ")));
        for (char c : line)
            SLANG_CHECK(static_cast<unsigned char>(c) < 128);
    }
    SLANG_CHECK(headerCount == 2);
}

SLANG_UNIT_TEST(diagnosticFormatWithoutSource)
{
    GenericDiagnostic diagnostic = {};
    diagnostic.code = -1;
    diagnostic.severity = Severity::Note;
    diagnostic.message = "unlocated note";
    diagnostic.primarySpan.message = "label without source";
    const String text = renderDiagnostic(
        nullptr,
        nullptr,
        {.format = SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO},
        diagnostic);
    SLANG_CHECK(text == "note: unlocated note\n    label without source\n");
}

SLANG_UNIT_TEST(diagnosticFormatEmptyMessages)
{
    GenericDiagnostic diagnostic = {};
    diagnostic.code = 42;
    diagnostic.severity = Severity::Error;
    diagnostic.primarySpan.message = "primary label";
    DiagnosticNote note;
    note.span.message = "note label";
    diagnostic.notes.add(note);
    note.message = "following note";
    note.span.message = String();
    diagnostic.notes.add(note);

    const String text = renderDiagnostic(
        nullptr,
        nullptr,
        {.format = SLANG_DIAGNOSTIC_FORMAT_VISUAL_STUDIO},
        diagnostic);
    SLANG_CHECK(
        text ==
        "error E00042: \n    primary label\nnote: \n    note label\nnote: following note\n");
}
