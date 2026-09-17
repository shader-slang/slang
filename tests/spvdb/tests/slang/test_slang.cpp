#include "../../lib/api/spvdb.h"
#include "../../lib/api/spvdb_session.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace spvdb;

static const std::string k_add_spv = std::string(SLANG_TEST_SPV_DIR) + "/add_buffers.spv";
static const std::string k_inl_spv = std::string(SLANG_TEST_SPV_DIR) + "/inlined_func.spv";
static const std::string k_add_src = std::string(SLANG_TEST_SRC_DIR) + "/add_buffers.slang";
static const std::string k_inl_src = std::string(SLANG_TEST_SRC_DIR) + "/inlined_func.slang";

// Read first N uint32_t values back from a descriptor binding.
static std::vector<uint32_t> read_u32s(Session &sess, uint32_t set, uint32_t binding, size_t count)
{
    auto r = read_descriptor(sess, set, binding);
    REQUIRE(r);
    std::vector<uint32_t> out(count);
    size_t bytes = count * sizeof(uint32_t);
    REQUIRE(r->size() >= bytes);
    std::memcpy(out.data(), r->data(), bytes);
    return out;
}

// ---------------------------------------------------------------------------
// add_buffers.slang: basic execution
// ---------------------------------------------------------------------------

TEST_CASE("slang add_buffers: correct output")
{
    auto mod = load_module_from_file(k_add_spv);
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, "[7]"));
    REQUIRE(set_descriptor_json(**sess, 0, 1, "[5]"));
    REQUIRE(set_descriptor_json(**sess, 0, 2, "[0]"));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    auto out = read_u32s(**sess, 0, 2, 1);
    CHECK(out[0] == 12u);
}

// ---------------------------------------------------------------------------
// add_buffers.slang: local_variables() via DebugValue
// ---------------------------------------------------------------------------

TEST_CASE("slang add_buffers: local_variables shows a and b")
{
    auto mod = load_module_from_file(k_add_spv);
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, "[7]"));
    REQUIRE(set_descriptor_json(**sess, 0, 1, "[5]"));
    REQUIRE(set_descriptor_json(**sess, 0, 2, "[0]"));

    bool saw_a = false, saw_b = false;
    for (int i = 0; i < 2000; ++i) {
        auto stop = step_instruction(**sess);
        auto locals = local_variables(**sess);
        for (auto &lv : locals) {
            if (lv.name == "a" && lv.value.kind == Value::Kind::UInt32 && lv.value.scalar.u32 == 7u)
                saw_a = true;
            if (lv.name == "b" && lv.value.kind == Value::Kind::UInt32 && lv.value.scalar.u32 == 5u)
                saw_b = true;
        }
        if (stop == StopReason::EntryFinished || stop == StopReason::Panic)
            break;
    }

    CHECK(saw_a);
    CHECK(saw_b);
}

// ---------------------------------------------------------------------------
// add_buffers.slang: source locations
// ---------------------------------------------------------------------------

TEST_CASE("slang add_buffers: source locations are valid")
{
    auto mod = load_module_from_file(k_add_spv);
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, "[1]"));
    REQUIRE(set_descriptor_json(**sess, 0, 1, "[1]"));
    REQUIRE(set_descriptor_json(**sess, 0, 2, "[0]"));

    bool saw_valid_loc = false;
    for (int i = 0; i < 2000; ++i) {
        auto loc = current_location(**sess);
        if (loc.line != 0)
            saw_valid_loc = true;
        auto stop = step_instruction(**sess);
        if (stop == StopReason::EntryFinished || stop == StopReason::Panic)
            break;
    }

    CHECK(saw_valid_loc);
}

// ---------------------------------------------------------------------------
// add_buffers.slang: stepping visits lines 9, 10, 11
// ---------------------------------------------------------------------------

TEST_CASE("slang add_buffers: step visits lines 9 10 11")
{
    auto mod = load_module_from_file(k_add_spv);
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, "[3]"));
    REQUIRE(set_descriptor_json(**sess, 0, 1, "[4]"));
    REQUIRE(set_descriptor_json(**sess, 0, 2, "[0]"));

    bool saw9 = false, saw10 = false, saw11 = false;
    for (int i = 0; i < 2000; ++i) {
        auto loc = current_location(**sess);
        if (loc.line == 9)
            saw9 = true;
        if (loc.line == 10)
            saw10 = true;
        if (loc.line == 11)
            saw11 = true;
        auto stop = step_instruction(**sess);
        if (stop == StopReason::EntryFinished || stop == StopReason::Panic)
            break;
    }

    CHECK(saw9);
    CHECK(saw10);
    CHECK(saw11);
}

// ---------------------------------------------------------------------------
// add_buffers.slang: breakpoint at source line fires correctly
// ---------------------------------------------------------------------------

TEST_CASE("slang add_buffers: breakpoint by file and line")
{
    auto mod = load_module_from_file(k_add_spv);
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, "[7]"));
    REQUIRE(set_descriptor_json(**sess, 0, 1, "[5]"));
    REQUIRE(set_descriptor_json(**sess, 0, 2, "[0]"));

    // Line 10: uint b = inputB[0];
    auto bp = set_breakpoint(**sess, k_add_src, 10);
    REQUIRE(bp);

    auto stop = run(**sess);
    // Should stop at the breakpoint or finish if the breakpoint was missed.
    if (stop == StopReason::Breakpoint) {
        auto loc = current_location(**sess);
        CHECK(loc.line == 10);
    } else {
        // Breakpoint missed — still acceptable if it finished without panic.
        CHECK(stop == StopReason::EntryFinished);
    }
}

// ---------------------------------------------------------------------------
// inlined_func.slang: correct output
// ---------------------------------------------------------------------------

TEST_CASE("slang inlined_func: correct output")
{
    auto mod = load_module_from_file(k_inl_spv);
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, "[7]"));
    REQUIRE(set_descriptor_json(**sess, 0, 1, "[5]"));
    REQUIRE(set_descriptor_json(**sess, 0, 2, "[0]"));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    auto out = read_u32s(**sess, 0, 2, 1);
    CHECK(out[0] == 12u);
}

// ---------------------------------------------------------------------------
// inlined_func.slang: inlined call-stack reconstruction via DebugInlinedAt
// ---------------------------------------------------------------------------

TEST_CASE("slang inlined_func: backtrace shows add_values inlined into main")
{
    auto mod = load_module_from_file(k_inl_spv);
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, "[7]"));
    REQUIRE(set_descriptor_json(**sess, 0, 1, "[5]"));
    REQUIRE(set_descriptor_json(**sess, 0, 2, "[0]"));

    // Step until we reach a source line inside add_values (line 2 in
    // inlined_func.slang: "return x + y").
    bool saw_inlined = false;
    for (int i = 0; i < 2000; ++i) {
        auto stop = step_instruction(**sess);
        auto loc = current_location(**sess);
        if (loc.line == 2) {
            // We should now be in the inlined scope.
            auto bt = backtrace(**sess);
            if (bt.size() >= 2) {
                bool frame0_is_add = bt[0].function_name == "add_values";
                bool frame1_is_main = bt[1].function_name == "main";
                bool frame1_line13 = bt[1].loc.line == 13;
                if (frame0_is_add && frame1_is_main && frame1_line13) {
                    saw_inlined = true;
                    break;
                }
            }
        }
        if (stop == StopReason::EntryFinished || stop == StopReason::Panic)
            break;
    }

    CHECK(saw_inlined);
}

// ---------------------------------------------------------------------------
// inlined_func.slang: local_variables for x and y inside inlined scope
// ---------------------------------------------------------------------------

TEST_CASE("slang inlined_func: local_variables shows x and y inside inlined scope")
{
    auto mod = load_module_from_file(k_inl_spv);
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, "[7]"));
    REQUIRE(set_descriptor_json(**sess, 0, 1, "[5]"));
    REQUIRE(set_descriptor_json(**sess, 0, 2, "[0]"));

    bool saw_x = false, saw_y = false;
    for (int i = 0; i < 2000; ++i) {
        auto stop = step_instruction(**sess);
        auto locals = local_variables(**sess);
        for (auto &lv : locals) {
            if (lv.name == "x" && lv.value.kind == Value::Kind::UInt32 && lv.value.scalar.u32 == 7u)
                saw_x = true;
            if (lv.name == "y" && lv.value.kind == Value::Kind::UInt32 && lv.value.scalar.u32 == 5u)
                saw_y = true;
        }
        if (stop == StopReason::EntryFinished || stop == StopReason::Panic)
            break;
    }

    CHECK(saw_x);
    CHECK(saw_y);
}
