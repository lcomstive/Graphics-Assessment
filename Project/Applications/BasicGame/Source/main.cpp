#include "Game.hpp"
#include <Engine/Application.hpp>

using namespace Engine;
using namespace std::chrono_literals; // seconds in literal

// Default arguments
ApplicationArgs AppArgs =
{
	true,				// VSync
	1,					// Samples
	"Game",				// Title
	{ 1280, 720 },		// Resolution
	10ms				// Physics Timestep
};

#if defined(NDEBUG) && defined(_WIN32)
int __stdcall WinMain(HINSTANCE inst, HINSTANCE prev, PSTR cmd, INT cmdShow)
#else
int main()
#endif
{
	Application app(AppArgs);
	app.AddService<Game>();
	app.Run();
	return 0;
}
