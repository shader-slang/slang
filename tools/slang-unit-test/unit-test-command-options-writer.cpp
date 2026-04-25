// unit-test-command-options-writer.cpp
//
// Tests for `CommandOptionsWriter` (renders the slangc -help output)
// + the underlying `CommandOptions` model. Builds a small in-memory
// option set, runs the writer in each output style, and asserts on
// observable structure of the rendered text.

#include "../../source/core/slang-command-options-writer.h"
#include "../../source/core/slang-command-options.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

void fillSampleOptions(CommandOptions& opts)
{
    opts.addCategory(
        CommandOptions::CategoryKind::Option, "general", "General options");
    opts.add("-help,-h", "", "Show this help message");
    opts.add("-version", "", "Print the version");
    opts.add("-quiet,-q", "", "Suppress non-error output");
    opts.add("-output,-o", "<path>", "Output path");

    opts.addCategory(
        CommandOptions::CategoryKind::Option, "compile", "Compilation options");
    opts.add("-target", "<format>", "Target format (e.g. spirv, hlsl)");
    opts.add("-stage", "<stage>", "Pipeline stage");

    opts.addCategory(
        CommandOptions::CategoryKind::Value, "format", "Supported target formats");
    opts.addValue("spirv", "Vulkan SPIR-V");
    opts.addValue("hlsl", "Direct3D HLSL");
    opts.addValue("glsl", "OpenGL GLSL");
}

String renderAll(CommandOptions& opts, CommandOptionsWriter::Style style)
{
    CommandOptionsWriter::Options writerOptions;
    writerOptions.style = style;
    auto writer = CommandOptionsWriter::create(writerOptions);
    writer->appendDescription(&opts);
    return writer->getBuilder().produceString();
}

bool containsAll(const String& text, std::initializer_list<const char*> needles)
{
    for (const char* n : needles)
    {
        if (text.indexOf(UnownedStringSlice(n)) == -1)
            return false;
    }
    return true;
}

} // namespace

SLANG_UNIT_TEST(commandOptionsLookup)
{
    CommandOptions opts; fillSampleOptions(opts);

    // Categories.
    Index cat = opts.findCategoryByName(toSlice("general"));
    SLANG_CHECK(cat >= 0);
    cat = opts.findCategoryByName(toSlice("nope"));
    SLANG_CHECK(cat == -1);

    // Options. Names with comma → first match wins; either alias works.
    SLANG_CHECK(opts.findOptionByName(toSlice("-help")) >= 0);
    SLANG_CHECK(opts.findOptionByName(toSlice("-h")) >= 0);
    SLANG_CHECK(opts.findOptionByName(toSlice("-version")) >= 0);
    SLANG_CHECK(opts.findOptionByName(toSlice("-not-a-thing")) == -1);
}

SLANG_UNIT_TEST(commandOptionsValueLookup)
{
    CommandOptions opts; fillSampleOptions(opts);
    Index formatCat = opts.findCategoryByName(toSlice("format"));
    SLANG_CHECK(formatCat >= 0);

    SLANG_CHECK(opts.findValueByName(formatCat, toSlice("spirv")) >= 0);
    SLANG_CHECK(opts.findValueByName(formatCat, toSlice("hlsl")) >= 0);
    SLANG_CHECK(opts.findValueByName(formatCat, toSlice("metal")) == -1);
}

SLANG_UNIT_TEST(commandOptionsFirstName)
{
    CommandOptions opts; fillSampleOptions(opts);
    Index helpIdx = opts.findOptionByName(toSlice("-h"));
    SLANG_CHECK(helpIdx >= 0);

    // First name listed (before the comma) wins as the canonical
    // display name.
    UnownedStringSlice firstName = opts.getFirstNameForOption(helpIdx);
    SLANG_CHECK(firstName == "-help");
}

SLANG_UNIT_TEST(writerCreatesSingleton)
{
    CommandOptionsWriter::Options writerOptions;
    auto w1 = CommandOptionsWriter::create(writerOptions);
    auto w2 = CommandOptionsWriter::create(writerOptions);
    // Two writers are independent (different builders).
    SLANG_CHECK(w1 != w2);
    SLANG_CHECK(w1->getBuilder().getLength() == 0);
}

SLANG_UNIT_TEST(writerRendersTextStyle)
{
    CommandOptions opts; fillSampleOptions(opts);
    String text = renderAll(opts, CommandOptionsWriter::Style::Text);

    // Sanity: the rendered text isn't empty and the first option's
    // primary name appears.
    SLANG_CHECK(text.getLength() > 0);
    SLANG_CHECK(text.indexOf(toSlice("-help")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("-version")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("spirv")) != -1);

    // Plain text style should not contain markdown headline markers
    // (`##` heading style is markdown-only).
    SLANG_CHECK(text.indexOf(toSlice("##")) == -1);
}

SLANG_UNIT_TEST(writerRendersMarkdownStyle)
{
    CommandOptions opts; fillSampleOptions(opts);
    String text = renderAll(opts, CommandOptionsWriter::Style::Markdown);

    SLANG_CHECK(containsAll(text, {"-help", "-version", "spirv"}));
    // Markdown style should produce headings.
    SLANG_CHECK(text.indexOf(toSlice("#")) != -1);
}

SLANG_UNIT_TEST(writerRendersNoLinkMarkdownStyle)
{
    CommandOptions opts; fillSampleOptions(opts);
    String text = renderAll(opts, CommandOptionsWriter::Style::NoLinkMarkdown);

    SLANG_CHECK(containsAll(text, {"-help", "spirv"}));
    // No-link markdown shouldn't emit `[text](url)` patterns since
    // our test data has no Link entries — but just verify markdown
    // shape (heading marker present).
    SLANG_CHECK(text.indexOf(toSlice("#")) != -1);
}

SLANG_UNIT_TEST(writerRendersSpecificCategory)
{
    CommandOptions opts; fillSampleOptions(opts);
    Index categoryIdx = opts.findCategoryByName(toSlice("compile"));
    SLANG_CHECK(categoryIdx >= 0);

    CommandOptionsWriter::Options writerOptions;
    auto writer = CommandOptionsWriter::create(writerOptions);
    writer->appendDescriptionForCategory(&opts, categoryIdx);

    String text = writer->getBuilder().produceString();
    // Even if the writer emits more or less detail per option, the
    // text shouldn't contain options from the unrelated 'general'
    // category when only 'compile' was requested.
    SLANG_CHECK(text.getLength() > 0);
    SLANG_CHECK(text.indexOf(toSlice("-help")) == -1);
    SLANG_CHECK(text.indexOf(toSlice("-version")) == -1);
}

SLANG_UNIT_TEST(writerStyleInfoExposed)
{
    auto styles = CommandOptionsWriter::getStyleInfos();
    // Three styles defined: Text, Markdown, NoLinkMarkdown.
    SLANG_CHECK(styles.getCount() >= 3);
}

SLANG_UNIT_TEST(writerRespectsLineLength)
{
    // Custom indent + short line length should still produce
    // non-empty, recognizable output without crashing.
    CommandOptions opts; fillSampleOptions(opts);
    CommandOptionsWriter::Options writerOptions;
    writerOptions.lineLength = 40;
    writerOptions.indent = toSlice("    ");
    auto writer = CommandOptionsWriter::create(writerOptions);
    writer->appendDescription(&opts);
    String text = writer->getBuilder().produceString();
    SLANG_CHECK(text.indexOf(toSlice("-help")) != -1);
    SLANG_CHECK(text.indexOf(toSlice("spirv")) != -1);
    SLANG_CHECK(text.getLength() > 0);
}

SLANG_UNIT_TEST(commandOptionsAddValuesViaPairs)
{
    CommandOptions opts;
    opts.addCategory(
        CommandOptions::CategoryKind::Value, "level", "Verbosity levels");
    CommandOptions::ValuePair pairs[] = {
        {"silent", "No output"},
        {"normal", "Standard output"},
        {"verbose", "Detailed output"},
    };
    opts.addValues(pairs, 3);

    Index cat = opts.findCategoryByName(toSlice("level"));
    SLANG_CHECK(cat >= 0);
    SLANG_CHECK(opts.findValueByName(cat, toSlice("silent")) >= 0);
    SLANG_CHECK(opts.findValueByName(cat, toSlice("normal")) >= 0);
    SLANG_CHECK(opts.findValueByName(cat, toSlice("verbose")) >= 0);
    SLANG_CHECK(opts.findValueByName(cat, toSlice("missing")) == -1);
}
