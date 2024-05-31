#pragma once
#include <map>
#include "cbVariableSemantics.h"
#include "cbVariable.h"
#include "clock.h"
#include <DirectXMath.h>
#include "model.h"
#include "cBufferDesc.h"
#include "dxstructures.h"
#include "effect.h"
#include "movementManager.h"

namespace mini
{
	class DxDevice;
	class ViewFrustrum;
	namespace directx
	{
		class camera;
	}

	namespace gk2
	{
		class CBVariableManager
		{
			// TODO : remove once moved to new namespace
			template<typename T>
			using dx_ptr = mini::directx::dx_ptr<T>;
		public:
			void UpdateView(const directx::camera& camera, const ViewFrustrum& frustrum);
			void UpdateFrustrum(const ViewFrustrum& frustrum, const directx::camera & camera);
			void UpdateViewAndFrustrum(const directx::camera & camera, const ViewFrustrum& frusturm);
			void UpdateFrame(const dx_ptr<ID3D11DeviceContext>& context, const std::vector<std::unique_ptr<MovementManager>>& manager, utils::clock const &clock);
			void UpdateModel(const Model::NodeIterator& modelPart);

			void AddSampler(const DxDevice& device, const std::string& name, const directx::sampler_info& desc = {});

			void AddTexture(const DxDevice& device, const std::string& name, const std::wstring& file);
			void AddTexture(const DxDevice& device, const std::string& name, const directx::tex2d_info& desc);
			void AddTexture(const DxDevice& device, const std::string& name, const dx_ptr<ID3D11Texture2D>& texture);
			void AddRWTexture(const DxDevice& device, const std::string& name, const directx::tex2d_info& desc);
			void AddRWTexture(const DxDevice& device, const std::string& name, unsigned int textureSize)
			{
				directx::tex2d_info desc(textureSize, textureSize);
				desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
				desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.MipLevels = 1;
				desc.CPUAccessFlags = 0;
				AddRWTexture(device, name, desc);
			}
			void AddUAV(const DxDevice& device, const std::string& name, const directx::tex2d_info& desc);
			void AddUAV(const DxDevice& device, const std::string& name, unsigned int textureSize)
			{
				directx::tex2d_info desc(textureSize, textureSize);
				desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
				desc.Format = DXGI_FORMAT_R32_FLOAT;
				desc.SampleDesc.Count = 1;
				desc.SampleDesc.Quality = 0;
				desc.MipLevels = 1;
				desc.CPUAccessFlags = 0;
				AddUAV(device, name, desc);
			}
			void AddRenderableTexture(const DxDevice& device, const std::string& name, const directx::tex2d_info& desc);
			void AddRenderableTexture(const DxDevice& device, const std::string& name, const SIZE textureSize)
			{
				directx::tex2d_info desc(textureSize.cx, textureSize.cy);
				desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
				desc.MipLevels = 1;
				AddRenderableTexture(device, name, desc);
			}

			void AddSemanticVariable(const std::string& name, VariableSemantic semantic);

			template<typename T>
			CBVariable<std::remove_cv_t<std::remove_reference_t<T>>>* AddNamedVariable(const std::string& name, T&& value)
			{
				using valueT = std::remove_cv_t<std::remove_reference_t<T>>;
				if (m_variableNames.find(name) != m_variableNames.end())
					return nullptr;
				auto uPtr = std::make_unique<CBVariable<valueT>>();
				auto result = uPtr.get();
				*uPtr = value;
				m_constantVariables.push_back(std::move(uPtr));
				m_variableNames.emplace(name, result);
				return result;
			}

			const dx_ptr<ID3D11SamplerState>& GetSampler(const std::string& name) const;

			const dx_ptr<ID3D11ShaderResourceView>& GetTexture(const std::string& name) const;

			const dx_ptr<ID3D11UnorderedAccessView>& GetUAV(const std::string& name) const;

			const RenderTargetsEffect& GetRenderTarget(const std::string& name) const;

			const ICBVariable* GetVariable(const std::string& name) const
			{
				auto it = m_variableNames.find(name);
				if (it == m_variableNames.end())
					return nullptr;
				return it->second;
			}

			inline const std::vector<std::string>& GetSharedTextures() const
			{
				return m_sharedTextures;
			}

			inline const std::vector<std::string>& GetSharedUavs() const
			{
				return m_sharedUavs;
			}

			void FillCBuffer(void* buffer, const CBufferDesc& bufferDesc) const
			{
				for(auto& desc : bufferDesc.variables)
				{
					auto varPtr = GetVariable(desc.name);
					if (!varPtr) continue;
					varPtr->copyTo(static_cast<unsigned char*>(buffer) + desc.offset, desc.size);
				}
			}

		private:
			using semantic_map_t = std::map<VariableSemantic, std::unique_ptr<ICBVariable>>;
			using semantic_map_iterator = semantic_map_t::iterator;

			template<typename T, VariableSemantic semantic>
			CBVariable<T>& _getSemanticVariable()
			{
				auto it = m_semanticVariables.find(semantic);
				assert(it!= m_semanticVariables.end());
				ICBVariable& var = *it->second;
				assert(typeid(var) == typeid(CBVariable<T>));
				return static_cast<CBVariable<T>&>(var);
			}

			static constexpr bool _isMatrixSemantic(VariableSemantic semantic)
			{
				return (semantic >= VariableSemantic::MatV && semantic <= VariableSemantic::MatVInvT) ||
					(semantic >= VariableSemantic::MatVP && semantic <= VariableSemantic::MatPInvT) ||
					(semantic >= VariableSemantic::MatM && semantic <= VariableSemantic::MatMVPInvT);
			}

			static constexpr bool _isVec4Semantic(VariableSemantic semantic)
			{
				return semantic >= VariableSemantic::Vec4CamPos && semantic <= VariableSemantic::Vec4CamUp;
			}

			static constexpr bool _isFloatSemantic(VariableSemantic semantic)
			{
				return (semantic >= VariableSemantic::FloatFOV && semantic <= VariableSemantic::FloatFarPlane) ||
					(semantic >= VariableSemantic::FloatDT && semantic <= VariableSemantic::FloatTotalFrames);
			}

			static constexpr bool _isBoolSemantic(VariableSemantic semantic)
			{
				return (semantic >= VariableSemantic::BoolSwap && semantic <= VariableSemantic::BoolSwap);
			}

			static constexpr bool _isVec2Semantic(VariableSemantic semantic)
			{
				return semantic == VariableSemantic::Vec2ViewportDims || semantic == VariableSemantic::Vec2Managed;
			}

			template<typename T>
			CBVariable<T>* _addSemanticVariable(VariableSemantic semantic)
			{
				auto uPtr = std::make_unique<CBVariable<T>>();
				auto result = uPtr.get();
				m_semanticVariables.emplace(semantic, std::move(uPtr));
				return result;
			}

			CBVariable<DirectX::XMFLOAT4X4>* _addSemanticMatrixVariable(VariableSemantic semantic);

			ICBVariable* _addSemanticVariable(VariableSemantic semantic);

			ICBVariable* _addOrGetSemanticVariable(VariableSemantic semantic);

			static constexpr VariableSemantic _semanticOffset(VariableSemantic semantic, int offset)
			{
				return static_cast<VariableSemantic>(static_cast<int>(semantic) + offset);
			}
			static constexpr VariableSemantic _semanticMT(VariableSemantic m) { return _semanticOffset(m, 1); }
			static constexpr VariableSemantic _semanticMI(VariableSemantic m) { return _semanticOffset(m, 2); }
			static constexpr VariableSemantic _semanticMIT(VariableSemantic m) { return _semanticOffset(m, 3); }

			static void _updateMatrix(CBVariable<DirectX::XMFLOAT4X4>& var, const DirectX::XMMATRIX& m)
			{
				XMStoreFloat4x4(&var.value, m);
			}
			static void _updateMatrix(ICBVariable& var, const DirectX::XMMATRIX& m)
			{
				assert(typeid(var) == typeid(CBVariable<DirectX::XMFLOAT4X4>));
				_updateMatrix(static_cast<CBVariable<DirectX::XMFLOAT4X4>&>(var), m);
			}
			static void _updateVec4(CBVariable<DirectX::XMFLOAT4>& var, const DirectX::XMVECTOR& v)
			{
				XMStoreFloat4(&var.value, v);
			}
			static void _updateVec4(ICBVariable& var, const DirectX::XMVECTOR& v)
			{
				assert(typeid(var) == typeid(CBVariable<DirectX::XMFLOAT4>));
				_updateVec4(static_cast<CBVariable<DirectX::XMFLOAT4>&>(var), v);
			}
			static void _updateBool(CBVariable<bool>& var)
			{
				var.value = !var.value;
			}
			static void _updateBool(ICBVariable& var)
			{
				assert(typeid(var) == typeid(CBVariable<bool>));
				_updateBool(static_cast<CBVariable<bool>&>(var));
			}
			static void _updateFloat(CBVariable<float>& var, float f)
			{
				var.value = f;
			}
			static void _updateFloat(ICBVariable& var, float f)
			{
				assert(typeid(var) == typeid(CBVariable<float>));
				_updateFloat(static_cast<CBVariable<float>&>(var), f);
			}

			static void _incrementFloat(CBVariable<float>& var, float df = 1)
			{
				var.value += df;
			}

			static void _incrementFloat(ICBVariable& var, float df = 1)
			{
				assert(typeid(var) == typeid(CBVariable<float>));
				_incrementFloat(static_cast<CBVariable<float>&>(var), df);
			}

			static void _updateVec2(CBVariable<DirectX::XMFLOAT2>& var, const DirectX::XMFLOAT2& v)
			{
				var.value = v;
			}

			static void _updateVec2(ICBVariable& var, const DirectX::XMFLOAT2& v)
			{
				assert(typeid(var) == typeid(CBVariable<DirectX::XMFLOAT2>));
				_updateVec2(static_cast<CBVariable<DirectX::XMFLOAT2>&>(var), v);
			}

			template<VariableSemantic semantic>
			bool _updateVec2(semantic_map_iterator& it, const DirectX::XMFLOAT2& v)
			{
				static_assert(semantic == VariableSemantic::Vec2ViewportDims || semantic == VariableSemantic::Vec2Managed, "Invalid vec2 semantic");
				if (it->first == semantic)
				{
					_updateVec2(*(it++)->second, v);
					return it != m_semanticVariables.end();
				}
				return true;
			}

			template<VariableSemantic semantic>
			bool _updateBool(semantic_map_iterator& it)
			{
				static_assert(semantic == VariableSemantic::BoolSwap, "Invalid bool semantic");
				if (it->first == semantic)
				{
					_updateBool(*(it++)->second);
					return it != m_semanticVariables.end();
				}
				return true;
			}

			template<VariableSemantic semantic>
			bool _updateFloat(semantic_map_iterator& it, float f)
			{
				static_assert(semantic == VariableSemantic::FloatDT || semantic == VariableSemantic::FloatFOV ||
					semantic == VariableSemantic::FloatFPS || semantic == VariableSemantic::FloatFarPlane ||
					semantic == VariableSemantic::FloatNearPlane, "Invalid float semantic");
				if (it->first == semantic)
				{
					_updateFloat(*(it++)->second, f);
					return it != m_semanticVariables.end();
				}
				return true;
			}

			template<VariableSemantic semantic>
			bool _incrementFloat(semantic_map_iterator& it, float df = 1)
			{
				static_assert(semantic == VariableSemantic::FloatT ||
					semantic == VariableSemantic::FloatTotalFrames, "Invalid float semantic");
				if (it->first == semantic)
				{
					_incrementFloat(*(it++)->second, df);
					return it != m_semanticVariables.end();
				}
				return true;
			}

			template<VariableSemantic semantic>
			bool _updateVec4(semantic_map_iterator& it, const DirectX::XMVECTOR& v)
			{
				static_assert(semantic == VariableSemantic::Vec4CamPos || semantic == VariableSemantic::Vec4CamDir ||
					semantic == VariableSemantic::Vec4CamRight || semantic == VariableSemantic::Vec4CamUp, "Invalid vec4 semantic");
				if (it->first == semantic)
				{
					_updateVec4(*(it++)->second, v);
					return it != m_semanticVariables.end();
				}
				return true;
			}

			template<VariableSemantic semantic>
			bool _updateMatrix(semantic_map_iterator& it, const DirectX::XMMATRIX& m)
			{
				if (it->first == semantic)
				{
					_updateMatrix(*(it++)->second, m);
					return it != m_semanticVariables.end();
				}
				return true;
			}

			template<VariableSemantic semantic>
			bool _update_M_MT(semantic_map_iterator& it, const DirectX::XMMATRIX& m)
			{
				static_assert(semantic == VariableSemantic::MatV || semantic == VariableSemantic::MatVInv ||
					semantic == VariableSemantic::MatVP || semantic == VariableSemantic::MatVPInv ||
					semantic == VariableSemantic::MatP || semantic == VariableSemantic::MatPInv ||
					semantic == VariableSemantic::MatM || semantic == VariableSemantic::MatMInv ||
					semantic == VariableSemantic::MatMV || semantic == VariableSemantic::MatMVInv ||
					semantic == VariableSemantic::MatMVP || semantic == VariableSemantic::MatMVPInv, "Invalid matrix semantic");
				if (!_updateMatrix<semantic>(it, m))
					return false;
				if (it->first == _semanticMT(semantic))
				{
					_updateMatrix(*(it++)->second, XMMatrixTranspose(m));
					return it != m_semanticVariables.end();
				}
				return true;
			}

			template<VariableSemantic semantic>
			bool _update_MI_MIT(semantic_map_iterator& it, const DirectX::XMMATRIX& m)
			{
				static_assert(semantic == VariableSemantic::MatV || semantic == VariableSemantic::MatVP ||
					semantic == VariableSemantic::MatP || semantic == VariableSemantic::MatM ||
					semantic == VariableSemantic::MatMV || semantic == VariableSemantic::MatMVP, "Invalid matrix semantic");
				if (it->first <= _semanticMIT(semantic))
				{
					DirectX::XMVECTOR det;
					return _update_M_MT<_semanticMI(semantic)>(it, XMMatrixInverse(&det, m));
				}
				return true;
			}

			template<VariableSemantic semantic>
			bool _update_M_MT_MI_MIT(semantic_map_iterator& it, const DirectX::XMMATRIX& m)
			{
				return _update_M_MT<semantic>(it, m) && /*will not execute if first returns false*/ _update_MI_MIT<semantic>(it, m);
			}

			bool _updateView(semantic_map_iterator& it, const DirectX::XMMATRIX& viewMtx);
			bool _updateFrustrum(semantic_map_iterator& it, const ViewFrustrum& frustrum);

			semantic_map_t m_semanticVariables;
			std::vector<std::unique_ptr<ICBVariable>> m_constantVariables;
			std::map<std::string, dx_ptr<ID3D11SamplerState>> m_samplers;
			std::map<std::string, dx_ptr<ID3D11ShaderResourceView>> m_textures;
			std::map<std::string, dx_ptr<ID3D11UnorderedAccessView>> m_uavs;
			std::vector<std::string> m_sharedTextures;
			std::vector<std::string> m_sharedUavs;
			std::map<std::string, ICBVariable*> m_variableNames;
			std::map<std::string, RenderTargetsEffect> m_renderTargets;
		};
	}
}
