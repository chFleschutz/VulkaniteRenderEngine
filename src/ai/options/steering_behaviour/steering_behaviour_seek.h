#pragma once

#include "ai/options/steering_behaviour/steering_behaviour.h"

#include <optional>

namespace VEAI
{
	class SteeringBehaviourSeek : public SteeringBehaviour
	{
	public:
		void setTarget(const EntityKnowledge& target) { m_target = target; }

	protected:
		virtual SteeringForce computeForce() override
		{
			SteeringForce force{};

			if (!m_target.has_value())
				return force;

			auto& transform = m_aiComponent->getComponent<VEComponent::Transform>();
			auto& playerTransform = m_target.value().entity.getComponent<VEComponent::Transform>();

			force.linear = glm::normalize(playerTransform.location - transform.location) * m_limits.maxLinearForce;
			return force;
		}

	private:
		std::optional<EntityKnowledge> m_target;
	};

} // namespace VEAI