#include <Engine/Application.hpp>
#include <Engine/Graphics/Gizmos.hpp>
#include <Engine/ResourceManager.hpp>
#include <Engine/Components/Camera.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Graphics/Framebuffer.hpp>
#include <Engine/Services/SceneService.hpp>
#include <Engine/Services/GizmoService.hpp>
#include <Engine/Services/ExternalService.hpp>
#include <Engine/Graphics/Pipelines/Forward.hpp>
#include <Engine/Graphics/Pipelines/Deferred.hpp>

#include <imgui.h>
#include <glad/glad.h>
// #include <GLFW/../../src/internal.h> // To access GLFW global state
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#if _WIN32
#undef max
#undef min
#endif

using namespace std;
using namespace glm;
using namespace Engine;
using namespace Engine::Graphics;
using namespace Engine::Services;
using namespace Engine::Components;

Application* Application::s_Instance = nullptr;

#ifdef NDEBUG // Release
ENGINE_API const std::string Application::AssetDir = "./Assets/";
#else		  // Debug
ENGINE_API const std::string Application::AssetDir = "../../../Applications/Assets/";
#endif

Application::Application(ApplicationArgs args) :
	m_Args(args),
	m_Window(nullptr),
	m_Gizmos(nullptr),
	m_Renderer(nullptr),
	m_Input(new Input()),
	m_WindowedResolution(1),
	m_State(ApplicationState::Starting)
{
	s_Instance = this;

	// m_Scene.GetPhysics().SetTimestep(s_Args.FixedTimestep);

	m_Args.Samples = std::clamp(m_Args.Samples, 0u, 16u);
	m_Args.Resolution.x = std::max(m_Args.Resolution.x, 800);
	m_Args.Resolution.y = std::max(m_Args.Resolution.y, 600);

	if (m_Args.Title.empty())
		m_Args.Title = "Application";

	m_State = ApplicationState::Starting;
}

void Application::Run()
{
	Log::SetLogLevel(Log::LogLevel::All);
	Log::Info("Starting engine..");

	CreateAppWindow();

	// Create Resource Manager instance
	m_ResourceManager = new ResourceManager();

	// Create Renderer instance
	Renderer::s_Instance = (m_Renderer = new Renderer());
	m_Renderer->m_Resolution = m_Args.Resolution;

	// Create Gizmos instance
	m_Gizmos = new Gizmos();
	Gizmos::s_Instance = m_Gizmos;

	// Load services
	InitServices();

#ifndef NDEBUG
	// Enable gizmos by default in debug mode
	// EnableGizmos(true);
#endif

	// Setup Render Pipeline
	// Renderer::SetPipeline<Pipelines::ForwardRenderPipeline>();
	Renderer::SetPipeline<Pipelines::DeferredRenderPipeline>();

	Renderer::SetVSync(m_Args.VSync);
	Renderer::Resized(m_Args.Resolution);

	for (auto& pair : m_Services)
		pair.second->OnStart();

	// Frame counting
	m_Renderer->m_FPS = m_Renderer->m_DeltaTime = 0;

	int frameCount = 0; // Frames counted in the last second
	float frameCountTime = 0; // How long since the last second has passed
	float frameStartTime = (m_Renderer->m_Time = (float)glfwGetTime()); // Game time at start of frame

	ImGuiIO& ImGUI_IO = ImGui::GetIO();
	m_State = ApplicationState::Running;
	SceneService* sceneService = GetService<SceneService>();
	while (!glfwWindowShouldClose(m_Window) && m_State == ApplicationState::Running)
	{
		float deltaTime = (float)m_Renderer->m_DeltaTime;
		for (auto& pair : m_Services)
			pair.second->OnUpdate(deltaTime);

		Input::s_Instance->Update();

#pragma region Drawing
		// Setup ImGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glViewport(0, 0, m_Renderer->m_Resolution.x, m_Renderer->m_Resolution.y);
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glClearColor(0, 0, 0, 1); // Set clear colour to black
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear framebuffer colour & depth

		for (auto& pair : m_Services)
			pair.second->OnDraw();

		if (sceneService)
		{
			auto cameras = sceneService->CurrentScene()->Root().GetComponentsInChildren<Camera>();
			for(Camera* camera : cameras)
				m_Renderer->GetPipeline()->Draw(*camera);
		}

		ImGui::Render(); // Draw ImGUI result

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (ImGUI_IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(m_Window);
		}

		glfwSwapBuffers(m_Window);
#pragma endregion

		glfwPollEvents();

		// Calculate delta time
		float frameEndTime = (m_Renderer->m_Time = (float)glfwGetTime());
		m_Renderer->m_DeltaTime = frameEndTime - frameStartTime;
		frameStartTime = frameEndTime;

		// Calculate FPS
		frameCount++;
		frameCountTime += m_Renderer->m_DeltaTime;
		if (frameCountTime >= 1.0)
		{
			m_Renderer->m_FPS = float(frameCount);
			frameCount = 0;
			frameCountTime = 0;
		}
	}

	Log::Info("Engine shutting down..");

	// Close ImGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Close GLFW
	glfwDestroyWindow(m_Window);
	glfwTerminate();

	for (auto& pair : m_Services)
	{
		pair.second->OnShutdown();
		delete pair.second;
	}
	m_Services.clear();

	delete m_Input;
	delete m_ResourceManager;

	m_Input = nullptr;
	m_Window = nullptr;
}

void Application::Exit() { s_Instance->m_State = ApplicationState::Stopping; }

void Application::UpdateGlobals()
{
#ifndef BUILD_SHARED_LIB
	// Update singleton instances of classes,
	//  this is used when using dll's and memory needs to be shared

	s_Instance = this;
	Input::s_Instance = m_Input;
	Gizmos::s_Instance = m_Gizmos;
	Renderer::s_Instance = m_Renderer;
	ImGui::SetCurrentContext(m_ImGuiContext);
	ResourceManager::s_Instance = m_ResourceManager;

	// Re-load the OpenGL context.
	// Since this isn't across threads there *should* be no issues :)
	gladLoadGLLoader(m_GladProc);
#endif
}

void Application::InitServices()
{
	AddService<SceneService>();
	AddService<ExternalServices>();
}

void Application::CreateAppWindow()
{
	Log::Assert(glfwInit(), "Failed to initialize GLFW");
	// m_GLFWGlobal = &_glfw;

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
	gladLoadGLLoader(m_GladProc = (GLADloadproc)glfwGetProcAddress);

#if !defined(NDEBUG) && !defined(__APPLE__)
	// If Debug configuration, enable OpenGL debug output
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GLFW_DebugOutput, nullptr);
#endif

#pragma region ImGUI
	// Initialise ImGUI
	IMGUI_CHECKVERSION();
	m_ImGuiContext = ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::StyleColorsDark();
	StyleImGUI();
#pragma endregion

	// Enable seamless transitions at cubemap joins
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	Log::Debug("Engine initialised");
}

// Totally not sourced from https://github.com/inanevin/LinaEngine/blob/master/LinaEditor/src/Core/GUILayer.cpp :)
const vec2 DefaultFramePadding = { 8, 2 };
const vec2 DefaultWindowPadding = { 8, 8 };
void Application::StyleImGUI()
{
	ImGuiIO& io = ImGui::GetIO();

	// Setup configuration flags.
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = ImGui::GetStyle().Colors;
	style.FrameBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	// style.AntiAliasedFill = false;
	// style.WindowRounding = 0.0f;
	style.TabRounding = 3.0f;
	// style.ChildRounding = 0.0f;
	style.PopupRounding = 3.0f;
	// style.FrameRounding = 0.0f;
	// style.ScrollbarRounding = 5.0f;
	style.FramePadding = ImVec2(DefaultFramePadding.x, DefaultFramePadding.y);
	style.WindowPadding = ImVec2(DefaultWindowPadding.x, DefaultWindowPadding.y);
	style.CellPadding = ImVec2(9, 2);
	// style.ItemInnerSpacing = ImVec2(8, 4);
	// style.ItemInnerSpacing = ImVec2(5, 4);
	// style.GrabRounding = 6.0f;
	// style.GrabMinSize     = 6.0f;
	style.ChildBorderSize = 0.0f;
	// style.TabBorderSize = 0.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMenuButtonPosition = ImGuiDir_None;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	// colors[ImGuiCol_PopupBorder] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.44f, 0.26f, 0.26f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.47f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.47f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.74f, 0.74f, 0.74f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.49f, 0.62f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.37f, 0.53f, 1.00f);
	/*
	colors[ImGuiCol_ButtonLocked] = ImVec4(0.183f, 0.273f, 0.364f, 1.000f);
	colors[ImGuiCol_ButtonSecondary] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ButtonSecondaryHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ButtonSecondaryActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ButtonSecondaryLocked] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_Folder] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
	colors[ImGuiCol_FolderHovered] = ImVec4(0.35f, 0.49f, 0.62f, 1.00f);
	colors[ImGuiCol_FolderActive] = ImVec4(0.24f, 0.37f, 0.53f, 1.00f);
	colors[ImGuiCol_Toolbar] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_Icon] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TitleHeader] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	colors[ImGuiCol_TitleHeaderHover] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleHeaderPressed] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
	colors[ImGuiCol_TitleHeaderBorder] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleHeaderDisabled] = ImVec4(0.17f, 0.00f, 0.00f, 1.00f);
	*/
	colors[ImGuiCol_Header] = ImVec4(0.47f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.43f, 0.24f, 0.24f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.49f, 0.32f, 0.32f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.58f, 0.58f, 0.58f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.73f, 0.73f, 0.73f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.25f, 0.26f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.47f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.69f, 0.15f, 0.29f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.20f, 0.20f, 0.71f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.23f, 0.23f, 0.71f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.61f);

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
}

ApplicationState Application::GetState() { return s_Instance->m_State; }
void Application::SetTitle(std::string title) { glfwSetWindowTitle(s_Instance->m_Window, (s_Instance->m_Args.Title = title).c_str()); }
bool Application::GetFullscreen() { return s_Instance->m_Args.Fullscreen; }
void Application::ToggleFullscreen() { SetFullscreen(!s_Instance->m_Args.Fullscreen); }

void Application::EnableGizmos(bool enable)
{
	if (enable) AddService<GizmoService>();
	else		RemoveService<GizmoService>();
}

vector<Service*> Application::GetAllServices()
{
	vector<Service*> services;
	for (auto& pair : s_Instance->m_Services)
		services.emplace_back(pair.second);
	return services;
}

void Application::ShowMouse(bool show)
{
	glfwSetInputMode(s_Instance->m_Window, GLFW_CURSOR, show ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Application::_SetFullscreen(bool fullscreen)
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

	for (auto& pair : m_Services)
		pair.second->OnResized(m_Args.Resolution);

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

#pragma region GLFW Callbacks
void Application::GLFW_WindowCloseCallback(GLFWwindow* window)
{
	s_Instance->Exit();
}

void Application::GLFW_ErrorCallback(int error, const char* description)
{
	Log::Error("[GLFW] " + string(description));
}

void Application::GLFW_ScrollCallback(GLFWwindow* window, double xOffset, double yOffset) { Input::s_Instance->m_ScrollDelta += (float)yOffset; }
void Application::GLFW_CursorPositionCallback(GLFWwindow* window, double xPos, double yPos) { Input::s_Instance->m_MousePos = { xPos, yPos }; }

void Application::GLFW_FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	ivec2 resolution = { width, height };

	Renderer::Resized(resolution);

	for (auto& pair : s_Instance->m_Services)
		pair.second->OnResized(resolution);

	if (!s_Instance->m_Args.Fullscreen)
		s_Instance->m_WindowedResolution = resolution;
}

void Application::GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:   Input::s_Instance->m_KeyStates[key] = Input::KeyState::Pressed; break;
	case GLFW_RELEASE: Input::s_Instance->m_KeyStates[key] = Input::KeyState::Released; break;
	default: break;
	}
}

void Application::GLFW_MouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS:   Input::s_Instance->m_MouseStates[button] = Input::KeyState::Pressed; break;
	case GLFW_RELEASE: Input::s_Instance->m_MouseStates[button] = Input::KeyState::Released; break;
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
