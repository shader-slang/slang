// Override module for link-time-vk-binding-glsl-resolve: links SLOT = 8, so the
// symbolic GLSL `layout(binding = SLOT, set = SLOT - 2)` re-selects binding 8 / set 6.

export static const int SLOT = 8;
