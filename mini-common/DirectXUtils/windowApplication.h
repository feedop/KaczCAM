#pragma once

#include <memory>
#include <optional>

#include "window.h"

namespace mini::windows
{
	/**********************************************************************//*!
	 * @brief Represents an instance of WINAPI application.
	 *
	 * Handles a lifetime of a GUI application, including creation of a window
	 * and implementation of the application main loop.
	 *
	 * It should be derived from to handle more specific application logic.
	 *************************************************************************/
	class window_app
	{
	public:
		static constexpr int s_default_window_width = 1280;
		static constexpr int s_default_window_height = 720;
		static constexpr std::wstring_view s_class_name{ L"Demo Window Class" };

		/******************************************************************//*!
		 * @brief Creates application instance.
		 *
		 * Creates new application instance along with its associated window
		 * with desired width, height and title.
		 * @param [in] instance Application instance handle (one that was
		 * passed to WinMain.
		 * @param [in] window_width Desired width of the associated window.
		 * @param [in] window_height Desired height of the associated window.
		 * @param [in] window_title Desired title of the associated window.
		 *********************************************************************/
		explicit window_app(HINSTANCE instance,
			int window_width = s_default_window_width,
			int window_height = s_default_window_height,
			std::wstring_view window_title = L"Default Window");

		/******************************************************************//*!
		 * Destroys the window and frees application resources.
		 *********************************************************************/
		virtual ~window_app() = default;

		/******************************************************************//*!
		 * Shows the window and runs program's main loop.
		 * @returns Application exit code.
		 *********************************************************************/
		[[nodiscard]] int run(int show_command = SW_SHOWNORMAL);

		/******************************************************************//*!
		 * Retrieves application's instance handle.
		 * @returns HINSTANCE handle of this program.
		 *********************************************************************/
		[[nodiscard]] constexpr HINSTANCE handle() const noexcept
		{
			return m_instance;
		}

	protected:
		/******************************************************************//*!
		 * @brief Handles system messages comming from the associated window.
		 *
		 * This is the implementation of IWindowMessageHandler. Uppon creation
		 * application instance passes itself as a details handler to its
		 * associated window. Whenever it recieves a details, it forwards it
		 * to this member function.
		 *
		 * This implementation ignores all messages. Derived classes can
		 * override this member function if needed.
		 * @param [in] msg contains message ID and its parameters.
		 * If the message is processed, method should return true and m.result
		 * should be set to the result of message processing.
		 * @returns std::nullopt if the message has not been processes or the
		 * result that should be returned from the window procedure in response
		 * to the message.
		 * @sa IWindowMessageHandler::ProcessMessage
		 *********************************************************************/
		virtual [[nodiscard]] std::optional<LRESULT> process_message([[maybe_unused]] message const &msg)
		{
			return std::nullopt;
		}

		/******************************************************************//*!
		 * @brief Main loop of the program.
		 *
		 * This member function is called by run to handle program's main loop.
		 * This is a simple blocking implementation, which waits for incomming
		 * messages in the program's queue and dispatches them. Derived
		 * classes can override this member function to implement different
		 * behavior.
		 * @returns Application exit code.
		 *********************************************************************/
		[[nodiscard]] virtual int main_loop();

		/******************************************************************//*!
		 * @returns Window associated with the application.
		 *********************************************************************/
		[[nodiscard]] window const &get_window() const { return  m_window; }


	private:
		/******************************************************************//*!
		 * @brief WindowProc callback function for application's main window.
		 *
		 * Callback function called by WINAPI to process messages incoming to
		 * the window. It handles forwarding of messages to specific
		 * application instances by calling non-static member function
		 * window_app::window_proc(UINT,WPARAM,LPARAM) on the application
		 * instance that is which main window is the recipient of a given
		 * details.
		 * @param [in] handle Handle of the window receiving the message.
		 * @param [in] msg The message.
		 * @param [in] w_param Additional message information.
		 * @param [in] l_param Additional message information.
		 * @returns Result of message processing.
		 * @remarks Since on some platforms Windows will silently eat all
		 * exceptions that leak through callback, the function catches them all
		 * displays an error message box and terminates the program.
		 *********************************************************************/
		static LRESULT CALLBACK window_proc(HWND handle, UINT msg, WPARAM w_param, LPARAM l_param) noexcept;


		/******************************************************************//*!
		 * @brief Processes messages incoming to this window.
		 *
		 * This member function is called whenever this window receives a
		 * details from the system. It forwards all messages (except
		 * WM_DESTROY, which is handled by posting WM_QUIT details)
		 * to the virtual process_message function.
		 *
		 * Derived classes can override that member function to implement
		 * additional program logic.
		 *********************************************************************/
		LRESULT window_proc(message const &m);

		[[nodiscard]] static std::wstring_view register_class(HINSTANCE instance);

		HINSTANCE m_instance;
		window m_window;
	};
}