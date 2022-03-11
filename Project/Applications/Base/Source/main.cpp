#include <Engine/Application.hpp>
#include <Engine/Services/ExternalService.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::Services;
using namespace std::chrono_literals; // seconds in literal

// Default arguments
ApplicationArgs AppArgs =
{
	false,			// VSync
	1,				// Samples
	"Application",	// Title
	{ 1280, 720 },	// Resolution
	20ms			// Physics Timestep
};

class BaseApp : public Application
{
	string m_GameLogicPath;

public:
	BaseApp(ApplicationArgs& args, string gameLogicPath) : Application(args), m_GameLogicPath(gameLogicPath) { }

	void InitServices() override
	{
		Application::InitServices();
		GetService<ExternalServices>()->Add(m_GameLogicPath);
	}
};

const string DefaultGameLogicFile = "GameLogic";

#if defined(NDEBUG) && defined(_WIN32)
int __stdcall WinMain(HINSTANCE inst, HINSTANCE prev, PSTR cmd, INT cmdShow)
#else
int main()
#endif
{
	string gameLogicFile = Application::AssetDir + DefaultGameLogicFile;

	gameLogicFile = Application::AssetDir + "Services/Demo";
#if defined(_WIN32)
	gameLogicFile += ".dll";
#else
	gameLogicFile += ".so";
#endif

	BaseApp app(AppArgs, gameLogicFile);
	app.Run();
	return 0;
}
