#include "stdafx.h"
#include "directx_backend.h"
#include <render/render_macros.h>
#include <math/math_utils.h>

using Microsoft::WRL::ComPtr;


ComPtr<IDXGIOutput> xtest::render::PrimaryOutput(ComPtr<ID3D11Device> d3dDevice)
{
	ComPtr<IDXGIDevice> dxgiDevice;
	XTEST_D3D_CHECK(d3dDevice.As(&dxgiDevice));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	XTEST_D3D_CHECK(dxgiDevice->GetAdapter(&dxgiAdapter));

	ComPtr<IDXGIOutput> dxgiOutput;
	XTEST_D3D_CHECK(dxgiAdapter->EnumOutputs(0, &dxgiOutput)); // the primary output has always index 0
	return dxgiOutput;
}

std::vector<DXGI_MODE_DESC> xtest::render::OutputModesByRatio(ComPtr<IDXGIOutput> d3dOutput, float ratio)
{
	UINT modesCount;
	d3dOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &modesCount, 0);

	std::vector<DXGI_MODE_DESC> modes(modesCount);
	d3dOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &modesCount, &modes[0]);
	
	auto eraseFromIter = std::remove_if(modes.begin(), modes.end(), [ratio](const DXGI_MODE_DESC& mode) { return !math::EqualRelativeEpsilon(float(mode.Width) / float(mode.Height), ratio); });
	modes.erase(eraseFromIter, modes.end());
	
	return modes;
}

std::vector<DXGI_MODE_DESC> xtest::render::BestMatchOutputModes(ComPtr<IDXGIOutput> d3dOutput, UINT desiredWidth, UINT desiredHeight, OutputModeOrder ordering)
{
	std::vector<DXGI_MODE_DESC> modes = OutputModesByRatio(d3dOutput, float(desiredWidth) / float(desiredHeight));
	//std::cout << modes.size();
	
	auto eraseFromIter = std::remove_if(modes.begin(), modes.end(), [desiredWidth](const DXGI_MODE_DESC& mode) { return mode.Width > desiredWidth; });
	modes.erase(eraseFromIter, modes.end());

	// decreasing size order
	std::sort(
		modes.begin(),
		modes.end(),
		[ordering](const DXGI_MODE_DESC& first, const DXGI_MODE_DESC& second)
		{
			if (first.Width == second.Width)
			{
				if (ordering == OutputModeOrder::high_refresh_first)
				{
					return first.RefreshRate.Numerator > second.RefreshRate.Numerator;
				}
				else
				{
					return first.RefreshRate.Numerator < second.RefreshRate.Numerator;
				}
			}
			else
			{
				return first.Width > second.Width;
			}
		}
	);
	
	return modes;
}



