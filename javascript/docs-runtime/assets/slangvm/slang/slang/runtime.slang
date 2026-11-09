// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Re-exports the runtime API for host programs and device callbacks: core helpers, I/O,
// resources, compute dispatch, rasterization, and window/input handling.
// Runtime implementation modules import their dependencies directly to avoid import cycles.

__exported import slang.runtime.core.simple_assert;
__exported import slang.runtime.core.simple_string;
__exported import slang.runtime.core.simple_time;

__exported import slang.runtime.io.byte_stream;
__exported import slang.runtime.io.exr_header;
__exported import slang.runtime.io.image;
__exported import slang.runtime.io.image_header;
__exported import slang.runtime.io.simple_file;

__exported import slang.runtime.resource.buffer_view;
__exported import slang.runtime.resource.simple_texture;
__exported import slang.runtime.resource.texture_clear;
__exported import slang.runtime.resource.texture_format;
__exported import slang.runtime.resource.upload_buffer;

__exported import slang.runtime.compute.grid;

__exported import slang.runtime.raster.draw_parameters;
__exported import slang.runtime.raster.primitive_stream;
__exported import slang.runtime.raster.raster_state;
__exported import slang.runtime.raster.render_targets;

__exported import slang.runtime.window.simple_input;
__exported import slang.runtime.window.simple_window;
