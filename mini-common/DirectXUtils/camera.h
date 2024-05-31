#pragma once

#include <DirectXMath.h>
#include <limits>

namespace mini::directx
{
	class camera
	{
	public:
		virtual ~camera() noexcept = default;
		[[nodiscard]] virtual DirectX::XMMATRIX view_matrix() const noexcept = 0;
	};

	class orbit_camera : public camera
	{
		constexpr void clamp_distance() noexcept
		{
			if (m_distance < m_d_min)
				m_distance = m_d_min;
			else if (m_distance > m_d_max)
				m_distance = m_d_max;
		}

	public:
		constexpr void distance_range(float min_distance, float max_distance) noexcept
		{
			if (max_distance < min_distance)
				max_distance = min_distance;
			m_d_min = min_distance;
			m_d_max = max_distance;
			clamp_distance();
		}

		explicit constexpr orbit_camera(DirectX::XMFLOAT3 target = { 0, 0, 0 }, float min_distance = 0.0f,
			float max_distance = std::numeric_limits<float>::max(), float distance = 0.0f) noexcept
			: m_ax{}, m_ay{}, m_distance{ distance }, m_target{ target.x, target.y, target.z, 1.0f }
		{
			distance_range(min_distance, max_distance);
		}

		explicit constexpr orbit_camera(float min_distance, float max_distance = std::numeric_limits<float>::max(),
			float distance = 0.0f) noexcept
			: orbit_camera{ { 0.0f, 0.0f, 0.0f }, min_distance, max_distance, distance }
		{ }

		[[nodiscard]] DirectX::XMMATRIX view_matrix() const noexcept override;
		[[nodiscard]] DirectX::XMFLOAT4 camera_position() const noexcept;

		void move_target(DirectX::XMFLOAT3 delta_target) noexcept { move_target(XMLoadFloat3(&delta_target)); }
		void move_target(DirectX::FXMVECTOR delta_target) noexcept;
		void rotate(float d_ax, float d_ay) noexcept;
		constexpr void zoom(float d_distance) noexcept
		{
			m_distance += d_distance;
			clamp_distance();
		}

		[[nodiscard]] constexpr float angle_y() const noexcept { return m_ax; }
		[[nodiscard]] constexpr float angle_x() const noexcept { return m_ay; }
		[[nodiscard]] constexpr float distance() const noexcept { return m_distance; }
		[[nodiscard]] constexpr DirectX::XMFLOAT4 target() const noexcept { return m_target; }

	private:

		float m_ax, m_ay;
		float m_distance, m_d_min, m_d_max;
		DirectX::XMFLOAT4 m_target;
	};

	class fps_camera : public orbit_camera
	{
	public:
		explicit fps_camera(DirectX::XMFLOAT3 target) noexcept
			: orbit_camera{ target, 0.0f, 0.0f }
		{ }

		using orbit_camera::move_target;
		using orbit_camera::rotate;
		using orbit_camera::angle_y;
		using orbit_camera::angle_x;
		using orbit_camera::target;
		using orbit_camera::view_matrix;

		/*Returns target's forward direction parallel to ground (XZ) plane*/
		[[nodiscard]] DirectX::XMVECTOR forward() const noexcept;
		/*Returns target's right direction parallel to ground (XZ) plane*/
		[[nodiscard]] DirectX::XMVECTOR right() const noexcept;
	};
}