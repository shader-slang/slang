// render-d3d12.cpp
#include "render-d3d12.h"

#include "options.h"
#include "render.h"

// In order to use the Slang API, we need to include its header

#include <slang.h>

// We will be rendering with Direct3D 12, so we need to include
// the Windows and D3D12 headers

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include <dxgi1_4.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#include "../../source/core/slang-com-ptr.h"

#include "resource-d3d12.h"

#include "d3d-util.h"

// We will use the C standard library just for printing error messages.
#include <stdio.h>

#ifdef _MSC_VER
#include <stddef.h>
#if (_MSC_VER < 1900)
#define snprintf sprintf_s
#endif
#endif
//

#define ENABLE_DEBUG_LAYER 1

namespace renderer_test {
using namespace Slang;

class D3D12Renderer : public Renderer, public ShaderCompiler
{
public:
    // Renderer    implementation
    virtual SlangResult initialize(void* inWindowHandle) override;
    virtual void setClearColor(const float color[4]) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    virtual SlangResult captureScreenShot(const char* outputPath) override;
    virtual void serializeOutput(BindingState* state, const char* fileName) override;
    virtual Buffer* createBuffer(const BufferDesc& desc) override;
    virtual InputLayout* createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) override;
    virtual BindingState * createBindingState(const ShaderInputLayout& layout) override;
    virtual ShaderCompiler* getShaderCompiler() override;
    virtual void* map(Buffer* buffer, MapFlavor flavor) override;
    virtual void unmap(Buffer* buffer) override;
    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual void setBindingState(BindingState* state);
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt* strides, const UInt* offsets) override;
    virtual void setShaderProgram(ShaderProgram* inProgram) override;
    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt* offsets) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;

    // ShaderCompiler implementation
    virtual ShaderProgram* compileProgram(const ShaderCompileRequest& request) override;
    
	~D3D12Renderer();

protected:
	static const Int kMaxNumRenderFrames = 4;
	static const Int kMaxNumRenderTargets = 3;
	
	struct FrameInfo
	{
		FrameInfo() :m_fenceValue(0) {}
		void reset()
		{
			m_commandAllocator.setNull();
		}
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;			///< The command allocator for this frame
		UINT64 m_fenceValue;										///< The fence value when rendering this Frame is complete
	};

	class ShaderProgramImpl: public ShaderProgram
	{
		public:
		List<uint8_t> m_vertexShader;
		List<uint8_t> m_pixelShader;
		List<uint8_t> m_computeShader;
	};
	class BufferImpl: public Buffer
	{
		public:
		BufferImpl(const BufferDesc& desc):
			m_desc(desc),
			m_mapFlavor(MapFlavor::HostRead)
		{
		}
		D3D12Resource m_resource;
		D3D12Resource m_uploadResource;
		BufferDesc m_desc;
		List<uint8_t> m_memory;
		MapFlavor m_mapFlavor;
	};
	class InputLayoutImpl: public InputLayout
	{
		public:
		List<D3D12_INPUT_ELEMENT_DESC> m_elements;
	};

	static PROC loadProc(HMODULE module, char const* name);
	Result createFrameResources();
		/// Blocks until gpu has completed all work
	void waitForGpu();
	void releaseFrameResources();

	List<RefPtr<BufferImpl> > m_boundVertexBuffers;
	List<RefPtr<BufferImpl> > m_boundConstantBuffers;

	RefPtr<ShaderProgramImpl> m_boundShaderProgram;
	RefPtr<InputLayoutImpl> m_boundInputLayout;

	DXGI_FORMAT m_targetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	bool m_hasVsync = true;
	bool m_isFullSpeed = false;
	bool m_allowFullScreen = false;
	bool m_isMultiSampled = false;
	int m_numTargetSamples = 1;								///< The number of multi sample samples
	int m_targetSampleQuality = 0;							///< The multi sample quality

	int m_windowWidth = 0;
	int m_windowHeight = 0;

	bool m_isInitialized = false;

	D3D_PRIMITIVE_TOPOLOGY m_primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    float m_clearColor[4] = { 0, 0, 0, 0 };

	D3D12_VIEWPORT m_viewport = {};

	ComPtr<ID3D12Debug> m_dxDebug;

	ComPtr<ID3D12Device> m_device;
	ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	D3D12_RECT m_scissorRect = {};

	UINT m_rtvDescriptorSize = 0;

	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	UINT m_dsvDescriptorSize = 0;

	// Synchronization objects.
	D3D12CounterFence m_fence;

	HANDLE m_swapChainWaitableObject;

	// Frame specific data
	int m_numRenderFrames = 0;
	UINT m_frameIndex = 0;
	FrameInfo m_frameInfos[kMaxNumRenderFrames];

	int m_numRenderTargets = 2;
	int m_renderTargetIndex = 0;

	D3D12Resource* m_backBuffers[kMaxNumRenderTargets];
	D3D12Resource* m_renderTargets[kMaxNumRenderTargets];

	D3D12Resource m_backBufferResources[kMaxNumRenderTargets];
	D3D12Resource m_renderTargetResources[kMaxNumRenderTargets];

	D3D12Resource m_depthStencil;
	D3D12_CPU_DESCRIPTOR_HANDLE m_depthStencilView;

	int32_t m_depthStencilUsageFlags = 0;	///< D3DUtil::UsageFlag combination for depth stencil
	int32_t m_targetUsageFlags = 0;			///< D3DUtil::UsageFlag combination for target

	HWND m_hwnd = nullptr;
};

Renderer* createD3D12Renderer()
{
	return new D3D12Renderer;
}

/* static */PROC D3D12Renderer::loadProc(HMODULE module, char const* name)
{
    PROC proc = ::GetProcAddress(module, name);
    if (!proc)
    {
        fprintf(stderr, "error: failed load symbol '%s'\n", name);
        return nullptr;
    }
    return proc;
}

void D3D12Renderer::releaseFrameResources()
{
	// https://msdn.microsoft.com/en-us/library/windows/desktop/bb174577%28v=vs.85%29.aspx

	// Release the resources holding references to the swap chain (requirement of
	// IDXGISwapChain::ResizeBuffers) and reset the frame fence values to the
	// current fence value.
	for (int i = 0; i < m_numRenderFrames; i++)
	{
		FrameInfo& info = m_frameInfos[i];
		info.reset();
		info.m_fenceValue = m_fence.getCurrentValue();
	}
	for (int i = 0; i < m_numRenderTargets; i++)
	{
		m_backBuffers[i]->setResourceNull();
		m_renderTargets[i]->setResourceNull();
	}
}

void D3D12Renderer::waitForGpu()
{
	m_fence.nextSignalAndWait(m_commandQueue);
}

D3D12Renderer::~D3D12Renderer()
{
	if (m_isInitialized)
	{
		// Ensure that the GPU is no longer referencing resources that are about to be
		// cleaned up by the destructor.
		waitForGpu();
	}
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult D3D12Renderer::initialize(void* inWindowHandle)
{
	m_hwnd = (HWND)inWindowHandle;
    // Rather than statically link against D3D, we load it dynamically.

    HMODULE d3dModule = LoadLibraryA("d3d12.dll");
    if (!d3dModule)
    {
        fprintf(stderr, "error: failed load 'd3d12.dll'\n");
        return SLANG_FAIL;
    }

	HMODULE dxgiModule = LoadLibraryA("Dxgi.dll");
	if (!dxgiModule)
	{
		fprintf(stderr, "error: failed load 'dxgi.dll'\n");
		return SLANG_FAIL;
	}


#define LOAD_D3D_PROC(TYPE, NAME) \
        TYPE NAME##_ = (TYPE) loadProc(d3dModule, #NAME); 
#define LOAD_DXGI_PROC(TYPE, NAME) \
        TYPE NAME##_ = (TYPE) loadProc(dxgiModule, #NAME); 

    UINT dxgiFactoryFlags = 0;

#if ENABLE_DEBUG_LAYER
	{
		LOAD_D3D_PROC(PFN_D3D12_GET_DEBUG_INTERFACE, D3D12GetDebugInterface);
		if (D3D12GetDebugInterface_)
		{
			if (SUCCEEDED(D3D12GetDebugInterface_(IID_PPV_ARGS(m_dxDebug.writeRef()))))
			{
				m_dxDebug->EnableDebugLayer();
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
	}
#endif

	// Try and create DXGIFactory
	ComPtr<IDXGIFactory4> dxgiFactory;
	{
		typedef HRESULT(WINAPI *PFN_DXGI_CREATE_FACTORY_2)(UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory);
		LOAD_DXGI_PROC(PFN_DXGI_CREATE_FACTORY_2, CreateDXGIFactory2);
		if (!CreateDXGIFactory2_)
		{
			return SLANG_FAIL;
		}
		SLANG_RETURN_ON_FAIL(CreateDXGIFactory2_(dxgiFactoryFlags, IID_PPV_ARGS(dxgiFactory.writeRef())));
	}
    
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

    // Search for an adapter that meets our requirements
    ComPtr<IDXGIAdapter> adapter;
    
    LOAD_D3D_PROC(PFN_D3D12_CREATE_DEVICE, D3D12CreateDevice);
	if (!D3D12CreateDevice_)
	{
		return SLANG_FAIL;
	}

    UINT adapterCounter = 0;
    for (;;)
    {
        UINT adapterIndex = adapterCounter++;

        ComPtr<IDXGIAdapter1> candidateAdapter;
        if (dxgiFactory->EnumAdapters1(adapterIndex, candidateAdapter.writeRef()) == DXGI_ERROR_NOT_FOUND)
            break;

        DXGI_ADAPTER_DESC1 desc;
        candidateAdapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            // TODO: may want to allow software driver as fallback
        }
        else if (SUCCEEDED(D3D12CreateDevice_(candidateAdapter, featureLevel, IID_PPV_ARGS(m_device.writeRef()))))
        {
            // We found one!
            adapter = candidateAdapter;
            break;
        }
    }

    if (!adapter)
    {
        // Couldn't find an adapter
        return SLANG_FAIL;
    }
	
	m_numRenderFrames = 3;
	m_numRenderTargets = 2;
	
	m_windowWidth = gWindowWidth;
	m_windowHeight = gWindowHeight;

	// set viewport
	{
		m_viewport.Width = float(m_windowWidth);
		m_viewport.Height = float(m_windowHeight);
		m_viewport.MinDepth = 0;
		m_viewport.MaxDepth = 1;
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
	}

	{
		m_scissorRect.left = 0;
		m_scissorRect.top = 0;
		m_scissorRect.right = m_windowWidth;
		m_scissorRect.bottom = m_windowHeight;
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	SLANG_RETURN_ON_FAIL(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_commandQueue.writeRef())));

	// Describe the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = m_numRenderTargets;
	swapChainDesc.BufferDesc.Width = m_windowWidth;
	swapChainDesc.BufferDesc.Height = m_windowHeight;
	swapChainDesc.BufferDesc.Format = m_targetFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = m_hwnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;

	if (m_isFullSpeed)
	{
		m_hasVsync = false;
		m_allowFullScreen = false;
	}

	if (!m_hasVsync)
	{
		swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
	}

	// Swap chain needs the queue so that it can force a flush on it.
	ComPtr<IDXGISwapChain> swapChain;
	SLANG_RETURN_ON_FAIL(dxgiFactory->CreateSwapChain(m_commandQueue, &swapChainDesc, swapChain.writeRef()));
	SLANG_RETURN_ON_FAIL(swapChain->QueryInterface(m_swapChain.writeRef()));

	if (!m_hasVsync)
	{
		m_swapChainWaitableObject = m_swapChain->GetFrameLatencyWaitableObject();

		int maxLatency = m_numRenderTargets - 2;

		// Make sure the maximum latency is in the range required by dx12 runtime
		maxLatency = (maxLatency < 1) ? 1 : maxLatency;
		maxLatency = (maxLatency > DXGI_MAX_SWAP_CHAIN_BUFFERS) ? DXGI_MAX_SWAP_CHAIN_BUFFERS : maxLatency;

		m_swapChain->SetMaximumFrameLatency(maxLatency);
	}

	// This sample does not support fullscreen transitions.
	SLANG_RETURN_ON_FAIL(dxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

	m_renderTargetIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};

		rtvHeapDesc.NumDescriptors = m_numRenderTargets;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		SLANG_RETURN_ON_FAIL(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.writeRef())));
		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	{
		// Describe and create a depth stencil view (DSV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		SLANG_RETURN_ON_FAIL(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.writeRef())));

		m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	// Setup frame resources
	{
		SLANG_RETURN_ON_FAIL(createFrameResources());
	}

	// Setup fence, and close the command list (as default state without begin/endRender is closed)
	{
		SLANG_RETURN_ON_FAIL(m_fence.init(m_device));
		// Create the command list. When command lists are created they are open, so close it.
		FrameInfo& frame = m_frameInfos[m_frameIndex];
		SLANG_RETURN_ON_FAIL(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.m_commandAllocator, nullptr, IID_PPV_ARGS(m_commandList.writeRef())));
		m_commandList->Close();
	}

	m_isInitialized = true;
    return SLANG_OK;
}

Result D3D12Renderer::createFrameResources()
{
	// Create back buffers
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvStart(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV, and a command allocator for each frame.
		for (int i = 0; i < m_numRenderTargets; i++)
		{
			// Get the back buffer
			ComPtr<ID3D12Resource> backBuffer;
			SLANG_RETURN_ON_FAIL(m_swapChain->GetBuffer(UINT(i), IID_PPV_ARGS(backBuffer.writeRef())));

			// Set up resource for back buffer
			m_backBufferResources[i].setResource(backBuffer, D3D12_RESOURCE_STATE_COMMON);
			m_backBuffers[i] = &m_backBufferResources[i];
			// Assume they are the same thing for now...
			m_renderTargets[i] = &m_backBufferResources[i];

			// If we are multi-sampling - create a render target separate from the back buffer
			if (m_isMultiSampled)
			{
				D3D12_HEAP_PROPERTIES heapProps;
				heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;

				D3D12_RESOURCE_DESC desc = backBuffer->GetDesc();

				DXGI_FORMAT resourceFormat = D3DUtil::calcResourceFormat(D3DUtil::USAGE_TARGET, m_targetUsageFlags, desc.Format);
				DXGI_FORMAT targetFormat = D3DUtil::calcFormat(D3DUtil::USAGE_TARGET, resourceFormat);

				// Set the target format
				m_targetFormat = targetFormat;

				D3D12_CLEAR_VALUE clearValue = {};
				clearValue.Format = targetFormat;

				// Don't know targets alignment, so just memory copy
				::memcpy(clearValue.Color, m_clearColor, sizeof(m_clearColor));

				desc.Format = resourceFormat;
				desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				desc.SampleDesc.Count = m_numTargetSamples;
				desc.SampleDesc.Quality = m_targetSampleQuality; 
				desc.Alignment = 0;

				SLANG_RETURN_ON_FAIL(m_renderTargetResources[i].initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue));
				m_renderTargets[i] = &m_renderTargetResources[i];
			}

			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { rtvStart.ptr + i * m_rtvDescriptorSize };
			m_device->CreateRenderTargetView(*m_renderTargets[i], nullptr, rtvHandle);
		}
	}

	// Set up frames
	for (int i = 0; i < m_numRenderFrames; i++)
	{
		FrameInfo& frame = m_frameInfos[i];
		SLANG_RETURN_ON_FAIL(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(frame.m_commandAllocator.writeRef())));
	}

	{
		D3D12_RESOURCE_DESC desc = m_backBuffers[0]->getResource()->GetDesc();
		assert(desc.Width == UINT64(m_windowWidth) && desc.Height == UINT64(m_windowHeight));
	}

	// Create the depth stencil view.
	{
		D3D12_HEAP_PROPERTIES heapProps;
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		DXGI_FORMAT resourceFormat = D3DUtil::calcResourceFormat(D3DUtil::USAGE_DEPTH_STENCIL, m_depthStencilUsageFlags, m_depthStencilFormat);
		DXGI_FORMAT depthStencilFormat = D3DUtil::calcFormat(D3DUtil::USAGE_DEPTH_STENCIL, resourceFormat);

		// Set the depth stencil format
		m_depthStencilFormat = depthStencilFormat;

		// Setup default clear
		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = depthStencilFormat;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Format = resourceFormat;
		resourceDesc.Width = m_windowWidth;
		resourceDesc.Height = m_windowHeight;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc.Count = m_numTargetSamples;
		resourceDesc.SampleDesc.Quality = m_targetSampleQuality;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		resourceDesc.Alignment = 0;

		SLANG_RETURN_ON_FAIL(m_depthStencil.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue));

		// Set the depth stencil
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = depthStencilFormat;
		depthStencilDesc.ViewDimension = m_isMultiSampled ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		// Set up as the depth stencil view
		m_device->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		m_depthStencilView = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	m_viewport.Width = static_cast<float>(m_windowWidth);
	m_viewport.Height = static_cast<float>(m_windowHeight);
	m_viewport.MaxDepth = 1.0f;

	m_scissorRect.right = static_cast<LONG>(m_windowWidth);
	m_scissorRect.bottom = static_cast<LONG>(m_windowHeight);

	return SLANG_OK;
}

void D3D12Renderer::setClearColor(const float color[4])
{
    memcpy(m_clearColor, color, sizeof(m_clearColor));
}

void D3D12Renderer::clearFrame()
{
	// Record commands
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { m_rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr + m_renderTargetIndex * m_rtvDescriptorSize };
	m_commandList->ClearRenderTargetView(rtvHandle, m_clearColor, 0, nullptr);
	if (m_depthStencil)
	{
		m_commandList->ClearDepthStencilView(m_depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
}

void D3D12Renderer::presentFrame()
{
	if (m_swapChainWaitableObject)
	{
		// check if now is good time to present
		// This doesn't wait - because the wait time is 0. If it returns WAIT_TIMEOUT it means that no frame is waiting to be be displayed
		// so there is no point doing a present.
		const bool shouldPresent = (WaitForSingleObjectEx(m_swapChainWaitableObject, 0, TRUE) != WAIT_TIMEOUT);
		if (shouldPresent)
		{
			m_swapChain->Present(0, 0);
		}
	}
	else
	{
		SLANG_ASSERT_VOID_ON_FAIL(m_swapChain->Present(1, 0));
	}

	// Increment the fence value. Save on the frame - we'll know that frame is done when the fence value >= 
	m_frameInfos[m_frameIndex].m_fenceValue = m_fence.nextSignal(m_commandQueue);

	// increment frame index after signal
	m_frameIndex = (m_frameIndex + 1) % m_numRenderFrames;
	// Update the render target index.
	m_renderTargetIndex = m_swapChain->GetCurrentBackBufferIndex();

	// On the current frame wait until it is completed 
	{
		FrameInfo& frame = m_frameInfos[m_frameIndex];
		// If the next frame is not ready to be rendered yet, wait until it is ready.
		m_fence.waitUntilCompleted(frame.m_fenceValue);
	}
}

SlangResult D3D12Renderer::captureScreenShot(const char* outputPath)
{
    return SLANG_FAIL;
}

ShaderCompiler* D3D12Renderer::getShaderCompiler() 
{
    return this;
}

Buffer* D3D12Renderer::createBuffer(const BufferDesc& desc)
{
	RefPtr<BufferImpl> buffer(new BufferImpl(desc));
	const size_t bufferSize = desc.size;

	switch (desc.flavor)
	{
		case BufferFlavor::Constant:
		{
			// Assume the constant buffer will change every frame. We'll just keep a copy of the contents 
			// in regular memory until it needed 
			buffer->m_memory.SetSize(UInt(bufferSize));
			break;	 
		}
		case BufferFlavor::Vertex:
		{
			D3D12_RESOURCE_DESC resourceDesc{};
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = bufferSize;
			resourceDesc.Height = 1;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			{
				D3D12_HEAP_PROPERTIES heapProps;
				heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;
				
				SLANG_RETURN_NULL_ON_FAIL(buffer->m_resource.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr));
			}

			{
				D3D12_HEAP_PROPERTIES heapProps;
				heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
				heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProps.CreationNodeMask = 1;
				heapProps.VisibleNodeMask = 1;

				SLANG_RETURN_NULL_ON_FAIL(buffer->m_uploadResource.initCommitted(m_device, heapProps, D3D12_HEAP_FLAG_NONE, resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr));
			}

			if (desc.initData)
			{
				// Copy data to the intermediate upload heap and then schedule a copy 
				// from the upload heap to the vertex buffer.
				UINT8* vertexData;
				D3D12_RANGE readRange = {}; 		// We do not intend to read from this resource on the CPU.
				
				ID3D12Resource* uploadResource = buffer->m_uploadResource; 
				ID3D12Resource* resource = buffer->m_resource;  
				
				SLANG_RETURN_NULL_ON_FAIL(uploadResource->Map(0, &readRange, reinterpret_cast<void**>(&vertexData)));
				::memcpy(vertexData, desc.initData, bufferSize);
				uploadResource->Unmap(0, nullptr);

				m_commandList->CopyBufferRegion(resource, 0, uploadResource, 0, bufferSize);
			}
			// Make sure it's in the right state
			{
				D3D12BarrierSubmitter submitter(m_commandList);
				buffer->m_resource.transition(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, submitter);
			}
			break;
		}
		default:
			return nullptr;
	}
	
	return buffer.detach();
}

InputLayout* D3D12Renderer::createInputLayout(const InputElementDesc* inputElements, UInt inputElementCount) 
{
	RefPtr<InputLayoutImpl> layout(new InputLayoutImpl);

	List<D3D12_INPUT_ELEMENT_DESC>& elements = layout->m_elements;
	elements.SetSize(inputElementCount);

	for (UInt i = 0; i < inputElementCount; ++i)
	{
		const InputElementDesc& srcEle = inputElements[i];
		D3D12_INPUT_ELEMENT_DESC& dstEle = elements[i];

		// JS: TODO - we need to store the semantic names somewhere... As they will be lost outside
		dstEle.SemanticName = srcEle.semanticName;
		dstEle.SemanticIndex = (UINT)srcEle.semanticIndex;
		dstEle.Format = D3DUtil::getMapFormat(srcEle.format);
		dstEle.InputSlot = 0;
		dstEle.AlignedByteOffset = (UINT)srcEle.offset;
		dstEle.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		dstEle.InstanceDataStepRate = 0;
	}

    return layout.detach();
}

void* D3D12Renderer::map(Buffer* buffer, MapFlavor flavor) 
{
	BufferImpl* impl = static_cast<BufferImpl*>(buffer);
	impl->m_mapFlavor = flavor;

	switch (impl->m_desc.flavor)
	{
		case BufferFlavor::Vertex:
		{
			D3D12_RANGE readRange = {}; 		// We do not intend to read from this resource on the CPU.

			// We need this in a state so we can upload
			switch (flavor)
			{
				case MapFlavor::HostWrite:
				case MapFlavor::WriteDiscard:
				{
					D3D12BarrierSubmitter submitter(m_commandList);
					impl->m_uploadResource.transition(D3D12_RESOURCE_STATE_GENERIC_READ, submitter);
					impl->m_resource.transition(D3D12_RESOURCE_STATE_COPY_DEST, submitter);
					break;
				}
				case MapFlavor::HostRead: 
				{
					// Lock whole of the buffer
					readRange.End = impl->m_desc.size;
					break;
				}
			}
			
			// Lock it
			void* uploadData;
			SLANG_RETURN_NULL_ON_FAIL(impl->m_uploadResource.getResource()->Map(0, &readRange, reinterpret_cast<void**>(&uploadData)));
			return uploadData;
		}
		case BufferFlavor::Constant:
		{
			return impl->m_memory.Buffer();
		}
	}

    return nullptr;
}

void D3D12Renderer::unmap(Buffer* buffer)
{
	BufferImpl* impl = static_cast<BufferImpl*>(buffer);

	switch (impl->m_desc.flavor)
	{
		case BufferFlavor::Vertex:
		{
			// Unmap
			ID3D12Resource* uploadResource = impl->m_uploadResource;
			ID3D12Resource* resource = impl->m_resource;

			uploadResource->Unmap(0, nullptr);

			// We need this in a state so we can upload
			switch (impl->m_mapFlavor)
			{
				case MapFlavor::HostWrite:
				case MapFlavor::WriteDiscard:
				{
					{
						D3D12BarrierSubmitter submitter(m_commandList);
						impl->m_uploadResource.transition(D3D12_RESOURCE_STATE_GENERIC_READ, submitter);
						impl->m_resource.transition(D3D12_RESOURCE_STATE_COPY_DEST, submitter);
					}

					m_commandList->CopyBufferRegion(resource, 0, uploadResource, 0, impl->m_desc.size);

					{
						D3D12BarrierSubmitter submitter(m_commandList);
						impl->m_resource.transition(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, submitter);
					}
					
					break;
				}
				case MapFlavor::HostRead: break;
			}
			break;
		}
		case BufferFlavor::Constant: 
		{
			break;
		}
	}
}

void D3D12Renderer::setInputLayout(InputLayout* inputLayout) 
{
	m_boundInputLayout = static_cast<InputLayoutImpl*>(inputLayout);
}

void D3D12Renderer::setPrimitiveTopology(PrimitiveTopology topology) 
{
	m_primitiveTopology = D3DUtil::getPrimitiveTopology(topology);
	assert(m_primitiveTopology != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
}

void D3D12Renderer::setVertexBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt * strides, const UInt* offsets)
{
	{
		const UInt num = startSlot + slotCount;
		if (num > m_boundVertexBuffers.Count())
		{
			m_boundVertexBuffers.SetSize(num);
		}
	}

	for (UInt i = 0; i < slotCount; i++)
	{
		BufferImpl* buffer = static_cast<BufferImpl*>(buffers[i]);
		if (buffer)
		{
			assert(buffer->m_desc.flavor == BufferFlavor::Vertex);
		}
		m_boundVertexBuffers[startSlot + i] = buffer;
	}
}

void D3D12Renderer::setShaderProgram(ShaderProgram* inProgram)
{
	m_boundShaderProgram = static_cast<ShaderProgramImpl*>(inProgram);
}

void D3D12Renderer::setConstantBuffers(UInt startSlot, UInt slotCount, Buffer*const* buffers, const UInt* offsets)
{
	{
		const UInt num = startSlot + slotCount;
		if (num > m_boundConstantBuffers.Count())
		{
			m_boundConstantBuffers.SetSize(num);
		}
	}

	for (UInt i = 0; i < slotCount; i++)
	{
		BufferImpl* buffer = static_cast<BufferImpl*>(buffers[i]);
		if (buffer)
		{
			assert(buffer->m_desc.flavor == BufferFlavor::Constant);
		}
		m_boundConstantBuffers[startSlot + i] = buffer;
	}
}

void D3D12Renderer::draw(UInt vertexCount, UInt startVertex)
{
	ID3D12GraphicsCommandList* commandList = m_commandList;
	
	//commandList->SetGraphicsRootSignature(m_rootSignature);
	///commandList->SetPipelineState(m_pipelineState);

	commandList->IASetPrimitiveTopology(m_primitiveTopology);
	//commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->DrawInstanced(UINT(vertexCount), 1, UINT(startVertex), 0);
}

void D3D12Renderer::dispatchCompute(int x, int y, int z)
{
	ID3D12GraphicsCommandList* commandList = m_commandList;

	//commandList->SetComputeRootSignature(m_rootSignature);
	//commandList->SetDescriptorHeaps(SLANG_COUNT_OF(heaps), heaps);

	//commandList->SetPipelineState(m_pipelineState);

	//commandList->SetComputeRootConstantBufferView(0, cbCursor.getGpuHandle());

	//commandList->SetComputeRootDescriptorTable(1, viewHandles[0]);
	//commandList->SetComputeRootDescriptorTable(2, viewHandles[1]);
	//commandList->SetComputeRootDescriptorTable(3, glob->m_samplerHeap.getGpuStart());

	commandList->Dispatch(x, y, z);
}

BindingState* D3D12Renderer::createBindingState(const ShaderInputLayout& layout)
{
    return nullptr;
}

void D3D12Renderer::setBindingState(BindingState* state)
{
}

void D3D12Renderer::serializeOutput(BindingState* state, const char* fileName)
{
}

// ShaderCompiler interface

ShaderProgram* D3D12Renderer::compileProgram(const ShaderCompileRequest& request)
{
	RefPtr<ShaderProgramImpl> program(new ShaderProgramImpl);

	if (request.computeShader.name)
	{
		ComPtr<ID3DBlob> computeShaderBlob;
		SLANG_RETURN_NULL_ON_FAIL(D3DUtil::compileHLSLShader(request.computeShader.source.path, request.computeShader.source.dataBegin, request.computeShader.name, request.computeShader.profile, computeShaderBlob));

		program->m_computeShader.InsertRange(0, (const uint8_t*)computeShaderBlob->GetBufferPointer(), UInt(computeShaderBlob->GetBufferSize()));
	}
	else
	{
		ComPtr<ID3DBlob> vertexShaderBlob, fragmentShaderBlob;
		SLANG_RETURN_NULL_ON_FAIL(D3DUtil::compileHLSLShader(request.vertexShader.source.path, request.vertexShader.source.dataBegin, request.vertexShader.name, request.vertexShader.profile, vertexShaderBlob));
		SLANG_RETURN_NULL_ON_FAIL(D3DUtil::compileHLSLShader(request.fragmentShader.source.path, request.fragmentShader.source.dataBegin, request.fragmentShader.name, request.fragmentShader.profile, fragmentShaderBlob));

		program->m_vertexShader.InsertRange(0, (const uint8_t*)vertexShaderBlob->GetBufferPointer(), UInt(vertexShaderBlob->GetBufferSize()));
		program->m_pixelShader.InsertRange(0, (const uint8_t*)fragmentShaderBlob->GetBufferPointer(), UInt(fragmentShaderBlob->GetBufferSize()));
	}

	return program.detach();
}


} // renderer_test
