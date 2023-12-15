#include "cocoa-util.h"

#import <Cocoa/Cocoa.h>

namespace gfx {

void CocoaUtil::getNSViewRectSize(void* nsview, int* widthOut, int* heightOut)
{
    NSView* view = (NSView*)nsview;
    NSRect rect = [view frame];
    *widthOut = rect.size.width;
    *heightOut = rect.size.height;
}

}