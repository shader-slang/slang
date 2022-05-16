#version 450
// geometry-shader.slang.glsl
//TEST_IGNORE_FILE:

#define RasterVertex RasterVertex_0
#define CoarseVertex CoarseVertex_0

#define input_position  _S1
#define input_color     _S2
#define input_id        _S3
#define output_position _S4
#define output_color    _S5

layout(row_major) uniform;
layout(row_major) buffer;

layout(location = 0)
in vec4  input_position[3];

layout(location = 1)
in vec3  input_color[3];

layout(location = 2)
in uint  input_id[3];

layout(location = 0)
out vec4 output_position;

layout(location = 1)
out vec3 output_color;

struct RasterVertex
{
    vec4 position_0;
    vec3 color_0;
    uint id_0;
};

struct CoarseVertex
{
    vec4 position_1;
    vec3 color_1;
    uint id_1;
};


layout(max_vertices = 3) out;
layout(triangles) in;
layout(triangle_strip) out;

void main()
{
    uint _S6 = uint(gl_PrimitiveIDIn);

    // TODO: Having to make this copy to transpose things is unfortunate.
    //
    // The front-end should be able to generate code using aggregate
    // types for the input, and/or eliminate the redundant temporary
    // by indexing directly into the sub-arrays.
    //
    CoarseVertex_0  _S7[3] = {
        CoarseVertex_0(input_position[0], input_color[0], input_id[0]),
        CoarseVertex_0(input_position[1], input_color[1], input_id[1]),
        CoarseVertex_0(input_position[2], input_color[2], input_id[2])
    };

    int ii_0;
    ii_0 = 0;
    for(;;)
    {
        if(ii_0 < 3)
        {}
        else
        {
            break;
        }

        CoarseVertex_0 coarseVertex_0 = _S7[ii_0];

        RasterVertex_0 rasterVertex_0;
        rasterVertex_0.position_0 = coarseVertex_0.position_1;
        rasterVertex_0.color_0 = coarseVertex_0.color_1;
        rasterVertex_0.id_0 = coarseVertex_0.id_1 + _S6;

        RasterVertex_0 _S8 = rasterVertex_0;

        output_position = _S8.position_0;
        output_color = _S8.color_0;
        gl_Layer = int(_S8.id_0);

        EmitVertex();

        ii_0 = ii_0 + 1;
    }

    return;
}
