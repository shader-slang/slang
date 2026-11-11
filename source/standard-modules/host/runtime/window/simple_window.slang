// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.runtime.resource.texture_format;


// Opaque host window type.
__target_intrinsic(hgbc, "OpWindowType") public struct Window
{
};

public Window createWindow(String title, uint width, uint height)
{
    return hgbc_asm
    {
    result:
        $$Window = OpCreateWindow $title $width $height;
    };
}

// Processes pending events and returns true while the window remains open.
public bool processWindowEvents(Window window)
{
    return hgbc_asm
    {
    result:
        $$bool = OpProcessWindowEvents $window;
    };
}

public void setTitle(Window window, String title)
{
    hgbc_asm
    {
        OpSetWindowTitle $window $title;
    };
}

/// The default surface uses normalized sRGB storage and exposes linear `float4` shader values.
public typealias DefaultSurface = Surface<TextureFormat::rgba8UnormSrgb>;
public typealias DefaultSurfaceTarget = RenderTarget2D<float4, TextureFormat::rgba8UnormSrgb>;

public extension<T, let format : int> RenderTarget2D<T, format>
{
    /// Reports the current surface-image dimensions through the same HGBC texture query.
    public void GetDimensions(out uint width, out uint height)
    {
        uint2 dimensions = hgbc_asm
        {
        result:
            $$uint2 = OpTextureGetDimensions $this;
        };
        width = dimensions.x;
        height = dimensions.y;
    }
}

/// Creates a surface with an explicitly selected logical storage format.
///
/// For example, `createSurface<TextureFormat::rgba16Float>(window)` requests an HDR-capable
/// floating-point swap chain. Surface acquisition preserves this format in its result type.
public Surface<format> createSurface<let format : int>(Window window, uint imageCount = 2)
{
    return hgbc_asm
    {
    result:
        $$Surface<format> = OpCreateSurface $window $imageCount;
    };
}

/// Creates the ordinary sRGB8 surface used by demos that do not request another format.
public DefaultSurface createSurface(Window window, uint imageCount = 2)
{
    return hgbc_asm
    {
    result:
        $$DefaultSurface = OpCreateSurface $window $imageCount;
    };
}

/// Finishes submitted work and replaces the swapchain at the requested size.
/// Call after handling a windowResize event and releasing all acquired-image references.
/// Returns false if the drawable area is temporarily unavailable (for example, minimized).
/// This changes the surface dimensions, not the native window dimensions.
public bool resizeSurface<let format : int>(Surface<format> surface, uint width, uint height)
{
    return hgbc_asm
    {
    result:
        $$bool = OpResizeSurface $surface $width $height;
    };
}

public extension<let format : int> Surface<format>
{
    /// Reports configured image dimensions without acquiring an image or waiting for the GPU.
    /// Returns zero dimensions for an unconfigured surface.
    public uint2 getDimensions()
    {
        return hgbc_asm
        {
        result:
            $$uint2 = OpGetSurfaceDimensions $this;
        };
    }
}

/// Waits for the next surface image without exposing it or starting surface rendering.
/// Pump window events after this wait to sample fresh input, then render offscreen and acquire.
/// Repeated waits before acquisition reuse the same reservation. Resizing releases it.
/// Returns false when the drawable area is unavailable or needs an explicit surface resize.
/// Browser hosts use their event loop for frame pacing, so this only checks readiness there.
public bool waitForSurface<let format : int>(Surface<format> surface)
{
    return hgbc_asm
    {
    result:
        $$bool = OpWaitForSurface $surface;
    };
}

/// Acquires the raw surface image, reusing a preceding waitForSurface reservation if present.
/// Acquisition never resizes the swapchain and does not require a preceding wait.
public RenderTarget2D<float4, format> acquireSurfaceTexture<let format : int>(
    Surface<format> surface)
{
    return hgbc_asm
    {
    result:
        $$RenderTarget2D<float4, format> = OpAcquireSurfaceTexture $surface;
    };
}

/// Returns false if the surface is unavailable and needs an explicit resize before retrying.
/// The output is empty on failure. Release all aliases to it before resizing the surface.
public bool tryAcquireSurfaceTexture<let format : int>(
    Surface<format> surface,
    out RenderTarget2D<float4, format> texture)
{
    return hgbc_asm
    {
    result:
        $$bool = OpTryAcquireSurfaceTexture $surface & texture;
    };
}

public void presentSurface<let format : int>(Surface<format> surface)
{
    hgbc_asm
    {
        OpPresentSurface $surface;
    };
}

/// Presents the acquired image, returning false if a concurrent resize discards the frame.
/// A discarded frame needs an explicit surface resize before acquiring again.
public bool tryPresentSurface<let format : int>(Surface<format> surface)
{
    return hgbc_asm
    {
    result:
        $$bool = OpTryPresentSurface $surface;
    };
}
