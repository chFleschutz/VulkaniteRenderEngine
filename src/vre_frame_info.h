#pragma once

#include "vre_camera.h"
#include "vre_game_object.h"

#include <vulkan/vulkan.h>

namespace vre
{
#define MAX_LIGHTS 10

	struct PointLight
	{
		glm::vec4 position{}; // ignore w
		glm::vec4 color{}; // w is intensity
	};

	struct GlobalUbo
	{
		glm::mat4 projection{1.0f};
		glm::mat4 view{1.0f};
		glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; // w is the intesity
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VreCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		VreGameObject::Map& gameObjects;
	};

} // namespace vre