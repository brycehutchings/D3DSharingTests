Tests sharing between D3D11 and D3D12 devices. Primary use is to bulk test the various DXGI_FORMATs, binding flags, and sharing mechanisms.

The key results (testing on an Nvidia card) is basically that sharing from D3D12 to D3D11 does not work in many cases where sharing from D3D11 to D3D12 does work.
Additionally, sharing D3D11 textures using NT handles (`D3D11_RESOURCE_MISC_SHARED_NTHANDLE`) does not work in some cases where sharing using the (deprecated?) mechanism of `IDXGIResource1::GetSharedHandle` does work.
