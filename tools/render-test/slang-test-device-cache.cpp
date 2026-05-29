#include "slang-test-device-cache.h"

#include <algorithm>

// Static member accessor functions (Meyer's singleton pattern)
// This ensures proper destruction order - function-local statics are destroyed
// in reverse order of first access, avoiding the static destruction order fiasco
std::mutex& DeviceCache::getMutex()
{
    static std::mutex instance;
    return instance;
}

std::unordered_map<
    DeviceCache::DeviceCacheKey,
    DeviceCache::CachedDevice,
    DeviceCache::DeviceCacheKeyHash>&
DeviceCache::getDeviceCache()
{
    static std::unordered_map<DeviceCacheKey, CachedDevice, DeviceCacheKeyHash> instance;
    return instance;
}

uint64_t& DeviceCache::getNextCreationOrder()
{
    static uint64_t instance = 0;
    return instance;
}

bool DeviceCache::DeviceCacheKey::operator==(const DeviceCacheKey& other) const
{
    return deviceType == other.deviceType && enableValidation == other.enableValidation &&
           enableRayTracingValidation == other.enableRayTracingValidation &&
           profileName == other.profileName && targetFlags == other.targetFlags &&
           defaultMatrixLayoutMode == other.defaultMatrixLayoutMode &&
           nvapiExtUavSlot == other.nvapiExtUavSlot &&
           dx12ExperimentalFeatures == other.dx12ExperimentalFeatures;
}

std::size_t DeviceCache::DeviceCacheKeyHash::operator()(const DeviceCacheKey& key) const
{
    std::size_t h1 = std::hash<int>{}(static_cast<int>(key.deviceType));
    std::size_t h2 = std::hash<bool>{}(key.enableValidation);
    std::size_t h3 = std::hash<bool>{}(key.enableRayTracingValidation);
    std::size_t h4 = std::hash<std::string>{}(key.profileName);
    std::size_t h5 = std::hash<unsigned int>{}(static_cast<unsigned int>(key.targetFlags));
    std::size_t h6 = std::hash<int>{}(static_cast<int>(key.defaultMatrixLayoutMode));
    std::size_t h7 = std::hash<uint32_t>{}(key.nvapiExtUavSlot);
    std::size_t h8 = std::hash<bool>{}(key.dx12ExperimentalFeatures);

    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4) ^ (h6 << 5) ^ (h7 << 6) ^ (h8 << 7);
}

DeviceCache::CachedDevice::CachedDevice()
    : creationOrder(0)
{
}

void DeviceCache::evictOldestDeviceIfNeeded()
{
    auto& deviceCache = getDeviceCache();
    if (deviceCache.size() < MAX_CACHED_DEVICES)
        return;

    // Find the oldest device to evict
    auto oldestIt = deviceCache.end();
    uint64_t oldestCreationOrder = UINT64_MAX;

    for (auto it = deviceCache.begin(); it != deviceCache.end(); ++it)
    {
        if (it->second.creationOrder < oldestCreationOrder)
        {
            oldestCreationOrder = it->second.creationOrder;
            oldestIt = it;
        }
    }

    // Remove the oldest device - ComPtr will handle the actual device release
    if (oldestIt != deviceCache.end())
    {
        deviceCache.erase(oldestIt);
    }
}

SlangResult DeviceCache::acquireDevice(const rhi::DeviceDesc& desc, rhi::IDevice** outDevice)
{
    if (!outDevice)
        return SLANG_E_INVALID_ARG;

    *outDevice = nullptr;

    // Skip caching for CUDA devices due to crashes
    if (desc.deviceType == rhi::DeviceType::CUDA)
    {
        return rhi::getRHI()->createDevice(desc, outDevice);
    }

    std::lock_guard<std::mutex> lock(getMutex());
    auto& deviceCache = getDeviceCache();
    auto& nextCreationOrder = getNextCreationOrder();

    // Create cache key
    DeviceCacheKey key;
    key.deviceType = desc.deviceType;
    key.enableValidation = desc.enableValidation;
    key.enableRayTracingValidation = desc.enableRayTracingValidation;
    key.profileName = desc.slang.targetProfile ? desc.slang.targetProfile : "Unknown";
    key.targetFlags = desc.slang.targetFlags;
    key.defaultMatrixLayoutMode = desc.slang.defaultMatrixLayoutMode;
    key.nvapiExtUavSlot = desc.nvapiExtUavSlot;
    key.dx12ExperimentalFeatures = (desc.next != nullptr);

    // Evict oldest device if we've reached the limit
    evictOldestDeviceIfNeeded();

    // Check if we have a cached device
    auto it = deviceCache.find(key);
    if (it != deviceCache.end())
    {
        // Return the cached device - COM reference counting handles the references
        *outDevice = it->second.device.get();
        if (*outDevice)
        {
            (*outDevice)->addRef();
            return SLANG_OK;
        }
    }

    // Create new device
    Slang::ComPtr<rhi::IDevice> device;
    auto result = rhi::getRHI()->createDevice(desc, device.writeRef());
    if (SLANG_FAILED(result))
    {
        return result;
    }

    // Cache the device
    CachedDevice& cached = deviceCache[key];
    cached.device = device;
    cached.creationOrder = nextCreationOrder++;

    // Return the device with proper reference counting
    *outDevice = device.get();
    if (*outDevice)
    {
        (*outDevice)->addRef();
    }

    return SLANG_OK;
}


void DeviceCache::cleanCache()
{
    std::lock_guard<std::mutex> lock(getMutex());
    auto& deviceCache = getDeviceCache();
    deviceCache.clear();
}
