#pragma once
#include <cstdint>
#include <string_view>
#include <vector>

namespace spvdb
{

// Loaded image data: always stored as RGBA float, row-major, values in [0,1].
struct ImageData
{
    int32_t width = 0;
    int32_t height = 0;
    std::vector<float> pixels; // 4 floats per pixel (RGBA)

    bool valid() const
    {
        return width > 0 && height > 0;
    }

    // Read one texel, clamping coords to [0, width/height-1].
    void fetch_texel(int x, int y, float out[4]) const;

    // Sample by UV coordinates using bilinear interpolation, wrapping [0,1).
    void sample_bilinear(float u, float v, float out[4]) const;
};

// Load an image from disk (PNG, BMP, JPG, TGA, HDR via stb_image).
// Returns an invalid (empty) ImageData on failure.
ImageData load_image(std::string_view path);

} // namespace spvdb
