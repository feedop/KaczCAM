#include "windowApplication.h"
#include "exceptions.h"
#include "scope_guard.h"

using namespace mini::windows;
using namespace mini::utils;

window_app::window_app(HINSTANCE instance, int window_width, int window_height, std::wstring_view window_title)
	: m_instance{ instance }, m_window{ {
			*this, instance, register_class(instance), window_title, CW_USEDEFAULT, 0, window_width, window_height,
			WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION, 0
		} }
{ }

int window_app::main_loop()
{
	MSG msg{};
	BOOL res;
	while ((res = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (res == -1)
			throw winapi_error{};
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK window_app::window_proc(HWND handle, UINT msg, WPARAM w_param, LPARAM l_param) noexcept
{
	window_app *app;
	constexpr UINT mb_flags = MB_OK;
	if (msg == WM_NCCREATE)
	{
		auto pcs = reinterpret_cast<LPCREATESTRUCT>(l_param);
		app = static_cast<window_app *>(pcs->lpCreateParams);
		SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
		app->m_window.reset_handle(handle);
	}
	else
	{
		app = reinterpret_cast<window_app *>(GetWindowLongPtrW(handle, GWLP_USERDATA));
	}
	LRESULT res = EXIT_FAILURE;
	//Windows likes to eat exceptions that leak through callbacks on some platforms.
	try
	{
		if (app)
		{
			auto destroy = make_guard([app, msg]() {
				if (msg == WM_NCDESTROY)
				{
					SetWindowLongPtrW(app->m_window.handle(), GWLP_USERDATA, 0);
					app->m_window.release_handle();
				}
				});
			return app->window_proc({ msg, w_param, l_param });
		}
		return DefWindowProcW(handle, msg, w_param, l_param);
	}
	catch (exception &e)
	{
		MessageBoxW(nullptr, e.details().c_str(), e.message().c_str(), mb_flags);
		res = e.exit_code();
	}
	catch (std::exception &e)
	{
		std::string s(e.what());
		MessageBoxW(nullptr, std::wstring(s.begin(), s.end()).c_str(), L"Błąd", mb_flags);
	}
	catch (const char *str)
	{
		std::string s(str);
		MessageBoxW(nullptr, std::wstring(s.begin(), s.end()).c_str(), L"Błąd", mb_flags);
	}
	catch (const wchar_t *str)
	{
		MessageBoxW(nullptr, str, L"Błąd", mb_flags);
	}
	catch (...)
	{
		MessageBoxW(nullptr, L"Nieznany Błąd", L"Błąd", mb_flags);
	}
	PostQuitMessage(static_cast<int>(res));
	return res;
}

LRESULT window_app::window_proc(message const &m)
{
	switch (m.type)
	{
	case WM_DESTROY:
		//If the app and its main window are destroyed during stack unwinding due to an exception
		//we either have not entered the main details loop yet or the exception has caused the main_loop
		//function to end. Sending the quit details in such case is pointless and will cause any modal
		//details loops (e.g. inside MessageBoxW) to end prematurely.
		if (!std::uncaught_exceptions())
			PostQuitMessage(EXIT_SUCCESS);
		break;
	default:
		if (auto const result = process_message(m))
			return result.value();
		if (m.type == WM_PAINT)
		{
			PAINTSTRUCT ps;
			BeginPaint(m_window.handle(), &ps);
			EndPaint(m_window.handle(), &ps);
			break;
		}
		return DefWindowProcW(m_window.handle(), m.type, m.w_param, m.l_param);
	}
	return 0;
}

std::wstring_view window_app::register_class(HINSTANCE instance)
{
	WNDCLASSEXW c{ .cbSize = sizeof(WNDCLASSEXW) };
	if (!GetClassInfoEx(instance, s_class_name.data(), &c))
	{
		c = WNDCLASSEXW{
		   .cbSize = sizeof(WNDCLASSEXW),
		   .style = CS_HREDRAW | CS_VREDRAW,
		   .lpfnWndProc = window_proc,
		   .hInstance = instance,
		   .hCursor = LoadCursorW(nullptr, IDC_ARROW),
		   .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
		   .lpszClassName = s_class_name.data()
		};
		if (!RegisterClassExW(&c))
			throw winapi_error{};
	}
	return s_class_name;
}

int window_app::run(int show_command)
{
	m_window.show(show_command);
	return main_loop();
}
