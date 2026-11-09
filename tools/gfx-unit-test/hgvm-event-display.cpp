// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#include "slangi/hgvm-event-display.h"

#include "hgvm-test-helpers.h"

using namespace Slang;
using namespace Slang::HGVM;
using namespace SlangITool;

SLANG_UNIT_TEST(hgvmEventDisplayStyles)
{
    EventText text;
    text << EventTextStyle::Label << "dispatch";
    text.field("type", "Buffer<float>", EventTextStyle::Type);
    text.field("name", "\"hello\"", EventTextStyle::String);
    text.field("ms", "1.250", EventTextStyle::Duration);
    text.field("count", 4);
    text.field("address", "0x123", EventTextStyle::Dim);
    text << '\n';
    String plain = "dispatch type=Buffer<float> name=\"hello\" ms=1.250 count=4 address=0x123\n";
    SLANG_CHECK(text.render(false) == plain);
    String color = text.render(true);
    SLANG_CHECK(
        color == "\x1b[1mdispatch\x1b[0m type=\x1b[36mBuffer<float>\x1b[0m"
                 " name=\x1b[32m\"hello\"\x1b[0m ms=\x1b[33m1.250\x1b[0m"
                 " count=\x1b[35m4\x1b[0m address=\x1b[2m0x123\x1b[0m\n");

    EventText multiline;
    multiline << EventTextStyle::Type << "first\nsecond\n";
    SLANG_CHECK(multiline.render(true) == "\x1b[36mfirst\x1b[0m\n\x1b[36msecond\x1b[0m\n");
}

SLANG_UNIT_TEST(hgvmEventDisplayClipping)
{
    EventText text;
    text << EventTextStyle::Label << "abc" << EventTextStyle::Type << "defghi";
    auto length = text.getText().getLength();
    SLANG_CHECK(text.render(false, 0, length, 6) == "abc...");
    SLANG_CHECK(text.render(true, 0, length, 7) == "\x1b[1mabc\x1b[0m\x1b[36md\x1b[0m...");
    SLANG_CHECK(text.render(false, 0, length, 0) == "");
    SLANG_CHECK(text.render(false, 0, length, 2) == "ab");
    SLANG_CHECK(text.render(false, 0, length, 3) == "...");
    SLANG_CHECK(text.render(false, 0, length, 9) == "abcdefghi");
    SLANG_CHECK(text.render(true, 3, length, 4) == "\x1b[36md\x1b[0m...");

    // A two-byte accented letter takes one column; a CJK character takes two. Neither
    // may be split when a long pipeline name is clipped at the terminal's right edge.
    EventText unicode;
    unicode << EventTextStyle::Type << "\xc3\xa9\xe7\x95\x8c" << "abcdef";
    length = unicode.getText().getLength();
    SLANG_CHECK(unicode.render(false, 0, length, 6) == "\xc3\xa9\xe7\x95\x8c...");
    SLANG_CHECK(unicode.render(false, 0, length, 5) == "\xc3\xa9...");
    SLANG_CHECK(unicode.render(false, 0, length, 2) == "\xc3\xa9");

    EventText combining;
    combining << "e\xcc\x81" << "abcde";
    SLANG_CHECK(combining.render(false, 0, combining.getText().getLength(), 4) == "e\xcc\x81...");

    EventText controls;
    controls << "a\t\x1b" << "b\r";
    SLANG_CHECK(controls.render(false) == "a  b ");
    SLANG_CHECK(controls.render(false, 0, controls.getText().getLength(), 5) == "a  b ");
}

SLANG_UNIT_TEST(hgvmEventDisplayNames)
{
    auto program = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            class Box { int value; __init() { value = 42; } }
            [host] void main() { let box = new Box(); print(box.value); }
        )",
        SLANG_DEBUG_INFO_LEVEL_MINIMAL,
        "display_module");
    DebugMapBuilder builder(program);
    auto map = builder.getDebugMap();
    SLANG_CHECK(map->getModuleNames().contains(String("display_module")));
    HGVMEventDisplay compact;
    HGVMEventDisplay full(true);
    String name = "display_module::run<Buffer<display_module::Scene.Node>, 4>";
    SLANG_CHECK(compact.getDisplayName(map, name) == "run<Buffer<Scene.Node>, 4>");
    SLANG_CHECK(full.getDisplayName(map, name) == name);
    SLANG_CHECK(compact.getDisplayName(map, "display_module::Outer.Inner") == "Outer.Inner");
    SLANG_CHECK(compact.getDisplayName(map, "Scene::Node") == "Scene::Node");
    SLANG_CHECK(
        compact.getDisplayName(map, "other/display_module::Node") == "other/display_module::Node");
    SLANG_CHECK(
        compact.getDisplayName(map, "prefix_display_module::Node") ==
        "prefix_display_module::Node");
    SLANG_CHECK(compact.getDisplayName(nullptr, name) == name);
    SLANG_CHECK(
        compact.getDisplayName(
            map,
            "display_module::vertex<float> + display_module::fragment<float>") ==
        "vertex<float> + fragment<float>");
    // Formatting never changes the canonical metadata used for lookup and aggregation.
    SLANG_CHECK(full.getDisplayName(map, name) == name);
}
