#pragma once

#include "dxptr.h"
#include "dxstructures.h"
#include <vector>
#include "dxSwapChain.h"
#include "mesh.h"
#include "constantBuffer.h"

namespace mini
{
	namespace windows
	{
		class window;
	}
	class DxDevice
	{
		// TODO : remove once moved to new namespace
		template<typename T>
		using dx_ptr = mini::directx::dx_ptr<T>;
	public:
		explicit DxDevice(const windows::window& window);
		DxDevice(DxDevice&& other) noexcept
			: m_device(std::move(other.m_device)), m_immediateContext(move(m_immediateContext)),
			m_swapChain(std::move(other.swapChain()))
		{ }

		DxDevice(const DxDevice& other)
			: m_swapChain(other.m_swapChain)
		{
			other.m_device->AddRef();
			m_device.reset(other.m_device.get());
			other.m_immediateContext->AddRef();
			m_immediateContext.reset(other.m_immediateContext.get());
		}

		dx_ptr<ID3D11Texture2D> CreateTexture(const directx::tex2d_info& desc,
			const directx::subresource_data* data = nullptr) const;
		dx_ptr<ID3D11Texture2D> CreateTexture(const directx::tex2d_info& desc,
			const directx::subresource_data& data) const
		{
			return CreateTexture(desc, &data);
		}
		dx_ptr<ID3D11Texture3D> CreateTexture(const directx::tex3d_info& desc,
			const directx::subresource_data* data = nullptr) const;
		dx_ptr<ID3D11Texture3D> CreateTexture(const directx::tex3d_info& desc,
			const directx::subresource_data& data) const
		{
			return CreateTexture(desc, &data);
		}
		dx_ptr<ID3D11RenderTargetView> CreateRenderTargetView(const dx_ptr<ID3D11Texture2D>& backTexture) const;
		dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(const dx_ptr<ID3D11Texture2D>& depthTexture, const directx::depth_stencil_view_info& desc) const
		{
			return CreateDepthStencilView(depthTexture, &desc);
		}
		dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(const dx_ptr<ID3D11Texture2D>& depthTexture, const directx::depth_stencil_view_info* desc = nullptr) const;
		dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(unsigned int width, unsigned int height) const;
		dx_ptr<ID3D11DepthStencilView> CreateDepthStencilView(SIZE size) const
		{
			return CreateDepthStencilView(size.cx, size.cy);
		}

		template<typename VERT>
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const std::vector<BYTE>& vsByteCode) const
		{
			return CreateInputLayout(VERT::Layout, vsByteCode);
		}

		template<unsigned int N>
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC (&layout)[N],
			const std::vector<BYTE>& vsByteCode) const
		{
			return CreateInputLayout(layout, N, vsByteCode);
		}

		dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout,
															 unsigned int layoutElements,
															 const std::vector<BYTE>& vsByteCode) const
		{
			return CreateInputLayout(layout, layoutElements, vsByteCode.data(), vsByteCode.size());
		}

		template<typename VERT, size_t M>
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const BYTE (&vsByteCode)[M]) const
		{
			return CreateInputLayout(VERT::Layout, vsByteCode, M);
		}

		template<unsigned int N, size_t M>
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC(&layout)[N],
			const BYTE (&vsByteCode)[M]) const
		{
			return CreateInputLayout(layout, N, vsByteCode, M);
		}

		template<size_t M>
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout,
			unsigned int layoutElements, const BYTE (&vsByteCode)[M]) const
		{
			return CreateInputLayout(layout, layoutElements, vsByteCode, M);
		}

		template<typename VERT>
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const BYTE* vsByteCode, UINT vsByteCodeSize) const
		{
			return CreateInputLayout(VERT::Layout, vsByteCode, vsByteCodeSize);
		}

		template<unsigned int N>
		dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC(&layout)[N],
			const BYTE* vsByteCode, UINT vsByteCodeSize) const
		{
			return CreateInputLayout(layout, N, vsByteCode, vsByteCodeSize);
		}

		dx_ptr<ID3D11InputLayout> CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout,
			unsigned int layoutElements, const BYTE* vsByteCode, size_t vsByteCodeSize) const;

		template<class T>
		dx_ptr<ID3D11Buffer> CreateVertexBuffer(unsigned int N) const
		{
			//since no data is provided assume buffer data will be frequently modified
			return CreateBuffer(directx::buffer_info::vertex_buffer(N * sizeof(T), D3D11_USAGE_DYNAMIC));
		}

		template<class T>
		dx_ptr<ID3D11Buffer> CreateVertexBuffer(const T* buffer, size_t count, D3D11_USAGE usage = D3D11_USAGE_DEFAULT)
			const
		{
			return CreateBuffer(
				directx::buffer_info::vertex_buffer(static_cast<UINT>(sizeof(T)*count), usage),
				reinterpret_cast<const void*>(buffer));
		}

		template<class T>
		dx_ptr<ID3D11Buffer> CreateVertexBuffer(const std::vector<T>& vertices,
			D3D11_USAGE usage = D3D11_USAGE_DEFAULT) const
		{
			assert(vertices.size() * sizeof(T) <= UINT_MAX);
			return CreateBuffer(
				directx::buffer_info::vertex_buffer(static_cast<UINT>(vertices.size() * sizeof(T)),
					usage),
				reinterpret_cast<const void*>(vertices.data()));
		}

		template<class T, unsigned int N>
		dx_ptr<ID3D11Buffer> CreateVertexBuffer(const T (&vertices)[N], D3D11_USAGE usage = D3D11_USAGE_DEFAULT) const
		{
			assert(N * sizeof(T) <= UINT_MAX);
			return CreateBuffer(directx::buffer_info::vertex_buffer(static_cast<UINT>(N * sizeof(T)),
					usage),
				reinterpret_cast<const void*>(vertices));
		}

		template<class T>
		dx_ptr<ID3D11Buffer> CreateIndexBuffer(size_t N) const
		{
			auto desc = directx::buffer_info::index_buffer(static_cast<UINT>(N * sizeof(T)));
			desc.Usage = D3D11_USAGE_DYNAMIC; //since no data is provided assume buffer data will be frequently modified
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			return CreateBuffer(desc);
		}

		template<class T>
		dx_ptr<ID3D11Buffer> CreateIndexBuffer(const std::vector<T>& indices) const
		{
			assert(indices.size() * sizeof(T) <= UINT_MAX);
			return CreateBuffer(directx::buffer_info::index_buffer(static_cast<UINT>(indices.size() * sizeof(T))),
				reinterpret_cast<const void*>(indices.data()));
		}

		template<class T, unsigned int N>
		dx_ptr<ID3D11Buffer> CreateIndexBuffer(const T (&indices)[N]) const
		{
			assert(N * sizeof(T) <= UINT_MAX);
			return CreateBuffer(directx::buffer_info::index_buffer(static_cast<UINT>(N * sizeof(T))),
				reinterpret_cast<const void*>(indices));
		}

		template<class T, unsigned int N = 1>
		ConstantBuffer<T, N> CreateConstantBuffer() const
		{
			assert(sizeof(T)*N <= UINT_MAX);
			return ConstantBuffer<T, N>{ 
				CreateBuffer(directx::buffer_info::const_buffer(static_cast<UINT>(sizeof(T)*N)))
			};
		}

		template<class T, unsigned int N = 1>
		ConstantBuffer<T, N> CreateConstantBuffer(const typename ConstantBuffer<T, N>::ref_type data) const
		{
			assert(sizeof(T)*N <= UINT_MAX);
			return ConstantBuffer<T, N>{ 
				std::move(CreateBuffer(
					directx::buffer_info::const_buffer(static_cast<UINT>(sizeof(T)*N)),
					ConstantBuffer<T,N>::dataPtr(data)))
			};
		}

		dx_ptr<ID3D11Buffer> CreateBuffer(const directx::buffer_info& desc, const void* pData = nullptr) const;
		dx_ptr<ID3D11BlendState> CreateBlendState(const directx::blend_info& desc = {}) const;
		dx_ptr<ID3D11DepthStencilState> CreateDepthStencilState(const directx::depth_stencil_info& desc = {}) const;
		static std::vector<BYTE> LoadByteCode(const std::wstring& fileName);
		dx_ptr<ID3D11VertexShader> CreateVertexShader(const std::wstring& fileName) const
		{
			return CreateVertexShader(LoadByteCode(fileName));
		}
		dx_ptr<ID3D11VertexShader> CreateVertexShader(const std::vector<BYTE>& byteCode) const
		{
			return CreateVertexShader(byteCode.data(), byteCode.size());
		}
		template<int N>
		dx_ptr<ID3D11VertexShader> CreateVertexShader(const BYTE (&byteCode)[N]) const
		{
			return CreateVertexShader(byteCode, N);
		}
		dx_ptr<ID3D11VertexShader> CreateVertexShader(const BYTE* byteCode, size_t byteCodeSize) const;
		dx_ptr<ID3D11HullShader> CreateHullShader(const std::wstring& filename) const
		{
			return CreateHullShader(LoadByteCode(filename));
		}
		dx_ptr<ID3D11HullShader> CreateHullShader(const std::vector<BYTE>& byteCode) const
		{
			return CreateHullShader(byteCode.data(), byteCode.size());
		}
		template<int N>
		dx_ptr<ID3D11HullShader> CreateHullShader(const BYTE(&byteCode)[N]) const
		{
			return CreateHullShader(byteCode, N);
		}
		dx_ptr<ID3D11HullShader> CreateHullShader(const BYTE* byteCode, size_t byteCodeSize) const;
		dx_ptr<ID3D11DomainShader> CreateDomainShader(const std::wstring& filename) const
		{
			return CreateDomainShader(LoadByteCode(filename));
		}
		dx_ptr<ID3D11DomainShader> CreateDomainShader(const std::vector<BYTE>& byteCode) const
		{
			return CreateDomainShader(byteCode.data(), byteCode.size());
		}
		template<int N>
		dx_ptr<ID3D11DomainShader> CreateDomainShader(const BYTE(&byteCode)[N]) const
		{
			return CreateDomainShader(byteCode, N);
		}
		dx_ptr<ID3D11DomainShader> CreateDomainShader(const BYTE* byteCode, size_t byteCodeSize) const;
		dx_ptr<ID3D11GeometryShader> CreateGeometryShader(const std::wstring& filename) const
		{
			return CreateGeometryShader(LoadByteCode(filename));
		}
		template<int N>
		dx_ptr<ID3D11GeometryShader> CreateGeometryShader(const std::vector<BYTE>& byteCode,
			const D3D11_SO_DECLARATION_ENTRY(&soEntries)[N])
		{
			return CreateGeometryShader(byteCode.data(), byteCode.size(), soEntries, N);
		}
		dx_ptr<ID3D11GeometryShader> CreateGeometryShader(const BYTE* byteCode, size_t byteCodeSize,
			const D3D11_SO_DECLARATION_ENTRY *soEntries, size_t soEntriesCount);
		dx_ptr<ID3D11GeometryShader> CreateGeometryShader(const std::vector<BYTE>& byteCode) const
		{
			return CreateGeometryShader(byteCode.data(), byteCode.size());
		}
		template<size_t N>
		dx_ptr<ID3D11GeometryShader> CreateGeometryShader(const BYTE(&byteCode)[N]) const
		{
			return CreateGeometryShader(byteCode, N);
		}
		dx_ptr<ID3D11GeometryShader> CreateGeometryShader(const BYTE* byteCode, size_t byteCodeSize) const;
		dx_ptr<ID3D11PixelShader> CreatePixelShader(const std::wstring& filename) const
		{
			return CreatePixelShader(LoadByteCode(filename));
		}
		dx_ptr<ID3D11PixelShader> CreatePixelShader(const std::vector<BYTE>& byteCode) const
		{
			return CreatePixelShader(byteCode.data(), byteCode.size());
		}
		template<int N>
		dx_ptr<ID3D11PixelShader> CreatePixelShader(const BYTE (&byteCode)[N]) const
		{
			return CreatePixelShader(byteCode, N);
		}
		dx_ptr<ID3D11PixelShader> CreatePixelShader(const BYTE* byteCode, size_t byteCodeSize) const;

		dx_ptr<ID3D11ComputeShader> CreateComputeShader(const std::wstring& filename) const
		{
			return CreateComputeShader(LoadByteCode(filename));
		}
		dx_ptr<ID3D11ComputeShader> CreateComputeShader(const std::vector<BYTE>& byteCode) const
		{
			return CreateComputeShader(byteCode.data(), byteCode.size());
		}
		template<int N>
		dx_ptr<ID3D11ComputeShader> CreateComputeShader(const BYTE(&byteCode)[N]) const
		{
			return CreateComputeShader(byteCode, N);
		}
		dx_ptr<ID3D11ComputeShader> CreateComputeShader(const BYTE* byteCode, size_t byteCodeSize) const;


		dx_ptr<ID3D11RasterizerState> CreateRasterizerState(const directx::rasterizer_info& desc = { }) const;
		dx_ptr<ID3D11SamplerState> CreateSamplerState(const directx::sampler_info& desc = { }) const;
		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(ID3D11Resource* res,
			const directx::shader_resource_view_info* desc = nullptr) const;
		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(ID3D11Resource* res,
			const directx::shader_resource_view_info& desc) const
		{
			return CreateShaderResourceView(res, &desc);
		}
		
		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const std::wstring& texPath) const;

		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const std::vector<BYTE> imageFileContent)
		{
			return CreateShaderResourceView(imageFileContent.data(), imageFileContent.size());
		}

		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const BYTE* imageFileContent, size_t imageFileSize);

		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const dx_ptr<ID3D11Texture2D>& texture,
			const directx::shader_resource_view_info& desc) const
		{
			return CreateShaderResourceView(texture.get(), &desc);
		}
		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const dx_ptr<ID3D11Texture2D>& texture,
			const directx::shader_resource_view_info* desc = nullptr) const
		{
			return CreateShaderResourceView(texture.get(), desc);
		}
		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const dx_ptr<ID3D11Texture3D>& texture,
			const directx::shader_resource_view_info& desc) const
		{
			return CreateShaderResourceView(texture.get(), &desc);
		}
		dx_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(const dx_ptr<ID3D11Texture3D>& texture,
			const directx::shader_resource_view_info* desc = nullptr) const
		{
			return CreateShaderResourceView(texture.get(), desc);
		}

		dx_ptr<ID3D11UnorderedAccessView> CreateUnorderedAccessView(ID3D11Resource* res, const directx::unordered_access_view_info* desc = nullptr) const;

		dx_ptr<ID3D11UnorderedAccessView> CreateUnorderedAccessView(const dx_ptr<ID3D11Texture2D>& texture,
																  const directx::unordered_access_view_info* desc = nullptr) const
		{
			return CreateUnorderedAccessView(texture.get(), desc);
		}

		template<typename IDX, typename... VERTS>
		Mesh CreateMesh(const std::vector<IDX>& indices, const std::vector<VERTS>&... vertices) const
		{
			return CreateMesh(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, indices, vertices...);
		}

		template<typename IDX, typename... VERTS>
		Mesh CreateMesh(D3D11_PRIMITIVE_TOPOLOGY topology, const std::vector<IDX>& indices,
			const std::vector<VERTS>&... vertices) const
		{
			if (indices.size() == 0)
				return{};
			//dx_ptr<ID3D11Buffer> vbarray[] = { CreateVertexBuffer(vertices)... };
			ID3D11Buffer* vbarray[] = { CreateVertexBuffer(vertices).release()... };
			dx_ptr_vector<ID3D11Buffer> vbuffers{
				//std::make_move_iterator(std::begin(vbarray)), std::make_move_iterator(std::end(vbarray))
				std::begin(vbarray), std::end(vbarray)
			};
			std::vector<UINT> strides{ sizeof(VERTS)... };
			std::vector<UINT> offsets(sizeof...(VERTS), 0);
			dx_ptr<ID3D11Buffer> ib = CreateIndexBuffer(indices);
			return Mesh{ 
				std::move(vbuffers), std::move(strides), std::move(offsets), std::move(ib),
				static_cast<unsigned int>(indices.size()), topology
			};
		}

		ID3D11Device* operator->() const { return m_device.get(); }
		operator ID3D11Device*() const { return m_device.get(); }
		const dx_ptr<ID3D11DeviceContext>& context() const { return m_immediateContext; }
		const DxSwapChain& swapChain() const { return m_swapChain; }

	private:

		dx_ptr<ID3D11Device> m_device;
		dx_ptr<ID3D11DeviceContext> m_immediateContext;
		DxSwapChain m_swapChain;
	};
}
