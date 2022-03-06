#include <Engine/Application.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Pipelines/Forward.hpp>
#include <Engine/Graphics/Pipelines/Deferred.hpp>

#include <imgui.h>
#include <glad/glad.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Components;

Application* Application::s_Instance = nullptr;

#ifdef NDEBUG // Release
const std::string Application::AssetDir = "./Assets/";
#else		  // Debug
const std::string Application::AssetDir = "../../../Applications/Assets/";
#endif

Application::Application(ApplicationArgs args)
	: m_Args(args), m_Window(nullptr), m_Scene(this), m_WindowedResolution(1)
{
	s_Instance = this;
	m_Scene.GetPhysics().SetTimestep(m_Args.FixedTimestep);

	m_Args.Samples = std::clamp(m_Args.Samples, 0u, 16u);
	m_Args.Resolution.x = std::max(m_Args.Resolution.x, 800);
	m_Args.Resolution.y = std::max(m_Args.Resolution.y, 600);

	if (m_Args.Title.empty())
		m_Args.Title = "Application";
}

void Application::Run()
{
	Log::Info("Starting engine..");

	Renderer::s_App = this;
	Renderer::SetVSync(m_Args.VSync);
	Renderer::s_Samples = m_Args.Samples;
	Renderer::s_Resolution = m_Args.Resolution;

	CreateAppWindow();
	m_Scene.GetPhysics().Start();

	// Setup Render Pipeline
	// Renderer::SetPipeline<Pipelines::ForwardRenderPipeline>();
	Renderer::SetPipeline<Pipelines::DeferredRenderPipeline>();

	Renderer::Resized(m_Args.Resolution);
	
	SetupGizmos();
	
	OnStart();

	// Frame counting
	Renderer::s_FPS = Renderer::s_DeltaTime = 0;

	int frameCount = 0; // Frames counted in the last second
	float frameCountTime = 0; // How long since the last second has passed
	float frameStartTime = (Renderer::s_Time = (float)glfwGetTime()); // Game time at start of frame

	while (!glfwWindowShouldClose(m_Window))
	{
		m_Scene.Update((float)Renderer::s_DeltaTime);
		OnUpdate();
		Input::Update();

#pragma region Drawing
		// Setup ImGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glViewport(0, 0, Renderer::s_Resolution.x, Renderer::s_Resolution.y);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glClearColor(0, 0, 0, 1); // Set clear colour to black
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear framebuffer colour & depth

		m_Scene.Draw();
		OnDraw();

		if(Camera::GetMainCamera())
			Renderer::GetPipeline()->Draw(*Camera::GetMainCamera());

		ImGui::Render(); // Draw ImGUI result
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_Window);
#pragma endregion

		glfwPollEvents();

		// Calculate delta time
		float frameEndTime = (Renderer::s_Time = (float)glfwGetTime());
		Renderer::s_DeltaTime = frameEndTime - frameStartTime;
		frameStartTime = frameEndTime;

		// Calculate FPS
		frameCount++;
		frameCountTime += Renderer::s_DeltaTime;
		if (frameCountTime >= 1.0)
		{
			Renderer::s_FPS = float(frameCount);
			frameCount = 0;
			frameCountTime = 0;
		}
	}

	Log::Info("Engine shutting down..");

	m_Scene.GetPhysics().Stop();

	// Close ImGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Close GLFW
	glfwDestroyWindow(m_Window);
	glfwTerminate();

	OnShutdown();

	m_Window = nullptr;
}

void Application::Exit()
{
	if (m_Window)
		glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
}

void Application::CreateAppWindow()
{
	Log::Assert(glfwInit(), "Failed to initialize GLFW");

	glfwWindowHint(GLFW_SAMPLES, m_Args.Samples);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#ifndef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
#endif
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
	// When in Debug configuration, enable OpenGL's debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

	m_Window = glfwCreateWindow(
		m_Args.Resolution.x,
		m_Args.Resolution.y,
		m_Args.Title.c_str(),
		nullptr, nullptr // Monitor, share (?)
	);

	Log::Assert(m_Window, "Failed to create GLFW window");

	// Callbacks
	glfwSetKeyCallback(m_Window, GLFW_KeyCallback);
	glfwSetScrollCallback(m_Window, GLFW_ScrollCallback);
	glfwSetMouseButtonCallback(m_Window, GLFW_MouseCallback);
	glfwSetWindowCloseCallback(m_Window, GLFW_WindowCloseCallback);
	glfwSetCursorPosCallback(m_Window, GLFW_CursorPositionCallback);
	glfwSetFramebufferSizeCallback(m_Window, GLFW_FramebufferResizeCallback);

	// Finalise OpenGL creation
	glfwMakeContextCurrent(m_Window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	Input::s_MainWindow = m_Window;

#if !defined(NDEBUG) && !defined(__APPLE__)
	// If Debug configuration, enable OpenGL debug output
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GLFW_DebugOutput, nullptr);
#endif

#pragma region ImGUI
	// Initialise ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::StyleColorsDark();
#pragma endregion
	
	Log::Debug("Engine initialised");
}

Scene* Application::CurrentScene() { return &m_Scene; }
void Application::SetTitle(std::string title) { glfwSetWindowTitle(m_Window, (m_Args.Title = title).c_str()); }
bool Application::GetFullscreen() { return m_Args.Fullscreen; }
void Application::ToggleFullscreen() { SetFullscreen(!m_Args.Fullscreen); }
void Application::SetFullscreen(bool fullscreen)
{
	// BUG/FEATURE: Going back into windowed mode from fullscreen uses initial resolution,
	//				instead of pre-fullscreen resolutiona

	if (fullscreen == m_Args.Fullscreen)
		return; // No change

	m_Args.Fullscreen = fullscreen;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

	if (m_Args.Fullscreen)
		m_WindowedResolution = m_Args.Resolution;

	m_Args.Resolution = m_Args.Fullscreen ? ivec2 { videoMode->width, videoMode->height } : m_WindowedResolution;
	Renderer::Resized(m_Args.Resolution);
	s_Instance->OnResized(m_Args.Resolution);

	glfwSetWindowMonitor(
		m_Window,
		fullscreen ? monitor : nullptr,
		fullscreen ? 0 : int(videoMode->width  / 2.0f - m_Args.Resolution.x / 2.0f),
		fullscreen ? 0 : int(videoMode->height / 2.0f - m_Args.Resolution.y / 2.0f),
		m_Args.Resolution.x,
		m_Args.Resolution.y,
		videoMode->refreshRate
	);
	Renderer::SetVSync(m_Args.VSync);
}

void Application::SetupGizmos()
{
	RenderPipelinePass pass;
	FramebufferSpec spec;
	spec.Attachments = { TextureFormat::RGBA8, TextureFormat::Depth };
	spec.Resolution = Renderer::GetResolution();
	pass.Pass = new RenderPass(spec);
	pass.Shader = new Shader(ShaderStageInfo
		{
			AssetDir + "Shaders/Gizmos.vert",
			AssetDir + "Shaders/Gizmos.frag"
		});
	pass.DrawCallback = [=](RenderPass* previous)
	{
		// Copy color & depth buffer to this pass
		previous->GetFramebuffer()->BlitTo(pass.Pass->GetFramebuffer(), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		m_Scene.DrawGizmos();
		OnDrawGizmos();

		Renderer::Draw();
	};
	Renderer::GetPipeline()->AddPass(pass);
}

#pragma region GLFW Callbacks
void Application::GLFW_WindowCloseCallback(GLFWwindow* window)
{
	s_Instance->Exit();
}

void Application::GLFW_ErrorCallback(int error, const char* description)
{
	Log::Error("[GLFW] " + string(description));
}

void Application::GLFW_ScrollCallback(GLFWwindow* window, double xOffset, double yOffset) { Input::s_ScrollDelta += (float)yOffset; }
void Application::GLFW_CursorPositionCallback(GLFWwindow* window, double xPos, double yPos) { Input::s_MousePos = { xPos, yPos }; }

void Application::GLFW_FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	ivec2 resolution = { width, height };

	Renderer::Resized(resolution);
	s_Instance->OnResized(resolution);

	if (!s_Instance->m_Args.Fullscreen)
		s_Instance->m_WindowedResolution = resolution;
}

void Application::GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:   Input::s_KeyStates[key] = Input::KeyState::Pressed; break;
	case GLFW_RELEASE: Input::s_KeyStates[key] = Input::KeyState::Released; break;
	default: break;
	}
}

void Application::GLFW_MouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:   Input::s_MouseStates[button] = Input::KeyState::Pressed; break;
	case GLFW_RELEASE: Input::s_MouseStates[button] = Input::KeyState::Released; break;
	}
}

void Application::GLFW_DebugOutput(
	GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* msg,
	const void* userParam)
 {
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		Log::Error(msg);
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
	case GL_DEBUG_TYPE_PERFORMANCE:
		Log::Warning(msg);
		break;
	case GL_DEBUG_TYPE_OTHER:
		break;
	default:
		Log::Info(msg);
		break;
	}
}
#pragma endregion
