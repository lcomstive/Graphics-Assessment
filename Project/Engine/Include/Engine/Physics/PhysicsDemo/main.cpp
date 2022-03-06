#include "PhysicsDemo.hpp"

using namespace std::chrono_literals; // seconds in literal

Engine::ApplicationArgs applicationArgs =
{
	false,			// VSync
	4,				// Samples
	"Physics Demo",	// Title
	{ 1280, 720 },	// Resolution
	10ms			// Physics Timestep
};

#if defined(NDEBUG) && defined(_WIN32)
int __stdcall WinMain(void*, void*, char*, int)
#else
int main()
#endif
{
	PhysicsDemo demo(applicationArgs);
	demo.Run();
	return 0;
}
