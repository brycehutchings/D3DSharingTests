#include <iostream>
#include <Unknwn.h>
#include <winrt/base.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3d11_4.h>

using namespace winrt;

// TextureSharing.cpp
void TexturePermationSharingTests(com_ptr<ID3D11Device5> d3d11Device, com_ptr<ID3D11Device5> d3d11DeviceSecond, com_ptr<ID3D12Device> d3d12Device, com_ptr<ID3D12Device> d3d12DeviceSecond);

// FenceSharing.cpp
void Fence11To12Test(com_ptr<ID3D11Device5> d3d11Device, com_ptr<ID3D12Device> d3d12Device);
void Fence12To11Test(com_ptr<ID3D11Device5> d3d11Device, com_ptr<ID3D12Device> d3d12Device);
void Fence12To12Test(com_ptr<ID3D12Device> d3d12Device1, com_ptr<ID3D12Device> d3d12Device2);

com_ptr<IDXGIAdapter3> GetAdapter(com_ptr<IDXGIFactory4> dxgiFactory) {
	com_ptr<IDXGIAdapter1> dxgiAdapter;
	check_hresult(dxgiFactory->EnumAdapters1(0, dxgiAdapter.put()));

	com_ptr<IDXGIAdapter3> adapter3 = dxgiAdapter.as<IDXGIAdapter3>();

	DXGI_ADAPTER_DESC adapterDesc{};
	check_hresult(adapter3->GetDesc(&adapterDesc));

	printf("Testing adapter: %ws\n", adapterDesc.Description);

	return adapter3;
}

int main()
{
	check_hresult(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

	try {
		//
		// Initialize DXGI and pick an adapter.
		//
		UINT DXGIDebugFlags = 0;
#ifdef _DEBUG
		DXGIDebugFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		com_ptr<IDXGIFactory4> dxgiFactory;
		check_hresult(CreateDXGIFactory2(
			DXGIDebugFlags, __uuidof(IDXGIFactory4), dxgiFactory.put_void()));

		auto adapter = GetAdapter(dxgiFactory);

#ifdef _DEBUG
		// D3D12 debug layer must be enabled before the first D3D12 device is created.
		com_ptr<ID3D12Debug> d3d12Debug0;
		check_hresult(D3D12GetDebugInterface(IID_PPV_ARGS(&d3d12Debug0)));
		com_ptr<ID3D12Debug3> d3d12Debug = d3d12Debug0.as<ID3D12Debug3>();
		d3d12Debug->EnableDebugLayer();
		d3d12Debug->SetEnableGPUBasedValidation(TRUE);
#endif

		//
		// Create two D3D12 devices
		//
		com_ptr<ID3D12Device> d3d12Device1, d3d12Device2;
		check_hresult(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_12_1, guid_of<ID3D12Device>(), d3d12Device1.put_void()));
		check_hresult(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_12_1, guid_of<ID3D12Device>(), d3d12Device2.put_void()));
		D3D12_FEATURE_DATA_D3D12_OPTIONS4 options4{};
		check_hresult(d3d12Device1->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &options4, sizeof(options4)));
		printf("D3D12 Shared Resource Compat Tier: %d\n\n", options4.SharedResourceCompatibilityTier);

		com_ptr<ID3D12CommandQueue> d3dDeviceQueue1, d3dDeviceQueue2;
		D3D12_COMMAND_QUEUE_DESC cqDesc{};
		cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		check_hresult(d3d12Device1->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&d3dDeviceQueue1)));
		check_hresult(d3d12Device2->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&d3dDeviceQueue2)));

		//
		// Create two D3D11 devices
		//
		UINT DebugFlags = 0;
#ifdef _DEBUG
		DebugFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		const std::vector<D3D_FEATURE_LEVEL> featureLevels{ D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
		com_ptr<ID3D11Device> d3d11Device1a, d3d11Device2a;
		check_hresult(D3D11CreateDevice(adapter.get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | DebugFlags,
			featureLevels.data(), static_cast<UINT>(std::size(featureLevels)), D3D11_SDK_VERSION, d3d11Device1a.put(), nullptr, nullptr));
		check_hresult(D3D11CreateDevice(adapter.get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | DebugFlags,
			featureLevels.data(), static_cast<UINT>(std::size(featureLevels)), D3D11_SDK_VERSION, d3d11Device2a.put(), nullptr, nullptr));
		com_ptr<ID3D11Device5> d3d11Device1 = d3d11Device1a.as<ID3D11Device5>(), d3d11Device2 = d3d11Device2a.as<ID3D11Device5>();

		//
		// Run the test(s)
		//
		TexturePermationSharingTests(d3d11Device1, d3d11Device2, d3d12Device1, d3d12Device2);

		Fence11To12Test(d3d11Device1, d3d12Device1);
		Fence12To11Test(d3d11Device1, d3d12Device1);
		Fence12To12Test(d3d12Device1, d3d12Device2);
	}
	catch (winrt::hresult_error& ex) {
		printf("Unexpected exception: HResult=%X Message=%ws", ex.code().value, ex.message().c_str());
	}
	catch (...) {
		printf("Unhandled exception\n");
	}

	CoUninitialize();
}
