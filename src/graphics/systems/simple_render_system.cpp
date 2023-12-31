#include "simple_render_system.h"

#include "graphics/renderer.h"
#include "scene/components.h"
#include "utils/math_utils.h"

#include <array>
#include <stdexcept>
#include <iostream>

namespace VEGraphics
{
	struct SimplePushConstantData
	{	// max 128 bytes
		Matrix4 modelMatrix{1.0f};
		Matrix4 normalMatrix{1.0f};
	};

	SimpleRenderSystem::SimpleRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : m_device{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(m_device.device(), mPipelineLayout, nullptr);
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo)
	{
		mPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mPipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		for (auto&& [entity, transform, mesh] : frameInfo.scene->viewEntitiesByType<VEComponent::Transform, VEComponent::Mesh>().each())
		{
			// TODO: transfer color as uniform
			Matrix4 colorNormalMatrix = MathLib::normalMatrix(transform.rotation, transform.scale);
			colorNormalMatrix[3] = mesh.color.rgba();

			SimplePushConstantData push{};
			push.modelMatrix = MathLib::tranformationMatrix(transform.location, transform.rotation, transform.scale);
			push.normalMatrix = colorNormalMatrix;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				mPipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			
			mesh.model->bind(frameInfo.commandBuffer);
			mesh.model->draw(frameInfo.commandBuffer);
		}
	}

	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout");
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = mPipelineLayout;

		std::string vertShaderPath = SHADER_DIR "simple_shader.vert.spv";
		std::string fragShaderPath = SHADER_DIR "simple_shader.frag.spv";

		mPipeline = std::make_unique<Pipeline>(
			m_device,
			vertShaderPath,
			fragShaderPath,
			pipelineConfig);
	}

} // namespace VEGraphics
