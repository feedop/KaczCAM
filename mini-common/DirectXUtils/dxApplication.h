#pragma once

#include "windowApplication.h"
#include "dxDevice.h"
#include "clock.h"
#include "effect.h"

namespace mini::directx
{
	class dx_app : public windows::window_app
	{
	public:
		static constexpr std::wstring_view s_window_title{ L"DirectX Window" };

		explicit dx_app(HINSTANCE instance,
			int window_width = s_default_window_width,
			int window_height = s_default_window_height,
			std::wstring_view window_title = s_window_title);

	protected:
		[[nodiscard]] int main_loop() override;

		[[nodiscard]] constexpr utils::clock const &clock() const noexcept { return m_clock; }
		virtual void render();
		virtual void update(utils::clock const &) { }

		[[nodiscard]] const RenderTargetsEffect& window_target() const { return m_renderTarget; }

		template<typename T, size_t N = 1>
		ConstantBuffer<T, N> create_buffer()
		{
			return m_device.CreateConstantBuffer<T, N>();
		}

		template<typename T>
		ConstantBuffer<T> create_buffer(const T& data)
		{
			auto buff = create_buffer<T>();
			update_buffer(buff, data);
			return buff;
		}

		template<typename T, size_t N>
		ConstantBuffer<T, N> create_buffer(const T (&data)[N])
		{
			auto buff = create_buffer<T, N>();
			Update(buff, data);
			return buff;
		}

		template<typename T, size_t N>
		void update_buffer(ConstantBuffer<T, N>& buffer, typename ConstantBuffer<T, N>::cref_type data)
		{
			buffer.Update(m_device.context(), data);
		}

		DxDevice m_device;

	private:
		RenderTargetsEffect m_renderTarget;
		utils::clock m_clock;
	};
}
