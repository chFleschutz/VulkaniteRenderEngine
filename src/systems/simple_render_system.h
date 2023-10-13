#pragma once

#include "renderer/device.h"
#include "renderer/frame_info.h"
#include "renderer/pipeline.h"

#include <memory>
#include <vector>

namespace vre
{
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VulkanDevice& mVreDevice;

		std::unique_ptr<Pipeline> mVrePipeline;
		VkPipelineLayout mPipelineLayout;
	};

} // namespace vre
