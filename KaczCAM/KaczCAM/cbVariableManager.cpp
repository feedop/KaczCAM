#include "cbVariableManager.h"
#include "camera.h"
#include "viewFrustrum.h"
#include "dxDevice.h"
#include "movementManager.h"

using namespace std;
using namespace DirectX;
using namespace mini;
using namespace gk2;
using namespace directx;

bool CBVariableManager::_updateView(semantic_map_iterator& it, const XMMATRIX& viewMtx)
{
	if (!_update_M_MT<VariableSemantic::MatV>(it, viewMtx))
		return false;
	if (it->first < VariableSemantic::MatVP)
	{
		XMVECTOR det;
		XMMATRIX viewInvMtx = XMMatrixInverse(&det, viewMtx);
		return (_update_M_MT<VariableSemantic::MatVInv>(it, viewInvMtx) &&
			_updateVec4<VariableSemantic::Vec4CamRight>(it, viewInvMtx.r[0]) &&
			_updateVec4<VariableSemantic::Vec4CamUp>(it, viewInvMtx.r[1]) &&
			_updateVec4<VariableSemantic::Vec4CamDir>(it, viewInvMtx.r[2]) &&
			_updateVec4<VariableSemantic::Vec4CamPos>(it, viewInvMtx.r[3]));
	}
	return true;
}

bool CBVariableManager::_updateFrustrum(semantic_map_iterator& it, const ViewFrustrum& frustrum)
{
	if (it->first <= VariableSemantic::FloatFarPlane)
	{
		SIZE viewportSize = frustrum.viewportSize();
		return _updateVec2<VariableSemantic::Vec2ViewportDims>(it, XMFLOAT2{ static_cast<float>(viewportSize.cx), static_cast<float>(viewportSize.cy) }) &&
			_updateFloat<VariableSemantic::FloatFOV>(it, frustrum.fov()) &&
			_updateFloat<VariableSemantic::FloatNearPlane>(it, frustrum.nearPlane()) &&
			_updateFloat<VariableSemantic::FloatFarPlane>(it, frustrum.farPlane());
	}
	return true;
}

void CBVariableManager::UpdateView(const directx::camera& camera, const ViewFrustrum& frustrum)
{
	auto viewRelated = m_semanticVariables.begin();
	if (m_semanticVariables.empty() || viewRelated->first >= VariableSemantic::MatP)
		return;
	XMMATRIX viewMtx = camera.view_matrix();
	if (!_updateView(viewRelated, viewMtx))
		return;
	if (viewRelated->first < VariableSemantic::MatP)
		_update_M_MT_MI_MIT<VariableSemantic::MatVP>(viewRelated, viewMtx * frustrum.getProjectionMatrix());
}

void CBVariableManager::UpdateFrustrum(const ViewFrustrum& frustrum, const directx::camera & camera)
{
	auto projRelated = m_semanticVariables.lower_bound(VariableSemantic::MatVP);
	if (projRelated == m_semanticVariables.end() || projRelated->first >= VariableSemantic::MatM)
		return;
	if (projRelated->first < VariableSemantic::Vec2ViewportDims)
	{
		XMMATRIX projMtx = frustrum.getProjectionMatrix();
		if (projRelated->first < VariableSemantic::MatP &&
			!_update_M_MT_MI_MIT<VariableSemantic::MatVP>(projRelated, camera.view_matrix() * projMtx))
			return;
		if (!_update_M_MT_MI_MIT<VariableSemantic::MatP>(projRelated, projMtx))
			return;
	}
	_updateFrustrum(projRelated, frustrum);
}

void CBVariableManager::UpdateViewAndFrustrum(const directx::camera & camera, const ViewFrustrum& frustrum)
{
	auto it = m_semanticVariables.begin();
	if (m_semanticVariables.empty() || it->first >= VariableSemantic::MatM)
		return;
	if (it->first < VariableSemantic::MatP)
	{
		XMMATRIX viewMtx = camera.view_matrix();
		if (!_updateView(it, viewMtx))
			return;
		if (it->first < VariableSemantic::Vec2ViewportDims)
		{
			XMMATRIX projMtx = frustrum.getProjectionMatrix();
			if (it->first < VariableSemantic::MatP &&
				!_update_M_MT_MI_MIT<VariableSemantic::MatVP>(it, viewMtx * projMtx))
				return;
			if (!_update_M_MT_MI_MIT<VariableSemantic::MatP>(it, projMtx))
				return;
		}
		_updateFrustrum(it, frustrum);
		return;
	}
	if (!_update_M_MT_MI_MIT<VariableSemantic::MatP>(it, frustrum.getProjectionMatrix()))
		return;
	_updateFrustrum(it, frustrum);
}

void CBVariableManager::UpdateFrame(const dx_ptr<ID3D11DeviceContext>& context, const std::vector<std::unique_ptr<MovementManager>>& managers, const utils::clock& clock)
{
	for (auto& rt : m_renderTargets)
		rt.second.ClearRenderTargets(context);

	// Height texture swap
	auto heightRelated = m_semanticVariables.lower_bound(VariableSemantic::BoolSwap);
	if (heightRelated != m_semanticVariables.end())
		_updateBool<VariableSemantic::BoolSwap>(heightRelated);

	// Duck movement
	auto movementRelated = m_semanticVariables.lower_bound(VariableSemantic::Vec2Managed);
	if (movementRelated != m_semanticVariables.end() && managers.size() > 0)
	{
		auto& duckPosition = managers[0]->GetPosition();
		XMFLOAT3 duckPositionF;
		XMStoreFloat3(&duckPositionF, duckPosition);
		_updateVec2<VariableSemantic::Vec2Managed>(movementRelated, { duckPositionF.x, duckPositionF.z});
	}
		

	// Clock related
	auto clockRelated = m_semanticVariables.lower_bound(VariableSemantic::FloatDT);
	if (clockRelated == m_semanticVariables.end())
		return;
	if (clockRelated->first <= VariableSemantic::FloatT)
	{
		float dt = static_cast<float>(clock.frame_time());
		if (!(_updateFloat<VariableSemantic::FloatDT>(clockRelated, dt) &&
			_incrementFloat<VariableSemantic::FloatT>(clockRelated, dt)))
			return;
	}
	if (_updateFloat<VariableSemantic::FloatFPS>(clockRelated, static_cast<float>(clock.fps())))
		_incrementFloat<VariableSemantic::FloatTotalFrames>(clockRelated);

}

void CBVariableManager::UpdateModel(const Model::NodeIterator& modelPart)
{
	auto modelRelated = m_semanticVariables.lower_bound(VariableSemantic::MatM);
	if (modelRelated == m_semanticVariables.end() || modelRelated->first >= VariableSemantic::MatMVPInvT)
		return;
	XMMATRIX modelMtx = XMLoadFloat4x4(&modelPart.transform());
	if (modelRelated->first <= VariableSemantic::MatMInvT && !_update_M_MT_MI_MIT<VariableSemantic::MatM>(modelRelated, modelMtx))
			return;
	if (modelRelated->first <= VariableSemantic::MatMVPInvT)
	{
		XMMATRIX mvMtx = modelMtx * XMLoadFloat4x4(&_getSemanticVariable<XMFLOAT4X4, VariableSemantic::MatV>().value);
		if (modelRelated->first <= VariableSemantic::MatMVInvT && !_update_M_MT_MI_MIT<VariableSemantic::MatMV>(modelRelated, mvMtx))
			return;
		if (modelRelated->first <= VariableSemantic::MatMVPInvT)
		{
			XMMATRIX mvpMtx = mvMtx * XMLoadFloat4x4(&_getSemanticVariable<XMFLOAT4X4, VariableSemantic::MatP>().value);
			_update_M_MT_MI_MIT<VariableSemantic::MatMVP>(modelRelated, mvpMtx);
		}
	}
}

void CBVariableManager::AddSampler(const DxDevice& device, const string& name, const directx::sampler_info& desc)
{
	m_samplers.emplace(name, device.CreateSamplerState(desc));
}

void CBVariableManager::AddTexture(const DxDevice& device, const string& name, const wstring& file)
{
	m_textures.emplace(name, device.CreateShaderResourceView(file));
}

void CBVariableManager::AddTexture(const DxDevice& device, const std::string& name, const directx::tex2d_info& desc)
{
	auto texture = device.CreateTexture(desc);
	m_textures.emplace(name, device.CreateShaderResourceView(texture));
}

void CBVariableManager::AddTexture(const DxDevice& device, const std::string& name, const dx_ptr<ID3D11Texture2D>& texture)
{
	m_textures.emplace(name, device.CreateShaderResourceView(texture));
}

void CBVariableManager::AddRWTexture(const DxDevice& device, const std::string& name, const directx::tex2d_info& desc)
{
	auto texture = device.CreateTexture(desc);
	m_textures.emplace(name, device.CreateShaderResourceView(texture));

	unordered_access_view_info descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	m_uavs.emplace(name + "UAV", device.CreateUnorderedAccessView(texture, &descUAV));
	m_sharedTextures.push_back(name);
	m_sharedUavs.push_back(name + "UAV");
}

void CBVariableManager::AddUAV(const DxDevice& device, const std::string& name, const directx::tex2d_info& desc)
{
	auto texture = device.CreateTexture(desc);

	unordered_access_view_info descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_R32_FLOAT;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	m_uavs.emplace(name + "UAV", device.CreateUnorderedAccessView(texture, &descUAV));
	m_sharedUavs.push_back(name + "UAV");
}

void CBVariableManager::AddRenderableTexture(const DxDevice& device, const std::string& name, const directx::tex2d_info& desc)
{
	auto texture = device.CreateTexture(desc);
	m_textures.emplace(name, device.CreateShaderResourceView(texture));
	SIZE s{ static_cast<LONG>(desc.Width), static_cast<LONG>(desc.Height) };
	m_renderTargets.emplace(name, RenderTargetsEffect(directx::viewport{ s }, device.CreateDepthStencilView(s.cx, s.cy), device.CreateRenderTargetView(texture)));
}

CBVariable<XMFLOAT4X4>* CBVariableManager::_addSemanticMatrixVariable(VariableSemantic semantic)
{
	auto var = _addSemanticVariable<XMFLOAT4X4>(semantic);
	if (semantic >= VariableSemantic::MatMV && semantic <= VariableSemantic::MatMVPInvT)
	{
		if (m_semanticVariables.find(VariableSemantic::MatV) == m_semanticVariables.end())
			_addSemanticVariable<XMFLOAT4X4>(VariableSemantic::MatV);
		if (semantic >= VariableSemantic::MatMVP && m_semanticVariables.find(VariableSemantic::MatP) == m_semanticVariables.end())
			_addSemanticVariable<XMFLOAT4X4>(VariableSemantic::MatP);
	}
	return var;
}

ICBVariable* CBVariableManager::_addSemanticVariable(VariableSemantic semantic)
{
	if (_isMatrixSemantic(semantic))
		return _addSemanticMatrixVariable(semantic);
	if (_isVec4Semantic(semantic))
		return _addSemanticVariable<XMFLOAT4>(semantic);
	if (_isVec2Semantic(semantic))
		return _addSemanticVariable<XMFLOAT2>(semantic);
	if (_isFloatSemantic(semantic))
		return _addSemanticVariable<float>(semantic);
	if (_isBoolSemantic(semantic))
		return _addSemanticVariable<bool>(semantic);
	return nullptr;
}

ICBVariable* CBVariableManager::_addOrGetSemanticVariable(VariableSemantic semantic)
{
	auto it = m_semanticVariables.find(semantic);
	if (it != m_semanticVariables.end())
		return it->second.get();
	return _addSemanticVariable(semantic);
}

void CBVariableManager::AddSemanticVariable(const string& name, VariableSemantic semantic)
{
	if (m_variableNames.find(name) != m_variableNames.end())
		return;
	auto var = _addOrGetSemanticVariable(semantic);
	if (var == nullptr)
		return;
	m_variableNames.emplace(name, var);
}

const dx_ptr<ID3D11SamplerState>& CBVariableManager::GetSampler(const string& name) const
{
	auto it = m_samplers.find(name);
	assert(it != m_samplers.end());
	return it->second;
}

const dx_ptr<ID3D11ShaderResourceView>& CBVariableManager::GetTexture(const string& name) const
{
	auto it = m_textures.find(name);
	assert(it != m_textures.end());
	return it->second;
}

const dx_ptr<ID3D11UnorderedAccessView>& CBVariableManager::GetUAV(const string& name) const
{
	auto it = m_uavs.find(name);
	assert(it != m_uavs.end());
	return it->second;
}

const RenderTargetsEffect& CBVariableManager::GetRenderTarget(const std::string& name) const
{
	auto it = m_renderTargets.find(name);
	assert(it != m_renderTargets.end());
	return it->second;
}

