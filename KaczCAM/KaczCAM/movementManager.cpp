#include "movementManager.h"

using namespace std;
using namespace DirectX;
using namespace mini;
using namespace gk2;


MovementManager::MovementManager(size_t modelId, const XMMATRIX& initialTransformation) : m_modelId(modelId), m_initialTransformation(initialTransformation)
{}

bool MovementManager::SpeedUp()
{
	if (m_velocity == MAX_VELOCITY)
		return false;

	m_velocity = std::clamp(m_velocity + VELOCITY_STEP, MIN_VELOCITY, MAX_VELOCITY);
	return m_velocity == MAX_VELOCITY;
}

DuckMovement::DuckMovement(size_t modelId, const XMMATRIX& initialTransformation) : MovementManager(modelId, initialTransformation)
{
	m_deBoorPoints.push_back(m_position);
	for (int i = 0; i < 3; i++)
	{
		m_deBoorPoints.push_back(RandomPos());
	}
}

void DuckMovement::Update(utils::clock const& clock)
{
	static constexpr float eps = 1e-3f;
	float dt = static_cast<float>(clock.frame_time());

	if (t >= 5.0f/8.0f)
	{
		m_deBoorPoints.erase(m_deBoorPoints.begin());
		m_deBoorPoints.push_back(RandomPos());
		t = 0.5f;
	}

	m_prevPosition = m_position;
	m_position = DeBoorAlgorithm();
	t += dt * m_velocity;

	auto toNext = XMVector3Normalize(m_position - m_prevPosition);
	XMFLOAT3 toNextF;
	XMStoreFloat3(&toNextF, toNext);

	m_transformation = m_initialTransformation *
		XMMatrixRotationY(atan2(-toNextF.z, toNextF.x)) *
		XMMatrixTranslationFromVector(m_position);
}

const XMMATRIX& DuckMovement::GetModelMtx() const
{
	return m_transformation;
}

XMVECTOR DuckMovement::RandomPos()
{
	return { m_dist(m_rng), 0, m_dist(m_rng) };
}

XMVECTOR DuckMovement::DeBoorAlgorithm() const
{
	static constexpr float knotDist = 1.0f / 8.0f;
	static constexpr int i = 4;

	float N[5];
	float A[4];
	float B[4];

	N[1] = 1;
	for (int j = 1; j <= 3; j++)
	{
		A[j] = knotDist * (i + j) - t;
		B[j] = t - knotDist * (i + 1 - j);
		float saved = 0;
		for (int k = 1; k <= j; k++)
		{
			float term = N[k] / (A[k] + B[j + 1 - k]);
			N[k] = saved + A[k] * term;
			saved = B[j + 1 - k] * term;
		}
		N[j + 1] = saved;
	}

	return m_deBoorPoints[0] * N[1] +
		m_deBoorPoints[1] * N[2] +
		m_deBoorPoints[2] * N[3] +
		m_deBoorPoints[3] * N[4];
}
