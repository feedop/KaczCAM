#pragma once

#include <source_location>
#include <Windows.h>
#include <string>
#include <stacktrace>

namespace mini::utils
{
	/**********************************************************************//*!
	 * @brief Abstract base class for exceptions supporting wide string error
	 * messages.
	 *
	 * Base abstract class for exceptions used by this project. It stores
	 * a pointer to a C string describing the location of the error. User must
	 * make sure the exception doesn't outlive the C string. __AT__ macro can
	 * be used to generate string literals with that description (literals
	 * have global lifetime, hence they will outlive any exception.)
	 *************************************************************************/
	class exception
	{
	public:
		virtual ~exception() = default;

		/******************************************************************//*!
		 * Creates new exception with the given location description.
		 * @param location Source location of where the exception has occurred.
		 * @param trace Stack trace for the exception.
		 *********************************************************************/
		explicit exception(std::source_location location, std::stacktrace trace) noexcept
			: m_location{ std::move(location) }, m_trace{ std::move(trace) }
		{ }

		/******************************************************************//*!
		 * Retrieves the error message for this exception.
		 * @returns String containing a short message associated
		 *			with this exception
		 *********************************************************************/
		[[nodiscard]] virtual std::wstring message() const noexcept;

		/******************************************************************//*!
		 * Retrieves the error details for this exception.
		 * @returns String containing detailed description of the circumstances
		 *			surrounding the error.
		 *********************************************************************/
		[[nodiscard]] virtual std::wstring details() const noexcept;

		/******************************************************************//*!
		 * Retrieves the error code associated with this exception.
		 * @returns Code number of the error.
		 *********************************************************************/
		[[nodiscard]] virtual int exit_code() const noexcept = 0;

	private:
		/// Location of the error.
		std::source_location m_location;
		std::stacktrace m_trace;
	};

	/**********************************************************************//*!
	 * @brief Exception representing an error encountered by a system function
	 * call.
	 *
	 * exception class used to represent system errors. It stores a pointer to
	 * a C string describing the location of the error and the error code.
	 * User must make sure the exception doesn't outlive the location
	 * description. The error code should be one of System Error Codes to
	 * provide a meaningful message.
	 * \sa https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms679360(v=vs.85).aspx
	 *************************************************************************/
	class winapi_error : public exception
	{
	public:
		/******************************************************************//*!
		 * Creates new exception with the given location description and error
		 * code.
		 * @param location Wide C string describing the location of the error.
		 * User must make sure the Exception doesn't outlive the C string.
		 * __AT__ macro can be used to generate string literals with that
		 * description (literals have global lifetime, hence they will outlive
		 * any Exception.)
		 * @param error_code Error code associated with this exception. This
		 * should be one of System Error Codes to provide a meaningful
		 * message.
		 * \sa https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms679360(v=vs.85).aspx
		 *********************************************************************/
		explicit winapi_error(DWORD error_code = GetLastError(), std::source_location location = std::source_location::current(), std::stacktrace trace = std::stacktrace::current()) noexcept
			: exception{ std::move(location), std::move(trace) }, m_code{ error_code }
		{ }

		/******************************************************************//*!
		 * Creates new exception with the given location description and error
		 * code.
		 * @param location Wide C string describing the location of the error.
		 * User must make sure the Exception doesn't outlive the C string.
		 * __AT__ macro can be used to generate string literals with that
		 * description (literals have global lifetime, hence they will outlive
		 * any Exception.)
		 * @param result Error code associated with this exception. This
		 * should be one of System Error Codes to provide a meaningful
		 * message.
		 * \sa https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms679360(v=vs.85).aspx
		 *********************************************************************/
		explicit winapi_error(HRESULT result, std::source_location location = std::source_location::current(), std::stacktrace trace = std::stacktrace::current()) noexcept
			: exception{ std::move(location), std::move(trace) }, m_code{ static_cast<DWORD>(result) }
		{ }

		/******************************************************************//*!
		 * Retrieves the error code associated with this exception.
		 * @returns Code number of the error.
		 *********************************************************************/
		[[nodiscard]] int exit_code() const noexcept override { return static_cast<int>(error_code()); }

		/******************************************************************//*!
		 * Retrieves the error code associated with this exception.
		 * @returns Code number of the error.
		 *********************************************************************/
		[[nodiscard]] DWORD error_code() const noexcept { return m_code; }

		/******************************************************************//*!
		 * Retrieves the error message associated with this exception.
		 * @returns String describing the error and its location.
		 * @remark The error message is obtained based on the error code
		 * using FormatMessage function and it will not produce meaningful
		 * message if the error code associated with this exception is not
		 * on of system error codes.
		 * @sa https://msdn.microsoft.com/pl-pl/library/windows/desktop/ms679351(v=vs.85).aspx
		 *********************************************************************/
		[[nodiscard]] std::wstring message() const noexcept override;

	private:
		/// Error code associated with this exception
		DWORD m_code;
	};

	/**********************************************************************//*!
	 * @brief Exception class for storing error location and custom error
	 * message in a wide string format.
	 *
	 * exception class used to represent custom errors. It stores a pointer to
	 * a C string describing the location of the error and the error message.
	 * User must make sure the exception doesn't outlive the location
	 * description.
	 *************************************************************************/
	class custom_error : public exception
	{
	public:
		/******************************************************************//*!
		 * Creates new exception with the given location description and error
		 * message.
		 * @param location Wide C string describing the location of the error.
		 * User must make sure the Exception doesn't outlive the C string.
		 * __AT__ macro can be used to generate string literals with that
		 * description (literals have global lifetime, hence they will outlive
		 * any Exception.)
		 * @param message Custom error message associated with this exception.
		 *********************************************************************/
		explicit custom_error(const std::wstring &message,
			std::source_location location = std::source_location::current(),
			std::stacktrace trace = std::stacktrace::current()) noexcept;
		/******************************************************************//*!
		 * Creates new exception with the given location description and error
		 * message.
		 * @param location Wide C string describing the location of the error.
		 * User must make sure the Exception doesn't outlive the C string.
		 * __AT__ macro can be used to generate string literals with that
		 * description (literals have global lifetime, hence they will outlive
		 * any Exception.)
		 * @param message Custom error message associated with this exception.
		 *********************************************************************/
		explicit custom_error(std::wstring &&message,
			std::source_location location = std::source_location::current(),
			std::stacktrace trace = std::stacktrace::current()) noexcept
			: exception{ std::move(location), std::move(trace) },
			m_message{ move(message) }
		{ }
		/******************************************************************//*!
		 * Retrieves the error code associated with this exception type.
		 * @returns Code number of this exception type.
		 * @remark Currently the error code is set to -1.
		 *********************************************************************/
		[[nodiscard]] int exit_code() const noexcept override
		{
			return EXIT_FAILURE;
		}

		/******************************************************************//*!
		 * Retrieves the error message associated with this exception.
		 * @returns String describing the error and its location.
		 *********************************************************************/
		[[nodiscard]] std::wstring message() const noexcept override
		{
			return m_message;
		}

	private:
		/// Custom message describing the error.
		std::wstring m_message;
	};
}