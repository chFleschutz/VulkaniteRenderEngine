#include "Material.h"

namespace vre
{
	void Material::bind(VkCommandBuffer commandBuffer, VkDescriptorSet* descriptorSet)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, descriptorSet, 0, nullptr);
	}

} // namespace vre