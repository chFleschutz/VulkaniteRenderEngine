#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vre 
{
	class VreWindow
	{
	public:
		VreWindow(int width, int height, std::string title);
		~VreWindow();

		VreWindow(const VreWindow&) = delete;
		VreWindow operator=(const VreWindow&) = delete;

		bool shouldClose() const { return glfwWindowShouldClose(mWindow); }
		VkExtent2D extend() const { return { static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight) }; }
		bool wasWindowResized() const { return mFrameBufferResized; }
		void resetWindowResizedFlag() { mFrameBufferResized = false; }
		GLFWwindow* glfwWindow() const { return mWindow; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		void initWindow();

		int mWidth;
		int mHeight;
		bool mFrameBufferResized = false;

		std::string mWindowTitle;
		GLFWwindow* mWindow = nullptr;
	};

} // namespace vre