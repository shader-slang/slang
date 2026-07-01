#pragma once

#include "slang-support.h"

#include <mutex>
#include <slang-rhi.h>
#include <string>
#include <unordered_map>
#include <vector>

// Device Cache for preventing NVIDIA Tegra driver state corruption
// This cache reuses Vulkan instances and devices to avoid the VK_ERROR_INCOMPATIBLE_DRIVER
// issue that occurs after ~19 device creation/destruction cycles on Tegra platforms.
// Uses ComPtr for automatic device lifecycle management - devices are released when removed from
// cache.
class DeviceCache
{
public:
    struct DeviceCacheKey
    {
        rhi::DeviceType deviceType;
        bool enableValidation;
        bool enableRayTracingValidation;
        std::string profileName;
        SlangTargetFlags targetFlags;
        SlangMatrixLayoutMode defaultMatrixLayoutMode;
        uint32_t nvapiExtUavSlot;
        bool dx12ExperimentalFeatures;

        bool operator==(const DeviceCacheKey& other) const;
    };

    struct DeviceCacheKeyHash
    {
        std::size_t operator()(const DeviceCacheKey& key) const;
    };

    struct CachedDevice
    {
        Slang::ComPtr<rhi::IDevice> device;

        // The debug bridge this device was created with (as desc.debugCallback). A cached device is
        // wired to this exact bridge for its whole life and cannot be re-pointed, so acquireDevice
        // hands it back to every caller — including a cache hit — to bind their per-invocation
        // callback to (see #11856).
        Slang::RefPtr<renderer_test::CoreToRHIDebugBridge> bridge;

        uint64_t creationOrder;

        CachedDevice();
    };

private:
    static constexpr int MAX_CACHED_DEVICES = 10;

    // Use function-local statics to control destruction order (Meyer's singleton pattern)
    static std::mutex& getMutex();
    static std::unordered_map<DeviceCacheKey, CachedDevice, DeviceCacheKeyHash>& getDeviceCache();
    static uint64_t& getNextCreationOrder();

    static void evictOldestDeviceIfNeeded();

public:
    // Acquires a device for `desc`, reusing a cached one when possible, and returns via `outBridge`
    // the debug bridge that device is wired to. On a miss the device is created with a fresh
    // retained bridge (set as desc.debugCallback) and both are cached together; on a hit the cached
    // device and its original bridge are returned. Callers bind a ScopedCoreDebugCallback to
    // `outBridge` so a reused device routes its validation messages to the current invocation's
    // callback instead of a stale one (see #11856).
    static SlangResult acquireDevice(
        const rhi::DeviceDesc& desc,
        rhi::IDevice** outDevice,
        Slang::RefPtr<renderer_test::CoreToRHIDebugBridge>* outBridge);

    static void cleanCache();
};

// RAII wrapper for cached devices to ensure proper cleanup
class CachedDeviceWrapper
{
private:
    Slang::ComPtr<rhi::IDevice> m_device;

public:
    CachedDeviceWrapper() = default;

    CachedDeviceWrapper(Slang::ComPtr<rhi::IDevice> device)
        : m_device(device)
    {
    }

    ~CachedDeviceWrapper() {}

    // Move constructor
    CachedDeviceWrapper(CachedDeviceWrapper&& other) noexcept
        : m_device(std::move(other.m_device))
    {
    }

    // Move assignment
    CachedDeviceWrapper& operator=(CachedDeviceWrapper&& other) noexcept
    {
        if (this != &other)
        {
            m_device = std::move(other.m_device);
        }
        return *this;
    }

    // Delete copy constructor and assignment
    CachedDeviceWrapper(const CachedDeviceWrapper&) = delete;
    CachedDeviceWrapper& operator=(const CachedDeviceWrapper&) = delete;

    rhi::IDevice* get() const { return m_device.get(); }
    rhi::IDevice* operator->() const { return m_device.get(); }
    operator bool() const { return m_device != nullptr; }

    Slang::ComPtr<rhi::IDevice>& getComPtr() { return m_device; }
};
