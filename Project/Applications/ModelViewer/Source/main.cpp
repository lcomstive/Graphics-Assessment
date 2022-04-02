#include "ModelViewer.hpp"
#include <Engine/Application.hpp>

using namespace Engine;
using namespace std::chrono_literals; // seconds in literal

// Default arguments
ApplicationArgs AppArgs =
{
	true,				// VSync
	1,					// Samples
	"Animation Test",	// Title
	{ 1280, 720 },		// Resolution
	20ms				// Physics Timestep
};

#if defined(NDEBUG) && defined(_WIN32)
int __stdcall WinMain(HINSTANCE inst, HINSTANCE prev, PSTR cmd, INT cmdShow)
#else
int main()
#endif
{
	Application app(AppArgs);
	app.AddService<MV::ModelViewer>();
	app.Run();
	return 0;
}
