#include "cocoa-util.h"

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

namespace gfx {

void CocoaUtil::getNSViewRectSize(void* nsview, int* widthOut, int* heightOut)
{
    NSView* view = (NSView*)nsview;
    NSRect rect = [view frame];
    *widthOut = rect.size.width;
    *heightOut = rect.size.height;
}

void* CocoaUtil::createMetalLayer(void* nswindow)
{
    CAMetalLayer *layer = [CAMetalLayer layer];
    NSWindow* window = (NSWindow*)nswindow;
    window.contentView.layer = layer;
    window.contentView.wantsLayer = YES;
    return layer;
}

void CocoaUtil::destroyMetalLayer(void* metalLayer)
{
    CAMetalLayer* layer = (CAMetalLayer*)metalLayer;
    [layer release];
}

}