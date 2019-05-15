#pragma once

#include <render/render_resource.h>
#include <render/shading/common_types.h>
#include <service/locator.h>


namespace xtest {
namespace render {
namespace shading {

	// cbuffer base class only used for collect all the buffers in the same collection,
	// always used the template implementation shown down below
	class CBufferBase : public RenderResource
	{
	public:
		virtual ~CBufferBase() {};
		

		void SetBindingTarget(ShaderTarget target) { m_target = target; };
		void SetFrequency(CBufferFrequency frequency) { m_frequency = frequency; }

		template<typename T> 
		void UpdateBuffer(const T& data);

		template<typename T>
		const T& CurrentData() const;

	protected:

		ShaderTarget m_target;
		CBufferFrequency m_frequency;

	};


	// cbuffer template implementation where the template parameter T stands for the data it handles
	template<typename T>
	class CBuffer final : public CBufferBase
	{
	public:
		
		typedef T data_type;


		CBuffer();
		virtual ~CBuffer() {}

		virtual void Init() override;
		virtual void Bind() override;

		void UpdateBuffer(const data_type& data);
		const data_type& CurrentData() const;


	protected:

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_d3dCBuffer;
		data_type m_data;
	};



	template<typename T>
	CBuffer<T>::CBuffer()
	{
		m_target = ShaderTarget::unknown;
		m_frequency = CBufferFrequency::unknown;
	}



	template<typename T>
	void CBuffer<T>::UpdateBuffer(const data_type& data)
	{
		XTEST_ASSERT(m_d3dCBuffer, L"uninitialized cbuffer");

		ID3D11DeviceContext* d3dContext = service::Locator::GetD3DContext();


		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
				
		XTEST_D3D_CHECK(d3dContext->Map(m_d3dCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		data_type* pData_asBufferType = static_cast<data_type*>(mappedResource.pData);
		*pData_asBufferType = data;
		m_data = data;

		d3dContext->Unmap(m_d3dCBuffer.Get(), 0);
	}


	template<typename T>
	const T& CBuffer<T>::CurrentData() const
	{
		return m_data;
	}


	template<typename T>
	void CBuffer<T>::Init()
	{
		if (m_d3dCBuffer)
		{
			return;
		}

		D3D11_BUFFER_DESC perObjectCBDesc;
		perObjectCBDesc.Usage = D3D11_USAGE_DYNAMIC;
		perObjectCBDesc.ByteWidth = sizeof(data_type);
		perObjectCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		perObjectCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		perObjectCBDesc.MiscFlags = 0;
		perObjectCBDesc.StructureByteStride = 0;
		XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateBuffer(&perObjectCBDesc, nullptr, &m_d3dCBuffer));
	}

	



	template<typename T>
	void CBuffer<T>::Bind()
	{
		XTEST_ASSERT(m_d3dCBuffer, L"uninitialized cbuffer");
		XTEST_ASSERT(m_frequency != CBufferFrequency::unknown);

		switch (m_target)
		{
		case ShaderTarget::vertex_shader:
			service::Locator::GetD3DContext()->VSSetConstantBuffers(static_cast<int>(m_frequency), 1, m_d3dCBuffer.GetAddressOf());
			break;

		case ShaderTarget::pixel_shader:
			service::Locator::GetD3DContext()->PSSetConstantBuffers(static_cast<int>(m_frequency), 1, m_d3dCBuffer.GetAddressOf());
			break;

		default:
			XTEST_ASSERT(false, L"unknown binding point");
			break;
		}
	}



	template<typename T>
	void CBufferBase::UpdateBuffer(const T& data)
	{
		XTEST_ASSERT(typeid(*this) == typeid(CBuffer<T>), L"The buffer being updated isn't based on the data type you specified");
		static_cast<CBuffer<T>*>(this)->UpdateBuffer(data);
	}


	template<typename T>
	const T& CBufferBase::CurrentData() const
	{
		XTEST_ASSERT(typeid(*this) == typeid(CBuffer<T>), L"The buffer being queried isn't based on the data type you specified");
		return static_cast<const CBuffer<T>*>(this)->CurrentData();
	}

}// shading
}// render
}// xtest