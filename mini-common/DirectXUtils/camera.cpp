#include "camera.h"

using namespace mini::directx;
using namespace DirectX;

XMMATRIX orbit_camera::view_matrix() const noexcept
{
	return XMMatrixTranslation(-m_target.x, -m_target.y, -m_target.z) * XMMatrixRotationY(-m_ay) *
		XMMatrixRotationX(-m_ax) * XMMatrixTranslation(0, 0, m_distance);
}

XMVECTOR fps_camera::forward() const noexcept
{
	return XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), XMMatrixRotationY(angle_x()));
}

XMVECTOR fps_camera::right() const noexcept
{
	return XMVector3TransformNormal(XMVectorSet(1, 0, 0, 0), XMMatrixRotationY(angle_x()));
}

void orbit_camera::move_target(FXMVECTOR delta_target) noexcept
{
	auto pos = XMLoadFloat4(&m_target);
	pos = pos + delta_target;
	XMStoreFloat4(&m_target, pos);
}

void orbit_camera::rotate(float d_ax, float d_ay) noexcept
{
	m_ax = XMScalarModAngle(m_ax + d_ax);
	m_ay = XMScalarModAngle(m_ay + d_ay);
}

XMFLOAT4 orbit_camera::camera_position() const noexcept
{
	if (m_distance == 0.0f)
		return m_target;
	XMMATRIX view_mtx = view_matrix();
	view_mtx = XMMatrixInverse(nullptr, view_mtx);
	//auto alt = XMMatrixTranslation(0.0f, 0.0f, -m_distance) * XMMatrixRotationY(m_ay) * XMMatrixRotationX(-m_ax);
	XMFLOAT3 res{ 0.0f, 0.0f, 0.0f };
	XMStoreFloat3(&res, XMVector3TransformCoord(XMLoadFloat3(&res), view_mtx));
	return { res.x, res.y, res.z, 1.0f };
}

