#include "sceneBase.h"
#include "model.h"
#include "windowsx.h"

using namespace std;
using namespace DirectX;
using namespace mini;
using namespace gk2;

#pragma comment(lib, "winmm.lib")

class CopyTextureEffectBase : public EffectComponent
{
public:
	using EffectComponent::EffectComponent;
	using EffectComponent::operator=;

	void Begin(const dx_ptr<ID3D11DeviceContext>& context) const override
	{
		context->CopyResource(_getDestination(context).get(), _getSource(context).get());
	}

protected:
	virtual dx_ptr<ID3D11Resource> _getSource(const dx_ptr<ID3D11DeviceContext>& context) const = 0;
	virtual dx_ptr<ID3D11Resource> _getDestination(const dx_ptr<ID3D11DeviceContext>& context) const = 0;
};

class CopyIntoTextureEffectBase : public CopyTextureEffectBase
{
public:
	CopyIntoTextureEffectBase() = default;

	CopyIntoTextureEffectBase(const dx_ptr<ID3D11Texture2D>& destination)
		: m_destination{ destination.get() }
	{
		m_destination->AddRef();
	}

	template<typename TView, typename TVoid = std::enable_if_t<std::is_base_of_v<ID3D11View, TView>>>
	CopyIntoTextureEffectBase(const dx_ptr<TView>& dstView)
	{
		ID3D11Resource* prs = nullptr;
		static_cast<ID3D11View*>(dstView.get())->GetResource(&prs);
		m_destination.reset(prs);
	}

	CopyIntoTextureEffectBase(CopyIntoTextureEffectBase&& other) = default;
	CopyIntoTextureEffectBase& operator=(CopyIntoTextureEffectBase&& other) = default;

	void SetDestination(const dx_ptr<ID3D11Texture2D>& destination) noexcept
	{
		m_destination.reset(destination.get());
		m_destination->AddRef();
	}

	template<typename TView>
	std::enable_if_t<std::is_base_of_v<ID3D11View, TView>> SetDestination(const dx_ptr<TView>& dstView)
	{
		ID3D11Resource* prs = nullptr;
		static_cast<ID3D11View*>(dstView.get())->GetResource(prs);
		m_destination.reset(prs);
	}

protected:
	dx_ptr<ID3D11Resource> _getDestination(const dx_ptr<ID3D11DeviceContext>&) const override
	{
		return clone(m_destination);
	}

private:
	dx_ptr<ID3D11Resource> m_destination;
};

class CopyRenderTargetEffect : public CopyIntoTextureEffectBase
{
public:
	using CopyIntoTextureEffectBase::CopyIntoTextureEffectBase;
	using CopyIntoTextureEffectBase::operator=;

protected:
	dx_ptr<ID3D11Resource> _getSource(const dx_ptr<ID3D11DeviceContext>& context) const override
	{
		ID3D11RenderTargetView* prtv = nullptr;
		context->OMGetRenderTargets(1, &prtv, nullptr);
		dx_ptr<ID3D11RenderTargetView> rtv{ prtv };
		if (!rtv)
			throw utils::custom_error{ L"Failed to obtain render target view" };

		ID3D11Resource* prs = nullptr;
		rtv->GetResource(&prs);
		return dx_ptr<ID3D11Resource>{ prs };
	}
};

class CopyDephtBufferEffect : public CopyIntoTextureEffectBase
{
public:
	using CopyIntoTextureEffectBase::CopyIntoTextureEffectBase;
	using CopyIntoTextureEffectBase::operator=;

protected:
	dx_ptr<ID3D11Resource> _getSource(const dx_ptr<ID3D11DeviceContext>& context) const override
	{
		ID3D11DepthStencilView* pdsv = nullptr;
		context->OMGetRenderTargets(0, nullptr, &pdsv);
		dx_ptr<ID3D11DepthStencilView> dsv{ pdsv };
		if (!dsv)
			throw utils::custom_error{ L"Failed to obtain render target view" };

		ID3D11Resource* prs = nullptr;
		dsv->GetResource(&prs);
		return dx_ptr<ID3D11Resource>{ prs };
	}
};

SceneBase::SceneBase(HINSTANCE hInst)
	: dx_app(hInst, 1280, 720, L"KaczCAM"), m_loader(m_device), m_layouts(m_device), m_camera(0.01f, 50.0f, 5),
	  m_frustrum(get_window().client_size(), XM_PIDIV4, 0.5f, 85.0f)
{
}

std::optional<LRESULT> SceneBase::process_message(windows::message const &msg)
{
	static bool hasCapture = false;
	static POINT lastPos{ 0,0 };
	switch(msg.type)
	{
	case WM_KEYDOWN:
		switch (msg.w_param)
		{
		case VK_UP:
			for (auto&& manager : m_movementManagers)
			{
				if (manager->SpeedUp())
					overdriveOn();
			}
			break;
		case VK_DOWN:
			for (auto&& manager : m_movementManagers)
				manager->SlowDown();
			overdriveOff();
		}
		break;
	case WM_MBUTTONDOWN:	
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if (!hasCapture)
		{
			hasCapture = true;
			SetCapture(get_window().handle());
		}
		break;
	case WM_MBUTTONUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		if (hasCapture && (msg.w_param & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON))==0)
		{
			hasCapture = false;
			ReleaseCapture();
		}
		break;
	case WM_MOUSEMOVE:
		{
			POINT pos{ GET_X_LPARAM(msg.l_param), GET_Y_LPARAM(msg.l_param) };
			if (msg.w_param & MK_LBUTTON)
				m_camera.rotate(static_cast<float>(pos.y - lastPos.y) * ROTATION_SPEED,
					static_cast<float>(pos.x - lastPos.x) * ROTATION_SPEED);
			else if (msg.w_param & MK_RBUTTON)
				m_camera.zoom(static_cast<float>(pos.y - lastPos.y) * ZOOM_SPEED);
			if (msg.w_param & (MK_LBUTTON | MK_RBUTTON))
				m_variables.UpdateView(m_camera, m_frustrum);
			lastPos = pos;
		}
		break;
	default:
		return dx_app::process_message(msg);
	}
	return 0;
}

int SceneBase::main_loop()
{
	m_variables.UpdateViewAndFrustrum(m_camera, m_frustrum);
	return dx_app::main_loop();
}

void SceneBase::update(utils::clock const &clock)
{
	for (auto&& manager : m_movementManagers)
	{
		manager->Update(clock);
		auto& transform = manager->GetModelMtx();
		XMFLOAT4X4 modelMtx;
		XMStoreFloat4x4(&modelMtx, transform);
		model(manager->GetModelId()).setTransform(modelMtx);
	}
	m_variables.UpdateFrame(m_device.context(), m_movementManagers, clock);
}

void SceneBase::render()
{
	auto& rt = window_target();
	float clearColor[4] = { 0.5f, 0.5f, 1.0f, 0.0f };
	rt.ClearRenderTargets(m_device.context(), clearColor);
	rt.Begin(m_device.context());

	// Prepare compute passes
	{
		for (auto&& p : m_computePasses)
		{
			ScopedBindUav _(*this);
			p.Execute(m_device.context(), m_variables, true);
		}
	}
	{
		ScopedBindTexture _(*this);
		// Rendering passes
		for (auto&& p : m_passes)
		{
			p.Execute(m_device.context(), m_variables);
		}
	}
}

size_t SceneBase::addModelFromFile(const std::string& path, bool customFormat)
{
	if (customFormat)
	{
		m_models.push_back(make_unique<Model>(m_loader.LoadFromCustomFile(path, m_layouts)));
	}
	else
	{
		m_models.push_back(make_unique<Model>(m_loader.LoadFromFile(path, m_layouts)));
	}
	
	return m_models.size() - 1;
}

size_t SceneBase::addModelFromString(const std::string& model, bool smoothNormals)
{
	m_models.push_back(make_unique<Model>(m_loader.LoadFromString(model, m_layouts, smoothNormals)));
	return m_models.size() - 1;
}

size_t SceneBase::addPass(const std::wstring& vsShader, const std::wstring& psShader)
{
	m_passes.emplace_back(m_device, m_variables, &m_layouts, vsShader, psShader);
	return m_passes.size() - 1;
}

size_t SceneBase::addPass(const std::wstring& vsShader, const std::wstring& gsShader,
	const std::wstring& psShader)
{
	m_passes.emplace_back(m_device, m_variables, &m_layouts, vsShader, gsShader, psShader);
	return m_passes.size() - 1;
}

size_t SceneBase::addPass(const std::wstring& vsShader, const std::wstring& psShader,
	const std::string& renderTarget, bool clearRenderTarget)
{
	m_passes.emplace_back(m_device, m_variables, &m_layouts, m_variables.GetRenderTarget(renderTarget),
		clearRenderTarget, vsShader, psShader);
	return m_passes.size() - 1;
}
size_t SceneBase::addPass(const std::wstring& vsShader, const std::wstring& psShader,
	const RenderTargetsEffect& renderTarget, bool clearRenderTarget)
{
	m_passes.emplace_back(m_device, m_variables, &m_layouts, renderTarget, clearRenderTarget, vsShader, psShader);
	return m_passes.size() - 1;
}

size_t SceneBase::addPass(const std::wstring& csShader)
{
	m_computePasses.emplace_back(m_device, m_variables, &m_layouts, csShader);
	return m_computePasses.size() - 1;
}

void SceneBase::addRasterizerState(size_t passId, const directx::rasterizer_info& desc)
{
	m_passes[passId].AddEffect(make_unique<RasterizerEffect>(m_device.CreateRasterizerState(desc)));
}

void SceneBase::addModelToPass(size_t passId, size_t modelId)
{
	m_passes[passId].AddModel(m_models[modelId].get());
}

void SceneBase::copyRenderTarget(size_t passId, std::string dstTexture)
{
	pass(passId).EmplaceEffect<CopyRenderTargetEffect>(m_variables.GetTexture(dstTexture));
}

void SceneBase::copyDepthBuffer(size_t passId, std::string dstTexture)
{
	pass(passId).EmplaceEffect<CopyDephtBufferEffect>(m_variables.GetTexture(dstTexture));
}

void SceneBase::overdriveOn()
{
	PlaySound(TEXT("audio/Initial D - Running in The 90s.wav"), nullptr, SND_FILENAME | SND_ASYNC);
}

void SceneBase::overdriveOff()
{
	PlaySound(nullptr, nullptr, 0);
}

SceneBase::ScopedBindUav::ScopedBindUav(const SceneBase& base) : m_base(base)
{
	for (auto&& tex : m_base.m_variables.GetSharedUavs())
	{
		auto texuav = m_base.m_variables.GetUAV(tex).get();
		m_base.m_device.context()->CSSetUnorderedAccessViews(0, 1, &texuav, 0);
	}
}

SceneBase::ScopedBindUav::~ScopedBindUav()
{
	int i = 0;
	for (auto&& tex : m_base.m_variables.GetSharedUavs())
	{
		ID3D11UnorderedAccessView* nullUav = nullptr;
		m_base.m_device.context()->CSSetUnorderedAccessViews(i++, 1, &nullUav, 0);
	}
}

SceneBase::ScopedBindTexture::ScopedBindTexture(const SceneBase& base) : m_base(base)
{
	for (auto&& tex : m_base.m_variables.GetSharedTextures())
	{
		auto texuav = m_base.m_variables.GetTexture(tex).get();
		m_base.m_device.context()->PSSetShaderResources(0, 1, &texuav);
	}
}

SceneBase::ScopedBindTexture::~ScopedBindTexture()
{
	int i = 0;
	for (auto&& tex : m_base.m_variables.GetSharedTextures())
	{
		ID3D11ShaderResourceView* nullSrv = nullptr;
		m_base.m_device.context()->PSSetShaderResources(i++, 1, &nullSrv);
	}
}
