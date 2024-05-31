#pragma once

#include <d3d11.h>
#include <limits>

namespace mini::directx
{
	struct swap_chain_info : DXGI_SWAP_CHAIN_DESC
	{
		constexpr swap_chain_info(HWND handle, SIZE size) noexcept
			: DXGI_SWAP_CHAIN_DESC{
				.BufferDesc {
					.Width = static_cast<UINT>(size.cx),
					.Height = static_cast<UINT>(size.cy),
					.RefreshRate{
					//.Numerator = 0,
					.Denominator = 1
				},
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM//,
				//.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED /* 0 */,
				//.Scaling = DXGI_MODE_SCALING_UNSPECIFIED /* 0 */,
				},
				.SampleDesc{
					.Count = 1//,
					//.Quality = 0
				},
				.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
				.BufferCount = 2,
				.OutputWindow = handle,
				.Windowed = true,
				.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
				.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
			  }
		{ }
	};
	
	struct tex2d_info : D3D11_TEXTURE2D_DESC
	{
		constexpr tex2d_info(UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
							 UINT mipLevels = 0 /* Full set of subtextures */) noexcept
			: D3D11_TEXTURE2D_DESC{
				.Width = width,
				.Height = height,
				.MipLevels = mipLevels,
				.ArraySize = 1,
				.Format = format,
				.SampleDesc{
					.Count = 1//,
					//.Quality = 0
				},
				//.Usage = D3D11_USAGE_DEFAULT /* 0 */,
				.BindFlags = D3D11_BIND_SHADER_RESOURCE//,
				//.CPUAccessFlags = 0,
				//.MiscFlags = 0
			  }
		{ }

		[[nodiscard]] constexpr static tex2d_info depth_stencil(UINT width, UINT height) noexcept
		{
			tex2d_info desc{ width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, 1 };
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			return desc;
		}
	};

	struct tex3d_info : D3D11_TEXTURE3D_DESC
	{
		explicit tex3d_info(UINT width, UINT height, UINT depth, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
							UINT mipLevels = 0)
			: D3D11_TEXTURE3D_DESC{
				.Width = width,
				.Height = height,
				.Depth = depth,
				.MipLevels = mipLevels,
				.Format = format,
				.Usage = D3D11_USAGE_DEFAULT,
				.BindFlags = D3D11_BIND_SHADER_RESOURCE,
				.CPUAccessFlags = 0,
				.MiscFlags = 0
			  }
		{ }
	};

	struct depth_stencil_view_info : D3D11_DEPTH_STENCIL_VIEW_DESC
	{
		constexpr depth_stencil_view_info() noexcept
			: D3D11_DEPTH_STENCIL_VIEW_DESC{
				.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
				.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
				.Flags = 0,
				.Texture2D{ .MipSlice = 0 }
			  }
		{ }
	};

	struct viewport : D3D11_VIEWPORT
	{
		constexpr explicit viewport(SIZE size = { }) noexcept
			: D3D11_VIEWPORT{
				//.TopLeftX = 0.f,
				//.TopLeftY = 0.f,
				.Width = static_cast<FLOAT>(size.cx),
				.Height = static_cast<FLOAT>(size.cy),
				//.MinDepth = 0.f,
				.MaxDepth = 1.f
			  }
		{ }
	};

	struct buffer_info : D3D11_BUFFER_DESC
	{
		constexpr buffer_info(UINT bind_flags, UINT byte_count, D3D11_USAGE usage = D3D11_USAGE_DEFAULT) noexcept
			: D3D11_BUFFER_DESC{
				.ByteWidth = byte_count,
				.Usage = usage,
				.BindFlags = bind_flags,
				.CPUAccessFlags = usage & D3D11_USAGE_DYNAMIC || usage & D3D11_USAGE_STAGING ?
									D3D11_CPU_ACCESS_WRITE : 0U //,
				//.MiscFlags = 0,
				//.StructureByteStride = 0
			  }
		{ }

		[[nodiscard]] constexpr static
		buffer_info vertex_buffer(UINT byte_count, D3D11_USAGE usage = D3D11_USAGE_DEFAULT) noexcept
		{
			return { D3D11_BIND_VERTEX_BUFFER, byte_count, usage };
		}

		[[nodiscard]] constexpr static buffer_info index_buffer(UINT byte_count) noexcept
		{
			return { D3D11_BIND_INDEX_BUFFER, byte_count };
		}

		[[nodiscard]] constexpr static buffer_info const_buffer(UINT byte_count) noexcept
		{
			return { D3D11_BIND_CONSTANT_BUFFER, byte_count, D3D11_USAGE_DYNAMIC };
		}

		template<typename T, UINT N>
		[[nodiscard]] constexpr static buffer_info const_buffer() noexcept
		{
			return { sizeof(T) * N };
		}
	};
	
	struct rasterizer_info : D3D11_RASTERIZER_DESC
	{
		constexpr rasterizer_info(bool front_ccw = false) noexcept
			: D3D11_RASTERIZER_DESC{
				.FillMode = D3D11_FILL_SOLID /* Determines the solid fill mode(as opposed to wireframe) */,
				.CullMode = D3D11_CULL_BACK /* Indicates that back facing triangles are not drawn */,
				.FrontCounterClockwise = front_ccw/* Indicates if vertices of a front facing triangle are counter-clockwise */,
				//.DepthBias = 0,
				//.DepthBiasClamp = 0.f,
				//.SlopeScaledDepthBias = 0.f,
				.DepthClipEnable = true//,
				//.ScissorEnable = false,
				//.MultisampleEnable = false,
				//.AntialiasedLineEnable = false
			  }
		{ }
	};
	
	struct sampler_info : D3D11_SAMPLER_DESC
	{
		constexpr sampler_info() noexcept
			: D3D11_SAMPLER_DESC{
				.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
				.AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
				.AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
				.AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
				.MaxAnisotropy = 16,
				.ComparisonFunc = D3D11_COMPARISON_NEVER,
				.BorderColor{ 1.f, 1.f, 1.f, 1.f },
				.MinLOD = -D3D11_FLOAT32_MAX,
				.MaxLOD = D3D11_FLOAT32_MAX
			  }
		{ }
	};

	struct blend_info : D3D11_BLEND_DESC
	{
		constexpr blend_info() noexcept
			: D3D11_BLEND_DESC{
				//.AlphaToCoverageEnable = false,
				//.IndependentBlendEnable = false,
				.RenderTarget{
					/*[0]*/{
						//.BlendEnable = false,
						.SrcBlend = D3D11_BLEND_ONE,
						.DestBlend = D3D11_BLEND_ZERO,
						.BlendOp = D3D11_BLEND_OP_ADD,
						.SrcBlendAlpha = D3D11_BLEND_ONE,
						.DestBlendAlpha = D3D11_BLEND_ZERO,
						.BlendOpAlpha = D3D11_BLEND_OP_ADD,
						.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL
					}
				}
			  }
		{ }

		constexpr static blend_info alpha_blend() noexcept
		{
			blend_info desc{ };
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			return desc;
		}

		constexpr static blend_info additive_blend() noexcept
		{
			blend_info desc{ };
			desc.RenderTarget[0].BlendEnable = true;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			return desc;
		}

		constexpr static UINT mask_all = std::numeric_limits<UINT>::max();
	};
	
	struct depth_stencil_info : D3D11_DEPTH_STENCIL_DESC
	{
		constexpr depth_stencil_info() noexcept
			: D3D11_DEPTH_STENCIL_DESC{
				.DepthEnable = true,
				.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
				.DepthFunc = D3D11_COMPARISON_LESS,
				//.StencilEnable = false,
				.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
				.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
				.FrontFace{
					.StencilFailOp = D3D11_STENCIL_OP_KEEP,
					.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
					.StencilPassOp = D3D11_STENCIL_OP_KEEP,
					.StencilFunc = D3D11_COMPARISON_ALWAYS
				},
				.BackFace{
					.StencilFailOp = D3D11_STENCIL_OP_KEEP,
					.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
					.StencilPassOp = D3D11_STENCIL_OP_KEEP,
					.StencilFunc = D3D11_COMPARISON_ALWAYS
				}
			  }
		{ }

		constexpr static depth_stencil_info stencil_write() noexcept
		{
			depth_stencil_info desc{ };
			desc.StencilEnable = true; //Enable stencil operations
			desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; //Disable writing to depth buffer
			desc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER; //Back faces should never pass stencil test
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; //Front faces should always pass stencil test
			desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; //when pixel passes depth and stencil test write to stencil buffer
			return desc;
		}

		constexpr static depth_stencil_info stencil_test() noexcept
		{
			depth_stencil_info desc{ };
			desc.StencilEnable = true; //Enable stencil operation
			desc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER; //Back faces should never pass stencil test
			desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL; //Front faces pass stencil test when drawn on pixels with specific stencil buffer value
			return desc;
		}
	};

	struct shader_resource_view_info : D3D11_SHADER_RESOURCE_VIEW_DESC
	{
		constexpr shader_resource_view_info() noexcept
			: D3D11_SHADER_RESOURCE_VIEW_DESC{}
		{ }

		constexpr static shader_resource_view_info tex2d_view() noexcept
		{
			shader_resource_view_info desc{ };
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipLevels = -1;
			desc.Texture2D.MostDetailedMip = 0;
			return desc;
		}
	};

	struct unordered_access_view_info : D3D11_UNORDERED_ACCESS_VIEW_DESC
	{
		constexpr unordered_access_view_info() noexcept
			: D3D11_UNORDERED_ACCESS_VIEW_DESC{}
		{}

		constexpr static unordered_access_view_info tex2d_view() noexcept
		{
			unordered_access_view_info desc{ };
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = 1;
			return desc;
		}
	};

	struct subresource_data : D3D11_SUBRESOURCE_DATA
	{
		constexpr subresource_data() noexcept
			: D3D11_SUBRESOURCE_DATA{ }
		{ }
	};
}
