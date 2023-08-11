#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace vre
{
	class Material
	{
	public:
		static Material createFromFile(const std::string& texturePath);
		 
		void bind(VkCommandBuffer commandBuffer, VkDescriptorSet* descriptorSet);

	private:
		VkPipeline m_Pipeline;
		VkPipelineLayout m_PipelineLayout;

		// refrence Pipeline
		// Texture(s) or descriptor bindings
	};

} // namespace vre