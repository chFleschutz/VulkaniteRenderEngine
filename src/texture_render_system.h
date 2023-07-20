#pragma once

#include "vre_camera.h"
#include "vre_device.h"
#include "vre_frame_info.h"
#include "vre_pipeline.h"

#include <memory>
#include <vector>

namespace vre
{
	class TextureRenderSystem
	{
	public:
		TextureRenderSystem(VreDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
		~TextureRenderSystem();

		TextureRenderSystem(const TextureRenderSystem&) = delete;
		TextureRenderSystem& operator=(const TextureRenderSystem&) = delete;

		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout;
		void createPipeline(VkRenderPass renderPass);

		VreDevice& mVreDevice;

		std::unique_ptr<VrePipeline> mVrePipeline;
		VkPipelineLayout mPipelineLayout;
	};

} // namespace vre
