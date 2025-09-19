#include <algorithm>
#include "slang-test-device-cache.h"

// Static member accessor functions (Meyer's singleton pattern)
// This ensures proper destruction order - function-local statics are destroyed
// in reverse order of first access, avoiding the static destruction order fiasco
std::mutex& DeviceCache::getMutex()
{
    static std::mutex instance;
    return instance;
}

std::unordered_map<DeviceCache::DeviceCacheKey, DeviceCache::CachedDevice, DeviceCache::DeviceCacheKeyHash>& DeviceCache::getDeviceCache()
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
    return deviceType == other.deviceType &&
           enableValidation == other.enableValidation &&
           enableRayTracingValidation == other.enableRayTracingValidation &&
           profileName == other.profileName &&
           requiredFeatures == other.requiredFeatures;
}

std::size_t DeviceCache::DeviceCacheKeyHash::operator()(const DeviceCacheKey& key) const
{
    std::size_t h1 = std::hash<int>{}(static_cast<int>(key.deviceType));
    std::size_t h2 = std::hash<bool>{}(key.enableValidation);
    std::size_t h3 = std::hash<bool>{}(key.enableRayTracingValidation);
    std::size_t h4 = std::hash<std::string>{}(key.profileName);
    
    std::size_t h5 = 0;
    for (const auto& feature : key.requiredFeatures)
    {
        h5 ^= std::hash<std::string>{}(feature) + 0x9e3779b9 + (h5 << 6) + (h5 >> 2);
    }
    
    return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
}

DeviceCache::CachedDevice::CachedDevice() 
    : refCount(0), creationOrder(0)
{
}

void DeviceCache::evictOldestDeviceIfNeeded()
{
    auto& deviceCache = getDeviceCache();
    if (deviceCache.size() < MAX_CACHED_DEVICES)
        return;
        
    // Find the oldest device that has only one reference (from our cache)
    auto oldestIt = deviceCache.end();
    uint64_t oldestCreationOrder = UINT64_MAX;
    
    for (auto it = deviceCache.begin(); it != deviceCache.end(); ++it)
    {
        // Check if device is not currently in use by any CachedDeviceWrapper
        // If refCount is 0, it means no active users and is safe to evict
        if (it->second.refCount == 0 && it->second.creationOrder < oldestCreationOrder)
        {
            oldestCreationOrder = it->second.creationOrder;
            oldestIt = it;
        }
    }
    
    // Remove the oldest truly unused device
    if (oldestIt != deviceCache.end())
    {
        deviceCache.erase(oldestIt);
    }
}

Slang::ComPtr<rhi::IDevice> DeviceCache::acquireDevice(const rhi::DeviceDesc& desc)
{
    // Only cache Vulkan devices to avoid the Tegra driver issue
    // if (desc.deviceType != rhi::DeviceType::Vulkan)
    // {
    //     Slang::ComPtr<rhi::IDevice> device;
    //     auto result = rhi::getRHI()->createDevice(desc, device.writeRef());
    //     if (SLANG_SUCCEEDED(result))
    //         return device;
    //     return nullptr;
    // }
    
    std::lock_guard<std::mutex> lock(getMutex());
    auto& deviceCache = getDeviceCache();
    auto& nextCreationOrder = getNextCreationOrder();
    
    // Create cache key
    DeviceCacheKey key;
    key.deviceType = desc.deviceType;
    key.enableValidation = desc.enableValidation;
    key.enableRayTracingValidation = desc.enableRayTracingValidation;
    key.profileName = desc.slang.targetProfile ? desc.slang.targetProfile : "";
    
    // Add required features to key
    for (int i = 0; i < desc.requiredFeatureCount; ++i)
    {
        key.requiredFeatures.push_back(desc.requiredFeatures[i]);
    }
    std::sort(key.requiredFeatures.begin(), key.requiredFeatures.end());
    
    // Evict oldest device if we've reached the limit
    evictOldestDeviceIfNeeded();
    
    // Check if we have a cached device
    auto it = deviceCache.find(key);
    if (it != deviceCache.end())
    {
        it->second.refCount++;
        return it->second.device;
    }
    
    // Create new device
    Slang::ComPtr<rhi::IDevice> device;
    auto result = rhi::getRHI()->createDevice(desc, device.writeRef());
    if (SLANG_FAILED(result))
    {
        return nullptr;
    }
    
    // Cache the device
    CachedDevice& cached = deviceCache[key];
    cached.device = device;
    cached.refCount = 1;
    cached.creationOrder = nextCreationOrder++;
    
    return device;
}

void DeviceCache::releaseDevice(rhi::IDevice* device)
{
    if (!device)
        return;
        
    // // Only manage Vulkan devices
    // if (device->getDeviceType() != rhi::DeviceType::Vulkan)
    //     return;
        
    std::lock_guard<std::mutex> lock(getMutex());
    auto& deviceCache = getDeviceCache();
    
    // Find the device in cache and decrement ref count
    for (auto& pair : deviceCache)
    {
        if (pair.second.device.get() == device)
        {
            pair.second.refCount--;
            break;
        }
    }
}

void DeviceCache::cleanCache()
{
    std::lock_guard<std::mutex> lock(getMutex());
    auto& deviceCache = getDeviceCache();
    deviceCache.clear();
}
