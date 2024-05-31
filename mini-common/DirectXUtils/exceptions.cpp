#include "exceptions.h"

#include <cassert>
#include <format>

using namespace mini::utils;
using std::wstring;

std::wstring exception::message() const noexcept
{
	return std::format(L"Error code: {}", exit_code());
}

std::wstring exception::details() const noexcept
{
	try {
		auto loc = std::format(
			"Location: {} in {}({}:{})\n\n{}",
			m_location.function_name(),
			m_location.file_name(),
			m_location.line(),
			m_location.column(),
			to_string(m_trace));
		wstring wloc;
		assert(loc.size() <= std::numeric_limits<int>::max());
		auto len =
			MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, loc.data(),
				static_cast<int>(loc.size()), nullptr, 0);
		wloc.resize(len);
		MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, loc.data(),
			static_cast<int>(loc.size()), wloc.data(), len);
		return wloc;
	}
	catch (...) {}
	return {};
}

wstring winapi_error::message() const noexcept
{
	wstring message;
	try
	{
		LPWSTR buffer;
		if (FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, m_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPWSTR>(&buffer), 0, nullptr))
		{
			message = buffer;
			LocalFree(buffer);
		}
	}
	catch (...) {}
	return message;
}

custom_error::custom_error(const std::wstring &message, std::source_location location, std::stacktrace trace) noexcept
	: exception{ std::move(location), std::move(trace) }
{
	try {
		m_message = message;
	}
	catch (...) {}
}
