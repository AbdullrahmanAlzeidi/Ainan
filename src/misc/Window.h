#pragma once

#define WINDOW_SIZE_FACTOR_ON_LAUNCH 500

namespace ALZ {

	const float GlobalScaleFactor = 1000.0f;

	class Window
	{
	public:
		static void Init();
		static void Present();
		static void HandleWindowEvents();
		static void Clear();
		static void Terminate();
		static void CenterWindow();
		static void SetWindowLaunchSize();
		static bool WindowSizeChangedSinceLastFrame() { return m_WindowSizeChanged; };

		static GLFWwindow& GetWindow();
		static glm::vec2 FramebufferSize;

		static bool m_WindowSizeChanged;
	private:
		static GLFWwindow* Ptr;
		friend void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	};
}