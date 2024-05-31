#pragma once
#include <chrono>
#include <array>

namespace mini::utils
{
	//Helper class to measure time between animation frames using Windows high resolution timer
	template<size_t Log2Samples = 6>
	class fps_clock
	{
	public:
		using clock = std::chrono::steady_clock;
		using time_point = clock::time_point;
		using duration = clock::duration;
		using float_seconds = std::chrono::duration<float, std::chrono::seconds::period>;

		static constexpr size_t s_samples_count = 1 << Log2Samples;
		static constexpr size_t s_index_mask = s_samples_count - 1;

		fps_clock() noexcept
			: m_last{ clock::now() }, m_samples{}, m_total{}, m_current{}
		{ }

		//Update the clock. Returns time from last query or clock creation in seconds.
		float query() noexcept
		{
			auto now = std::chrono::steady_clock::now();
			m_current = m_current + 1 & s_index_mask;
			m_total -= m_samples[m_current];
			m_samples[m_current] = now - m_last;
			m_total += m_samples[m_current];
			m_last = now;
			return frame_time();
		}

		//Returns the time between last and second to last query in seconds;
		[[nodiscard]] constexpr float frame_time() const noexcept { return float_seconds{ frame_duration() }.count(); }

		//Returns the number of internal clock ticks between the last and second to last query.
		[[nodiscard]] constexpr duration frame_duration() const noexcept { return m_samples[m_current]; }

		//Returns the average FPS over the last s_samples_count frames
		[[nodiscard]] constexpr float fps() const noexcept { return s_samples_count / float_seconds{ m_total }.count(); }

	private:
		time_point m_last;
		std::array<duration, s_samples_count> m_samples;
		duration m_total;
		size_t m_current;
	};

	using clock = fps_clock<>;
}
