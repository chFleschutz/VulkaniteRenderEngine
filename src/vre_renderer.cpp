#include "vre_renderer.h"

#include <array>
#include <stdexcept>

namespace vre
{
	VreRenderer::VreRenderer(VreWindow& window, VreDevice& device) : mVreWindow{ window }, mVreDevice{ device }
	{
		recreateSwapChain();
		createCommandBuffers();
	}

	VreRenderer::~VreRenderer()
	{
		freeCommandBuffers();
	}

	void VreRenderer::renderFrame(const FrameInfo& frameInfo)
	{
		auto commandBuffer = beginFrame();
		if (!commandBuffer)
			return;

		beginSwapChainRenderPass(commandBuffer);

		for (const auto& [id, object] : frameInfo.gameObjects)
		{
			// only for each type (when it changes)
			
			//mVrePipeline->bind(frameInfo.commandBuffer);
			//vkCmdBindDescriptorSets();

			// for all objects
			// setup Push constants
			//vkCmdPushConstants();
			// bind and draw model
			//vkCmdDraw();

			if (!object.material)
				continue;

			object.material->bind(commandBuffer);
			object.model->bind(commandBuffer);

			// push constants

			object.model->draw(commandBuffer);
		}

		// for each object (sorted by material/pipeline)
		//	bind pipeline (if changed)
		//	bind descriptor sets (if changed)
		// 
		//	push constants
		//	
		//	bind vertex buffers (if changed)
		// 
		// draw

		endSwapChainRenderPass(commandBuffer);
		endFrame();
	}

	VkCommandBuffer VreRenderer::beginFrame()
	{
		assert(!mIsFrameStarted && "Cannot call beginFrame while already in progress");

		auto result = mVreSwapChain->acquireNextImage(&mCurrentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("failed to aquire swap chain image");

		mIsFrameStarted = true;

		auto commandBuffer = currentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer");

		return commandBuffer;
	}

	void VreRenderer::endFrame()
	{
		assert(mIsFrameStarted && "Cannot call endFrame while frame is not in progress");

		auto commandBuffer = currentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer");

		auto result = mVreSwapChain->submitCommandBuffers(&commandBuffer, &mCurrentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mVreWindow.wasWindowResized())
		{
			mVreWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image");
		}

		mIsFrameStarted = false;
		mCurrentFrameIndex = (mCurrentFrameIndex + 1) % VreSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void VreRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(mIsFrameStarted && "Cannot call beginSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == currentCommandBuffer() && "Cannot begin render pass on a command buffer from a diffrent frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mVreSwapChain->renderPass();
		renderPassInfo.framebuffer = mVreSwapChain->frameBuffer(mCurrentImageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = { mVreSwapChain->swapChainExtent() };

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mVreSwapChain->swapChainExtent().width);
		viewport.height = static_cast<float>(mVreSwapChain->swapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0,0}, mVreSwapChain->swapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VreRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(mIsFrameStarted && "Cannot call endSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == currentCommandBuffer() && "Cannot end render pass on a command buffer from a diffrent frame");

		vkCmdEndRenderPass(commandBuffer);
	}

	void VreRenderer::createCommandBuffers()
	{
		mCommandBuffers.resize(VreSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = mVreDevice.commandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

		if (vkAllocateCommandBuffers(mVreDevice.device(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffers");
	}

	void VreRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			mVreDevice.device(),
			mVreDevice.commandPool(),
			static_cast<uint32_t>(mCommandBuffers.size()),
			mCommandBuffers.data());

		mCommandBuffers.clear();
	}

	void VreRenderer::recreateSwapChain()
	{
		auto extend = mVreWindow.extend();
		while (extend.width == 0 || extend.height == 0) // minimized
		{
			extend = mVreWindow.extend();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(mVreDevice.device());

		if (mVreSwapChain == nullptr)
		{
			mVreSwapChain = std::make_unique<VreSwapChain>(mVreDevice, extend);
		}
		else
		{
			std::shared_ptr<VreSwapChain> oldSwapChain = std::move(mVreSwapChain);
			mVreSwapChain = std::make_unique<VreSwapChain>(mVreDevice, extend, oldSwapChain);
			if (!oldSwapChain->compareSwapFormats(*mVreSwapChain.get()))
				throw std::runtime_error("Swap chain image or depth format has changed");
		}

		// Todo
	}

} // namespace vre
