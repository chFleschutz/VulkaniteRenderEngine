#pragma once

#include "vre_device.h"
#include "vre_descriptors.h"

#include <vulkan/vulkan.h>

#include <string>

namespace vre
{
	class VreTexture
	{
	public:
		struct CreateInfo
		{
			std::string textureFilePath;
			VkSampler& textureSampler;
			VreDescriptorSetLayout& descriptorSetLayout;
			VreDescriptorPool& descriptorPool;
		};

		VreTexture(VreDevice& device, const VreTexture::CreateInfo& createInfo);
		~VreTexture();

		VreTexture(const VreTexture&) = delete;
		VreTexture& operator=(const VreTexture&) = delete;

		VkDescriptorSet descriptorSet() { return mDescriptorSet; }

	private:
		void loadTexture(const std::string& filePath);
		void createImageView();
		void createTextureSampler(); // Todo remove
		void createDescriptorSet(const CreateInfo& createInfo);

		VreDevice& mVreDevice;

		VkImage mTextureImage;
		VkDeviceMemory mTextureImageMemory;
		VkImageView mTextureImageView;

		VkSampler& mTextureSampler;
		VkDescriptorSet mDescriptorSet;
	};

} // namespace vre