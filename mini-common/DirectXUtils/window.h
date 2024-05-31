#pragma once

#include <memory>
#include <string>
#include <Windows.h>

namespace mini::windows
{
	struct hwnd_deleter
	{
		void operator()(HWND handle) const
		{
			if (handle)
				DestroyWindow(handle);
		}
	};

	using window_handle = std::unique_ptr<std::remove_pointer_t<HWND>, hwnd_deleter>;

	class window;

	struct window_info : CREATESTRUCTW
	{
		constexpr window_info(
			HINSTANCE instance,
			std::wstring_view class_name,
			std::wstring_view title,
			int x = CW_USEDEFAULT,
			int y = 0,
			int width = CW_USEDEFAULT,
			int height = 0,
			LONG style = WS_OVERLAPPEDWINDOW,
			DWORD ex_style = WS_EX_OVERLAPPEDWINDOW,
			HMENU menu = nullptr,
			window const *parent = nullptr,
			LPVOID create_param = nullptr
		);

		constexpr window_info(
			HINSTANCE instance,
			std::wstring_view class_name,
			std::wstring_view title,
			window const &parent,
			int x = CW_USEDEFAULT,
			int y = 0,
			int width = CW_USEDEFAULT,
			int height = 0,
			LONG style = WS_OVERLAPPEDWINDOW,
			DWORD ex_style = WS_EX_OVERLAPPEDWINDOW,
			HMENU menu = nullptr,
			LPVOID create_param = nullptr
		)
			: window_info(instance, class_name, title, x, y, width, height, style, ex_style, menu, &parent, create_param)
		{ }

		template<typename T>
		constexpr window_info(
			T &create_param,
			HINSTANCE instance,
			std::wstring_view class_name,
			std::wstring_view title,
			int x = CW_USEDEFAULT,
			int y = 0,
			int width = CW_USEDEFAULT,
			int height = 0,
			LONG style = WS_OVERLAPPEDWINDOW,
			DWORD ex_style = WS_EX_OVERLAPPEDWINDOW,
			HMENU menu = nullptr,
			window const *parent = nullptr
		)
			: window_info(instance, class_name, title, x, y, width, height, style, ex_style, menu, parent, &create_param)
		{ }

		template<typename T>
		constexpr window_info(
			T &create_param,
			HINSTANCE instance,
			std::wstring_view class_name,
			std::wstring_view title,
			window const &parent,
			int x = CW_USEDEFAULT,
			int y = 0,
			int width = CW_USEDEFAULT,
			int height = 0,
			LONG style = WS_OVERLAPPEDWINDOW,
			DWORD ex_style = WS_EX_OVERLAPPEDWINDOW,
			HMENU menu = nullptr
		)
			: window_info{
				instance, class_name, title, x, y, width, height, style, ex_style, menu, &parent, &create_param
			}
		{ }
	};

	/**********************************************************************//*!
	 * Wraps a standard WINAPI window details.
	 * @sa https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms633573(v=vs.85).aspx
	 *************************************************************************/
	struct message
	{
		UINT type;	/*!< The details.					   */
		WPARAM w_param;	/*!< Additional details information.   */
		LPARAM l_param;	/*!< Additional details information.   */
	};

	/**********************************************************************//*!
	 * @brief WINAPI window.
	 *
	 * Represents an empty window that can be used for DirectX drawing.
	 *************************************************************************/
	class window
	{
	public:
		/******************************************************************//*!
		 * Creates window with given properties
		 * @param [in] info Window properties.
		 * @throws WinAPIException Window could not be created.
		 * @remark Window class with a given name must already be registered.
		 *********************************************************************/
		window(window_info const &info)
			: m_handle{ create_handle(info) }
		{ }

		window(const window &other) = delete;

		/******************************************************************//*!
		 * Destroys the window
		 *********************************************************************/
		virtual ~window() = default;

		/******************************************************************//*!
		 * Sets window's show state.
		 * @param [in] show_command Specifies how the window is to be shown
		 * @sa https://msdn.microsoft.com/en-us/library/ms633548(v=vs.85).aspx
		 *********************************************************************/
		virtual void show(int show_command) noexcept;
		/******************************************************************//*!
		 * Retrieves size of the window's client area.
		 * @returns SIZE structure containing width and height of the
		 * window's client area.
		 *********************************************************************/
		[[nodiscard]] SIZE client_size() const noexcept;
		/******************************************************************//*!
		 * Retrieves coordinates of the window's client area.
		 * @returns RECT structure containing top, bottom, left and right
		 * coordinates of window's client area. Top and left are always 0.
		 * Right and bottom contain width and height of the client area.
		 *********************************************************************/
		[[nodiscard]] RECT client_rectangle() const noexcept;
		/******************************************************************//*!
		 * Retrieves WinAPI handle for this window.
		 * @returns HWND handle of this window.
		 *********************************************************************/
		[[nodiscard]] constexpr HWND handle() const noexcept
		{
			return m_handle.get();
		}

		/******************************************************************//*!
		 * Disassociates the object with WinAPI window handle.
		 * @returns HWND handle previously associated with this window.
		 *********************************************************************/
		constexpr HWND release_handle() noexcept
		{
			return m_handle.release();
		}

		/******************************************************************//*!
		 * Associates the object with new WinAPI window handle.
		 * @param handle New window handle.
		 * @remark Previously associated WinAPI window handle (if any)
		 * will be destroyed.
		 *********************************************************************/
		constexpr void reset_handle(HWND handle) noexcept
		{
			m_handle.reset(handle);
		}

	private:
		static HWND create_handle(window_info const &info);

		/// Handle of this window instance
		window_handle m_handle;
	};

	constexpr window_info::window_info(
		HINSTANCE instance,
		std::wstring_view class_name,
		std::wstring_view title,
		int x,
		int y,
		int width,
		int height,
		LONG style,
		DWORD ex_style,
		HMENU menu,
		window const *parent,
		LPVOID create_param)
		: CREATESTRUCTW{
			.lpCreateParams = create_param,
			.hInstance = instance,
			.hMenu = menu,
			.hwndParent = parent ? parent->handle() : nullptr,
			.cy = height,
			.cx = width,
			.y = y,
			.x = x,
			.style = style,
			.lpszName = title.data(),
			.lpszClass = class_name.data(),
			.dwExStyle = ex_style
		}
	{ }

}