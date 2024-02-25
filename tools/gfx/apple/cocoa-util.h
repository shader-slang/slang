#pragma once

namespace gfx {

// Utility functions for Cocoa
struct CocoaUtil {

    static void getNSViewRectSize(void* nsview, int* widthOut, int* heightOut);

    static void* createMetalLayer(void* nswindow);
    static void destroyMetalLayer(void* metalLayer);

};

}
