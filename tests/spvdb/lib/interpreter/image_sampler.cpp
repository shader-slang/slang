// SPVDB_SKIP_STB_IMAGE_IMPL is set by CMake when spvdb is built as a
// subdirectory of a project that already compiles the stb_image implementation
// (e.g. slang-test). In that case, omit the implementation here and rely on
// the parent to provide it at link time.
#ifndef SPVDB_SKIP_STB_IMAGE_IMPL
#define STB_IMAGE_IMPLEMENTATION
#endif
#define STBI_FAILURE_USERMSG
// Suppress warnings from stb_image internals.
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#include <stb_image.h>
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include "image_sampler.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

namespace spvdb
{

ImageData load_image(std::string_view path)
{
    std::string p(path);
    int w = 0, h = 0, c = 0;
    // Always load as 4 channels (RGBA), always as float [0,1].
    float *data = stbi_loadf(p.c_str(), &w, &h, &c, 4);
    if (!data)
        return {};
    ImageData img;
    img.width = w;
    img.height = h;
    img.pixels.assign(data, data + static_cast<size_t>(w) * h * 4);
    stbi_image_free(data);
    return img;
}

void ImageData::fetch_texel(int x, int y, float out[4]) const
{
    x = std::max(0, std::min(x, width - 1));
    y = std::max(0, std::min(y, height - 1));
    const float *p = pixels.data() + (static_cast<size_t>(y) * width + x) * 4;
    out[0] = p[0];
    out[1] = p[1];
    out[2] = p[2];
    out[3] = p[3];
}

void ImageData::sample_bilinear(float u, float v, float out[4]) const
{
    if (!valid()) {
        out[0] = out[1] = out[2] = out[3] = 0.0f;
        return;
    }
    // Wrap UVs to [0,1).
    u -= std::floor(u);
    v -= std::floor(v);
    float fx = u * static_cast<float>(width - 1);
    float fy = v * static_cast<float>(height - 1);
    int x0 = static_cast<int>(fx);
    int y0 = static_cast<int>(fy);
    float tx = fx - static_cast<float>(x0);
    float ty = fy - static_cast<float>(y0);
    float c00[4], c10[4], c01[4], c11[4];
    fetch_texel(x0, y0, c00);
    fetch_texel(x0 + 1, y0, c10);
    fetch_texel(x0, y0 + 1, c01);
    fetch_texel(x0 + 1, y0 + 1, c11);
    for (int i = 0; i < 4; ++i) {
        out[i] = (1.0f - tx) * (1.0f - ty) * c00[i] + tx * (1.0f - ty) * c10[i]
               + (1.0f - tx) * ty * c01[i] + tx * ty * c11[i];
    }
}

} // namespace spvdb
