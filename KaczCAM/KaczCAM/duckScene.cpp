#include "duckScene.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;
using namespace directx;
using namespace utils;

DuckScene::DuckScene(HINSTANCE hInst): SceneBase(hInst)
{
	//Shader Variables
	m_variables.AddSemanticVariable("modelMtx", VariableSemantic::MatM);
	m_variables.AddSemanticVariable("modelInvTMtx", VariableSemantic::MatMInvT);
	m_variables.AddSemanticVariable("viewProjMtx", VariableSemantic::MatVP);
	m_variables.AddSemanticVariable("camPos", VariableSemantic::Vec4CamPos);
	m_variables.AddSemanticVariable("time", VariableSemantic::FloatT);
	m_variables.AddSemanticVariable("swap", VariableSemantic::BoolSwap);

	XMFLOAT4 lightPos[1] = {
		/*{ -1.f, 0.0f, -3.5f, 1.f },*/
		{ 0.0f, 19.f, 0.0f, 1.0f } };
	XMFLOAT3 lightColor[1] = {
		/*{ 12.f, 9.f, 10.f },*/
		{ 1.0f, 1.0f, 1.0f } };
	m_variables.AddNamedVariable("lightPos", lightPos);
	m_variables.AddNamedVariable("lightColor", lightColor);
	m_variables.AddSampler(m_device, "samp");

	m_variables.AddTexture(m_device, "envMap", L"textures/cubemap.dds");
	m_variables.AddTexture(m_device, "duckTex", L"textures/ducktex.jpg");
	m_variables.AddSemanticVariable("mvpMtx", VariableSemantic::MatMVP);
	m_variables.AddSemanticVariable("duckPos", VariableSemantic::Vec2Managed);

	m_variables.AddRWTexture(m_device, "normTex", 256);
	m_variables.AddUAV(m_device, "heightTex0", 256);
	m_variables.AddUAV(m_device, "heightTex1", 256);

	//Models
	auto duck = addModelFromFile("models/duck.txt", true);

	float h0 = 1.5f;
	XMMATRIX transform = XMMatrixScaling(1.0f / 75.0f, 1.0f / 75.0f, 1.0f / 75.0f) * XMMatrixRotationY(XM_PI);
	addMovementManager<DuckMovement>(duck, transform);

	XMFLOAT4X4 modelMtx;
	XMStoreFloat4x4(&modelMtx, XMMatrixTranslation(0, -h0, 0));

	auto quad = addModelFromString(
		"pp 4\n1 0 1 0 1 0\n1 0 -1 0 1 0\n"
		"-1 0 -1 0 1 0\n-1 0 1 0 1 0\n");
	auto envModel =
		addModelFromString("hex 0 0 0 1.73205");

	XMStoreFloat4x4(&modelMtx,
					XMMatrixScaling(20, 20, 20));
	model(quad).addTransform(modelMtx);
	model(envModel).addTransform(modelMtx);

	//Render Passes
	auto passHeight = addPass(L"heightCS.cso");
	auto passNorm = addPass(L"normCS.cso");

	auto passDuck = addPass(L"duckVS.cso", L"duckPS.cso");
	addModelToPass(passDuck, duck);

	auto passEnv = addPass(L"envVS.cso", L"envPS.cso");
	addModelToPass(passEnv, envModel);
	addRasterizerState(passEnv, rasterizer_info(true));

	auto passWater = addPass(L"waterVS.cso", L"waterPS.cso");
	addModelToPass(passWater, quad);
	rasterizer_info rs;
	rs.CullMode = D3D11_CULL_NONE;
	addRasterizerState(passWater, rs);
}
