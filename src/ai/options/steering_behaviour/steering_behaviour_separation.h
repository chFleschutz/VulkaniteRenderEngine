#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"
#include "utils/math_utils.h"

namespace VEAI
{
	class SteeringBehaviourSeparation : public SteeringBehaviour
	{
	public:
		SteeringBehaviourSeparation(AIComponent* aiComponent, const EntityGroupKnowledge& group)
			: SteeringBehaviour(aiComponent), m_group(group) {}

		virtual VEPhysics::Force computeForce() override
		{
			if (m_group.entities.empty())
				return VEPhysics::Force{};

			auto& tranform = m_aiComponent->getComponent<VEComponent::Transform>();
			auto& dynamics = m_aiComponent->getComponent<VEPhysics::MotionDynamics>();

			VEPhysics::Force force{};
			for (const auto& entity : m_group.entities)
			{
				auto& otherTransform = entity.getComponent<VEComponent::Transform>();

				auto OtherDirection = otherTransform.location - tranform.location;
				auto distance = glm::length(OtherDirection);

				if (distance == 0.0f)
					continue;

				if (distance < m_activationRadius and MathLib::inFOV(dynamics.linearVelocity(), OtherDirection, m_fov))
				{
					auto strength = m_limits.maxLinearForce * (m_activationRadius - distance) / m_activationRadius;
					force.linear += MathLib::normalize(-OtherDirection) * strength;
				}
			}

			return force;
		}

	private:
		float m_activationRadius = 5.0f;
		float m_fov = glm::radians(360.0f);

		EntityGroupKnowledge m_group;
	};

} // namespace VEAI