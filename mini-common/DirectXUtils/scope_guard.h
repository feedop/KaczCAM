#pragma once
namespace mini::utils
{
	template<class F>
	class scope_guard
	{
		F m_f;
		bool m_active;
	public:
		constexpr explicit scope_guard(F f) noexcept
			: m_f{ std::move(f) }, m_active(true)
		{ }
		scope_guard() = delete;
		scope_guard(const scope_guard &) = delete;
		constexpr scope_guard(scope_guard &&other) noexcept
			: m_f(std::move(other.m_f)), m_active(other.m_active)
		{
			other.m_active = false;
		}
		scope_guard &operator=(const scope_guard &) = delete;
		constexpr scope_guard &operator=(scope_guard &&other) noexcept
		{
			m_f = std::move(other.m_f);
			m_active = other.m_active;
			other.m_active = false;
			return *this;
		}

		constexpr ~scope_guard() noexcept(noexcept(m_f())) { if (m_active) m_f(); }
	};

	template<class F>
	[[nodiscard]] constexpr scope_guard<F> make_guard(F f) noexcept
	{
		return scope_guard<F>{std::move(f)};
	}
}