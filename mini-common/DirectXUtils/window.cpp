#include "window.h"
#include "exceptions.h"

using namespace mini::windows;
using namespace mini::utils;

HWND window::create_handle(window_info const &info)
{
	RECT rect{ 0, 0, info.cx, info.cy };
	if (info.cx != CW_USEDEFAULT && !AdjustWindowRectExForDpi(&rect, info.style, info.hMenu != nullptr, info.dwExStyle, GetDpiForSystem()))
		throw winapi_error{};
	auto hwnd = CreateWindowExW(info.dwExStyle, info.lpszClass, info.lpszName, info.style, info.x, info.y,
		rect.right - rect.left, rect.bottom - rect.top, info.hwndParent, info.hMenu, info.hInstance, info.lpCreateParams);
	if (!hwnd)
		throw winapi_error{};
	return hwnd;
}
void window::show(int show_command) noexcept
{
	ShowWindow(handle(), show_command);
}

RECT window::client_rectangle() const noexcept
{
	RECT r{};
	GetClientRect(handle(), &r);
	return r;
}

SIZE window::client_size() const noexcept
{
	auto r = client_rectangle();
	SIZE s{ r.right - r.left, r.bottom - r.top };
	return s;
}