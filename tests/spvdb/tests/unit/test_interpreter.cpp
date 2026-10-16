#include "../../lib/api/spvdb.h"
#include "../../lib/api/spvdb_session.h"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

using namespace spvdb;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string spv(const char *name)
{
    return std::string(SPIRV_TEST_DIR) + "/" + name + ".spv";
}

// Write a minimal 1×1 24-bit BMP with the given RGB values.
// Returns the file path on success, empty string on failure.
static std::string write_test_bmp_1x1(const char *filename, uint8_t r, uint8_t g, uint8_t b)
{
    std::string path = std::string(SPVDB_TEST_TMP_DIR) + "/" + filename;
    // BMP format: file header (14) + info header (40) + pixel data (4) = 58 bytes.
    // Pixels are stored BGR with row padded to 4-byte boundary.
    uint8_t bmp[58] = {};
    // File header
    bmp[0] = 'B';
    bmp[1] = 'M';
    bmp[2] = 58; // file size (little-endian)
    bmp[10] = 54; // pixel data offset
    // Info header (BITMAPINFOHEADER)
    bmp[14] = 40; // header size
    bmp[18] = 1; // width = 1
    bmp[22] = 1; // height = 1 (positive = bottom-up)
    bmp[26] = 1; // planes
    bmp[28] = 24; // bits per pixel
    bmp[34] = 4; // image size (1 pixel + 1 padding byte × 2, rounded to 4)
    // Pixel data: BGR order + 1 padding byte
    bmp[54] = b;
    bmp[55] = g;
    bmp[56] = r;
    bmp[57] = 0;
    std::ofstream f(path, std::ios::binary);
    if (!f)
        return {};
    f.write(reinterpret_cast<const char *>(bmp), sizeof(bmp));
    return path;
}

// Build a JSON array string from a vector of uint32 values, e.g. "[1, 2, 3]".
static std::string json_u32(std::initializer_list<uint32_t> vals)
{
    std::string s = "[";
    bool first = true;
    for (auto v : vals) {
        if (!first)
            s += ", ";
        s += std::to_string(v);
        first = false;
    }
    return s + "]";
}

// Read the first N uint32 values back from a descriptor binding.
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
// Tests
// ---------------------------------------------------------------------------

TEST_CASE("interpreter: minimal compute shader runs to completion")
{
    auto mod = load_module_from_file(spv("minimal"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    auto reason = run(**sess);
    CHECK(reason == StopReason::EntryFinished);
}

TEST_CASE("interpreter: entry point enumeration")
{
    auto mod = load_module_from_file(spv("minimal"));
    REQUIRE(mod);

    auto eps = list_entry_points(**mod);
    REQUIRE(eps.size() == 1);
    CHECK(eps[0].name == "main");
    CHECK(eps[0].execution_model == ExecutionModel::GLCompute);
}

TEST_CASE("interpreter: copy_uint - output[0] == input[0]")
{
    auto mod = load_module_from_file(spv("copy_uint"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({42})));
    REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    auto out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 42u);
}

TEST_CASE("interpreter: add_uint - output[0] == input[0] + input[1]")
{
    auto mod = load_module_from_file(spv("add_uint"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({7, 13})));
    REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    auto out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 20u);
}

TEST_CASE("interpreter: add_uint - multiple invocations produce independent results")
{
    auto mod = load_module_from_file(spv("add_uint"));
    REQUIRE(mod);

    for (uint32_t a : {0u, 1u, 100u, 0xFFFF0000u}) {
        uint32_t b = 5;
        auto sess = create_session(*mod, "main");
        REQUIRE(sess);
        REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({a, b})));
        REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));
        auto reason = run(**sess);
        if (reason == StopReason::Panic)
            FAIL("Panic: " + panic_message(**sess));
        REQUIRE(reason == StopReason::EntryFinished);
        auto out = read_u32s(**sess, 0, 1, 1);
        CHECK(out[0] == (a + b));
    }
}

TEST_CASE("interpreter: loop_sum_inline - sums 4 elements via inline loop")
{
    auto mod = load_module_from_file(spv("loop_sum_inline"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({10, 20, 30, 40})));
    REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    auto out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 100u);
}

TEST_CASE("interpreter: loop_sum - sums 4 elements via loop and function call")
{
    auto mod = load_module_from_file(spv("loop_sum"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({10, 20, 30, 40})));
    REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    auto out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 100u);
}

TEST_CASE("interpreter: spec_const - default multiplier (3) applied")
{
    auto mod = load_module_from_file(spv("spec_const"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({7})));
    REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);
    auto out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 21u); // 7 * 3
}

TEST_CASE("interpreter: spec_const - overridden multiplier applied")
{
    auto mod = load_module_from_file(spv("spec_const"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    // Override SpecId 7 to value 10.
    REQUIRE(set_spec_constant(**sess, 7, static_cast<uint32_t>(10)));

    REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({5})));
    REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);
    auto out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 50u); // 5 * 10
}

TEST_CASE("interpreter: step_instruction advances through copy_uint")
{
    auto mod = load_module_from_file(spv("copy_uint"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({99})));
    REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));

    // Step until done; should never panic.
    StopReason reason = StopReason::Step;
    int steps = 0;
    while (reason != StopReason::EntryFinished && reason != StopReason::Panic) {
        reason = step_instruction(**sess);
        REQUIRE(steps++ < 1000); // Guard against infinite loops.
    }
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    auto out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 99u);
}

TEST_CASE("interpreter: breakpoint on result id - no match, run to completion")
{
    auto mod = load_module_from_file(spv("add_uint"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({3, 4})));
    REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));

    // id 0 won't match any real result id -run should finish normally.
    set_breakpoint_at_id(**sess, 0);
    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    CHECK(reason == StopReason::EntryFinished);

    auto out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 7u);
}

TEST_CASE("interpreter: fragment passthrough - input location 0 copied to output")
{
    auto mod = load_module_from_file(spv("passthrough_frag"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    // Bind a vec4 (1.0, 0.5, 0.25, 1.0) to input location 0.
    Value color = Value::make_composite({Value::make_f32(1.0f),
                                         Value::make_f32(0.5f),
                                         Value::make_f32(0.25f),
                                         Value::make_f32(1.0f)});
    REQUIRE(set_input_location(**sess, 0, color));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    // Output variables should contain the same vec4.
    auto outs = output_variables(**sess);
    REQUIRE(outs.size() == 1);
    REQUIRE(outs[0].value.kind == Value::Kind::Composite);
    REQUIRE(outs[0].value.elements.size() == 4);
    CHECK(outs[0].value.elements[0].scalar.f32 == 1.0f);
    CHECK(outs[0].value.elements[1].scalar.f32 == 0.5f);
    CHECK(outs[0].value.elements[2].scalar.f32 == 0.25f);
    CHECK(outs[0].value.elements[3].scalar.f32 == 1.0f);
}

TEST_CASE("interpreter: image sampling - no image bound returns zero without panic")
{
    auto mod = load_module_from_file(spv("image_sample_frag"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    // UV = (0.5, 0.5); no image bound at set=0, binding=0.
    Value uv = Value::make_composite({Value::make_f32(0.5f), Value::make_f32(0.5f)});
    REQUIRE(set_input_location(**sess, 0, uv));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    // Output should be zero (the unbound-image diagnostic path).
    auto outs = output_variables(**sess);
    REQUIRE(outs.size() == 1);
    REQUIRE(outs[0].value.kind == Value::Kind::Composite);
    REQUIRE(outs[0].value.elements.size() == 4);
    CHECK(outs[0].value.elements[0].scalar.f32 == 0.0f);
    CHECK(outs[0].value.elements[1].scalar.f32 == 0.0f);
    CHECK(outs[0].value.elements[2].scalar.f32 == 0.0f);
    CHECK(outs[0].value.elements[3].scalar.f32 == 0.0f);
}

TEST_CASE("interpreter: image sampling - 1x1 red BMP returns red pixel")
{
    auto mod = load_module_from_file(spv("image_sample_frag"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    // Write a 1×1 red BMP and bind it to set=0, binding=0.
    std::string img_path = write_test_bmp_1x1("red.bmp", 255, 0, 0);
    REQUIRE(!img_path.empty());
    REQUIRE(set_image(**sess, 0, 0, img_path));

    // UV = (0.5, 0.5) -for a 1×1 image, any UV resolves to the single texel.
    Value uv = Value::make_composite({Value::make_f32(0.5f), Value::make_f32(0.5f)});
    REQUIRE(set_input_location(**sess, 0, uv));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    // Output should be approximately (1.0, 0.0, 0.0, 1.0).
    auto outs = output_variables(**sess);
    REQUIRE(outs.size() == 1);
    REQUIRE(outs[0].value.kind == Value::Kind::Composite);
    REQUIRE(outs[0].value.elements.size() == 4);
    CHECK(outs[0].value.elements[0].scalar.f32 == Catch::Approx(1.0f).margin(0.01f));
    CHECK(outs[0].value.elements[1].scalar.f32 == Catch::Approx(0.0f).margin(0.01f));
    CHECK(outs[0].value.elements[2].scalar.f32 == Catch::Approx(0.0f).margin(0.01f));
    CHECK(outs[0].value.elements[3].scalar.f32 == Catch::Approx(1.0f).margin(0.01f));
}

TEST_CASE("interpreter: fragment derivative - OpDPdx returns zero (single invocation)")
{
    auto mod = load_module_from_file(spv("deriv_frag"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    // Set input color to something non-zero; derivative should still be zero.
    Value color = Value::make_composite({Value::make_f32(1.0f),
                                         Value::make_f32(2.0f),
                                         Value::make_f32(3.0f),
                                         Value::make_f32(4.0f)});
    REQUIRE(set_input_location(**sess, 0, color));

    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    REQUIRE(reason == StopReason::EntryFinished);

    // OpDPdx in single-invocation mode returns zero.
    auto outs = output_variables(**sess);
    REQUIRE(outs.size() == 1);
    REQUIRE(outs[0].value.kind == Value::Kind::Composite);
    REQUIRE(outs[0].value.elements.size() == 4);
    CHECK(outs[0].value.elements[0].scalar.f32 == 0.0f);
    CHECK(outs[0].value.elements[1].scalar.f32 == 0.0f);
    CHECK(outs[0].value.elements[2].scalar.f32 == 0.0f);
    CHECK(outs[0].value.elements[3].scalar.f32 == 0.0f);
}

TEST_CASE("interpreter: breakpoint on result id fires before instruction executes")
{
    // In add_uint.spv: %22 = OpIAdd %uint %19 %21  (the addition result)
    // %23 = OpAccessChain ... (out ptr), then OpStore %23 %22.
    // A breakpoint on %22 should stop BEFORE the add, so the output buffer
    // still holds its initial value (0).
    auto mod = load_module_from_file(spv("add_uint"));
    REQUIRE(mod);

    auto sess = create_session(*mod, "main");
    REQUIRE(sess);

    REQUIRE(set_descriptor_json(**sess, 0, 0, json_u32({3, 4})));
    REQUIRE(set_descriptor_json(**sess, 0, 1, json_u32({0})));

    set_breakpoint_at_id(**sess, 22); // %22 = OpIAdd
    auto reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic: " + panic_message(**sess));
    CHECK(reason == StopReason::Breakpoint);

    // Output not yet written -the store comes after the add.
    auto out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 0u);

    // Continuing should finish and produce the result.
    reason = run(**sess);
    if (reason == StopReason::Panic)
        FAIL("Panic after continue: " + panic_message(**sess));
    CHECK(reason == StopReason::EntryFinished);
    out = read_u32s(**sess, 0, 1, 1);
    CHECK(out[0] == 7u);
}
