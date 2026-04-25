// unit-test-command-options-writer.cpp
//
// Contract under test
// -------------------
// `CommandOptions` is the data model for `slangc`'s help-text catalog
// (categories, options, value-enums). `CommandOptionsWriter` renders
// that data model in three styles (Text, Markdown, NoLinkMarkdown).
//
// The catalog data model contract:
//   * `findOptionByName` / `findCategoryByName` / `findValueByName`
//     resolve names to indices; aliases listed in a comma-separated
//     name string all resolve to the same option.
//   * `getFirstNameForOption` returns the canonical alias (the one
//     listed first in the comma-separated name string).
//
// The writer rendering contract:
//   * TEXT — every category appears as a heading underlined with `=`.
//     Every option's primary alias appears (or, if the option has a
//     non-empty `usage` field, the usage replaces the names — used by
//     options like `-output <path>` whose usage is more informative).
//     Every option description appears on the same line, separated
//     from name/usage by `: `.
//   * MARKDOWN — adds a "Quick Links" section at the top with anchor
//     hrefs to each category, anchors before each category heading,
//     `## category` headings, `### option-name` subheadings.
//   * NO-LINK-MD — like MARKDOWN minus the Quick Links section and the
//     per-heading `<a id>` anchors.

#include "../../source/core/slang-command-options-writer.h"
#include "../../source/core/slang-command-options.h"
#include "../../source/core/slang-name-value.h"
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
    opts.add("-output,-o", "<path>", "Output path");

    opts.addCategory(
        CommandOptions::CategoryKind::Option, "compile", "Compilation options");
    opts.add("-target", "<format>", "Target format");

    opts.addCategory(
        CommandOptions::CategoryKind::Value, "format", "Supported formats");
    opts.addValue("spirv", "Vulkan SPIR-V");
    opts.addValue("hlsl", "Direct3D HLSL");
}

bool contains(const String& s, const char* needle)
{
    return s.indexOf(UnownedStringSlice(needle)) != -1;
}

} // namespace

// ---------------------------------------------------------------
// CommandOptions data model
// ---------------------------------------------------------------

// Aliases listed comma-separated must all resolve to the SAME index;
// callers (parsers) rely on this so users can write either alias
// interchangeably. The first alias is the canonical one.
SLANG_UNIT_TEST(commandOptionsAliasesShareIndex)
{
    CommandOptions opts; fillSampleOptions(opts);
    Index helpByLong = opts.findOptionByName(toSlice("-help"));
    Index helpByShort = opts.findOptionByName(toSlice("-h"));
    SLANG_CHECK(helpByLong >= 0);
    SLANG_CHECK(helpByLong == helpByShort);

    Index outByLong = opts.findOptionByName(toSlice("-output"));
    Index outByShort = opts.findOptionByName(toSlice("-o"));
    SLANG_CHECK(outByLong >= 0);
    SLANG_CHECK(outByLong == outByShort);

    // Canonical name = first listed alias.
    SLANG_CHECK(opts.getFirstNameForOption(helpByShort) == "-help");
    SLANG_CHECK(opts.getFirstNameForOption(outByShort) == "-output");

    // Different options must resolve to different indices.
    SLANG_CHECK(helpByLong != outByLong);
}

// Unknown names return -1 (used as the "not found" sentinel by
// argv parsers). Both option and category lookups share this rule.
SLANG_UNIT_TEST(commandOptionsUnknownNameReturnsNegativeOne)
{
    CommandOptions opts; fillSampleOptions(opts);
    SLANG_CHECK(opts.findOptionByName(toSlice("-not-a-thing")) == -1);
    SLANG_CHECK(opts.findCategoryByName(toSlice("nonexistent")) == -1);

    Index formatCat = opts.findCategoryByName(toSlice("format"));
    SLANG_CHECK(formatCat >= 0);
    SLANG_CHECK(opts.findValueByName(formatCat, toSlice("metal")) == -1);
}

// Values are scoped to their category — same value name across
// different categories doesn't conflict (and -1 for "wrong category").
SLANG_UNIT_TEST(commandOptionsValueScopedToCategory)
{
    CommandOptions opts; fillSampleOptions(opts);
    Index formatCat = opts.findCategoryByName(toSlice("format"));
    Index generalCat = opts.findCategoryByName(toSlice("general"));

    SLANG_CHECK(opts.findValueByName(formatCat, toSlice("spirv")) >= 0);
    SLANG_CHECK(opts.findValueByName(formatCat, toSlice("hlsl")) >= 0);
    // The 'general' category has options not values — looking up a
    // value name there must miss.
    SLANG_CHECK(opts.findValueByName(generalCat, toSlice("spirv")) == -1);
}

// addValues from a ValuePair[] should register every entry.
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
    opts.addValues(pairs, SLANG_COUNT_OF(pairs));

    Index cat = opts.findCategoryByName(toSlice("level"));
    SLANG_CHECK(cat >= 0);
    SLANG_CHECK(opts.findValueByName(cat, toSlice("silent")) >= 0);
    SLANG_CHECK(opts.findValueByName(cat, toSlice("normal")) >= 0);
    SLANG_CHECK(opts.findValueByName(cat, toSlice("verbose")) >= 0);
    SLANG_CHECK(opts.findValueByName(cat, toSlice("missing")) == -1);
}

// ---------------------------------------------------------------
// CommandOptionsWriter — TEXT style
// ---------------------------------------------------------------

// TEXT: each Option-kind category gets a name + `===` underline, a
// description line, then each option formatted as
// `  <usage-or-names>: <description>`. The contract is:
//   * options whose `usage` is empty render with their alias list
//     (`-help, -h`); options with non-empty `usage` render the
//     usage instead (`<path>` for -output).
//   * Value-kind categories DO NOT render their value list inline;
//     they appear only in the "Getting Help" footer's "can be:"
//     listing. To get the value list, the user runs `slangc -h
//     <category>` separately. This keeps the default help short.
SLANG_UNIT_TEST(writerTextRendersOptionCategories)
{
    CommandOptions opts; fillSampleOptions(opts);
    CommandOptionsWriter::Options writerOptions;
    writerOptions.style = CommandOptionsWriter::Style::Text;
    auto writer = CommandOptionsWriter::create(writerOptions);
    writer->appendDescription(&opts);
    String text = writer->getBuilder().produceString();

    // Option categories: name + `===` underline + description.
    SLANG_CHECK(contains(text, "general\n======="));
    SLANG_CHECK(contains(text, "compile\n======="));
    SLANG_CHECK(contains(text, "General options"));
    SLANG_CHECK(contains(text, "Compilation options"));

    // Options without a `usage` string render via their alias list.
    SLANG_CHECK(contains(text, "-help, -h"));
    SLANG_CHECK(contains(text, "-version"));

    // Options with a non-empty `usage` render the usage in place of
    // the names (this is documented; -output's usage is `<path>`,
    // -target's is `<format>`).
    SLANG_CHECK(contains(text, "<path>"));
    SLANG_CHECK(contains(text, "<format>"));

    // Descriptions appear, separated from name/usage by `: `.
    SLANG_CHECK(contains(text, ": Show this help message"));
    SLANG_CHECK(contains(text, ": Output path"));
    SLANG_CHECK(contains(text, ": Target format"));

    // The "Getting Help" footer lists all category names so the
    // user can drill in.
    SLANG_CHECK(contains(text, "Getting Help"));
    SLANG_CHECK(contains(text, "<help-category> can be:"));
    SLANG_CHECK(contains(text, "general"));
    SLANG_CHECK(contains(text, "compile"));
    SLANG_CHECK(contains(text, "format"));

    // TEXT mode must NOT use markdown headings or link syntax.
    SLANG_CHECK(!contains(text, "##"));
    SLANG_CHECK(!contains(text, "<a id="));
    SLANG_CHECK(!contains(text, "](#"));
}

// Value-kind categories DO NOT inline their values in TEXT mode.
// They appear only by name in the footer's `<help-category> can be`
// listing. This keeps the default help short — the user types
// `slangc -h format` to see the value list.
SLANG_UNIT_TEST(writerTextSkipsValueListInline)
{
    CommandOptions opts; fillSampleOptions(opts);
    CommandOptionsWriter::Options writerOptions;
    writerOptions.style = CommandOptionsWriter::Style::Text;
    auto writer = CommandOptionsWriter::create(writerOptions);
    writer->appendDescription(&opts);
    String text = writer->getBuilder().produceString();

    // Value names should NOT appear inline in default rendering.
    SLANG_CHECK(!contains(text, "Vulkan SPIR-V"));
    SLANG_CHECK(!contains(text, "Direct3D HLSL"));

    // Asking for the category specifically renders the values.
    Index formatCat = opts.findCategoryByName(toSlice("format"));
    SLANG_CHECK_ABORT(formatCat >= 0);
    auto writer2 = CommandOptionsWriter::create(writerOptions);
    writer2->appendDescriptionForCategory(&opts, formatCat);
    String catText = writer2->getBuilder().produceString();
    SLANG_CHECK(contains(catText, "spirv"));
    SLANG_CHECK(contains(catText, "hlsl"));
    SLANG_CHECK(contains(catText, "Vulkan SPIR-V"));
}

// ---------------------------------------------------------------
// CommandOptionsWriter — MARKDOWN style
// ---------------------------------------------------------------

// MARKDOWN: includes a "Quick Links" section at the top, an
// `<a id>` anchor before each category, and `## category` /
// `### option` heading levels.
SLANG_UNIT_TEST(writerMarkdownEmitsAnchorsAndHeadings)
{
    CommandOptions opts; fillSampleOptions(opts);
    CommandOptionsWriter::Options writerOptions;
    writerOptions.style = CommandOptionsWriter::Style::Markdown;
    auto writer = CommandOptionsWriter::create(writerOptions);
    writer->appendDescription(&opts);
    String text = writer->getBuilder().produceString();

    // Quick Links table at the top.
    SLANG_CHECK(contains(text, "Quick Links"));
    SLANG_CHECK(contains(text, "(#general)"));
    SLANG_CHECK(contains(text, "(#format)"));

    // Anchors before category headings.
    SLANG_CHECK(contains(text, "<a id=\"general\"></a>"));
    SLANG_CHECK(contains(text, "<a id=\"format\"></a>"));

    // Markdown heading levels: `## category`, `### option`.
    SLANG_CHECK(contains(text, "## general"));
    SLANG_CHECK(contains(text, "## format"));
    SLANG_CHECK(contains(text, "### -help"));
    SLANG_CHECK(contains(text, "### -version"));
    SLANG_CHECK(contains(text, "### -output"));

    // Usage shown in bold; HTML-escaped so the angle brackets render
    // as text and not as HTML tags.
    SLANG_CHECK(contains(text, "**&lt;path&gt;**"));

    // Value entries appear as bullets with the name in code style.
    SLANG_CHECK(contains(text, "* `spirv`"));
    SLANG_CHECK(contains(text, "* `hlsl`"));
}

// ---------------------------------------------------------------
// CommandOptionsWriter — NO-LINK-MARKDOWN style
// ---------------------------------------------------------------

// NO-LINK-MD: same headings as MARKDOWN, but the Quick Links section
// and the `<a id>` anchors are stripped. Used when embedding into
// docs that don't want intra-page navigation links.
SLANG_UNIT_TEST(writerNoLinkMarkdownStripsLinks)
{
    CommandOptions opts; fillSampleOptions(opts);
    CommandOptionsWriter::Options writerOptions;
    writerOptions.style = CommandOptionsWriter::Style::NoLinkMarkdown;
    auto writer = CommandOptionsWriter::create(writerOptions);
    writer->appendDescription(&opts);
    String text = writer->getBuilder().produceString();

    // Headings still present.
    SLANG_CHECK(contains(text, "## general"));
    SLANG_CHECK(contains(text, "### -help"));

    // Anchors and Quick Links section stripped.
    SLANG_CHECK(!contains(text, "<a id="));
    SLANG_CHECK(!contains(text, "Quick Links"));
    SLANG_CHECK(!contains(text, "](#"));
}

// ---------------------------------------------------------------
// Per-category rendering
// ---------------------------------------------------------------

// `appendDescriptionForCategory` renders ONLY the requested category.
// Other categories' option names must not leak into the output.
SLANG_UNIT_TEST(writerSingleCategoryDoesNotLeakOthers)
{
    CommandOptions opts; fillSampleOptions(opts);
    Index compileIdx = opts.findCategoryByName(toSlice("compile"));
    SLANG_CHECK_ABORT(compileIdx >= 0);

    CommandOptionsWriter::Options writerOptions;
    writerOptions.style = CommandOptionsWriter::Style::Text;
    auto writer = CommandOptionsWriter::create(writerOptions);
    writer->appendDescriptionForCategory(&opts, compileIdx);
    String text = writer->getBuilder().produceString();

    // 'compile' content present.
    SLANG_CHECK(contains(text, "compile"));
    SLANG_CHECK(contains(text, "<format>"));
    SLANG_CHECK(contains(text, "Target format"));

    // 'general' content absent.
    SLANG_CHECK(!contains(text, "-help"));
    SLANG_CHECK(!contains(text, "-version"));
    SLANG_CHECK(!contains(text, "Show this help message"));

    // 'format' values absent.
    SLANG_CHECK(!contains(text, "spirv"));
    SLANG_CHECK(!contains(text, "hlsl"));
}

// ---------------------------------------------------------------
// Style catalog
// ---------------------------------------------------------------

// `getStyleInfos` exposes one entry per Style enum case, all named.
// Callers (slangc help) iterate this to render the available styles
// in `-help-style`. Adding a new Style without updating this catalog
// breaks the help listing.
SLANG_UNIT_TEST(writerStyleInfosCoversEnum)
{
    auto styles = CommandOptionsWriter::getStyleInfos();
    // Three styles (Text, Markdown, NoLinkMarkdown).
    SLANG_CHECK(styles.getCount() == 3);
    for (Index i = 0; i < styles.getCount(); ++i)
    {
        SLANG_CHECK(styles[i].names != nullptr);
        SLANG_CHECK(*styles[i].names != '\0');
    }
}

// ---------------------------------------------------------------
// Independence of writer instances
// ---------------------------------------------------------------

// Two writers built from the same Options must not share state —
// the catalog is rendered into each writer's own builder. (Reusing
// a writer's builder across renders would otherwise concatenate.)
SLANG_UNIT_TEST(writerInstancesAreIndependent)
{
    CommandOptionsWriter::Options writerOptions;
    auto a = CommandOptionsWriter::create(writerOptions);
    auto b = CommandOptionsWriter::create(writerOptions);
    SLANG_CHECK(a.get() != b.get());
    SLANG_CHECK(a->getBuilder().getLength() == 0);
    SLANG_CHECK(b->getBuilder().getLength() == 0);

    CommandOptions opts; fillSampleOptions(opts);
    a->appendDescription(&opts);
    SLANG_CHECK(a->getBuilder().getLength() > 0);
    // b's builder must still be empty — no shared state.
    SLANG_CHECK(b->getBuilder().getLength() == 0);
}
