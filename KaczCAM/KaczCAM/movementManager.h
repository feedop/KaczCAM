#pragma once

#include <random>
#include <vector>
#include <DirectXMath.h>

#include <clock.h>

namespace mini
{
	namespace gk2
	{
		class MovementManager
		{
		public:
			virtual ~MovementManager() = default;
			virtual void Update(utils::clock const& clock) = 0;

			virtual const DirectX::XMMATRIX& GetModelMtx() const = 0;

			inline const DirectX::XMVECTOR& GetPosition() const
			{
				return m_position;
			}

			inline size_t GetModelId() const
			{
				return m_modelId;
			}

			inline void SlowDown()
			{
				m_velocity = std::clamp(m_velocity - VELOCITY_STEP, MIN_VELOCITY, MAX_VELOCITY);
			}

			bool SpeedUp();

		protected:
			inline static constexpr float MIN_VELOCITY = 0.04f;
			inline static constexpr float MAX_VELOCITY = 0.2f;
			inline static constexpr float VELOCITY_STEP = 0.02f;

			const size_t m_modelId;
			const DirectX::XMMATRIX m_initialTransformation;

			float m_velocity = 0.1f;

			DirectX::XMMATRIX m_transformation = DirectX::XMMatrixIdentity();
			DirectX::XMVECTOR m_position{ 0.0f, 0.0f, 0.0f };

			MovementManager(size_t modelId, const DirectX::XMMATRIX& initialTransformation);
		};

		class DuckMovement sealed : public MovementManager
		{
		public:
			DuckMovement(size_t modelId, const DirectX::XMMATRIX& initialTransformation);
			void Update(utils::clock const& clock) override;
			const DirectX::XMMATRIX& GetModelMtx() const override;

		private:
			float t = 0.5f;
			std::vector<DirectX::XMVECTOR> m_deBoorPoints;
			DirectX::XMVECTOR m_prevPosition{ -1.0f, 0.0f, 0.0f };

			std::random_device m_dev;
			std::mt19937 m_rng{ m_dev()};
			std::uniform_real_distribution<float> m_dist{ -19.0f, 19.0f };

			DirectX::XMVECTOR RandomPos();
			DirectX::XMVECTOR DeBoorAlgorithm() const;
		};
	}
}

