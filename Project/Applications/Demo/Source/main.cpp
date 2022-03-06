#include "Demo.hpp"

using namespace std::chrono_literals; // seconds in literal

Engine::ApplicationArgs applicationArgs =
{
	false,			// VSync
	8,				// Samples
	"Demo",			// Title
	{ 1280, 720 },	// Resolution
	10ms			// Physics Timestep
};

#if defined(NDEBUG) && defined(_WIN32)
int __stdcall WinMain(void*, void*, char*, int)
#else
int main()
#endif
{
	Demo demo(applicationArgs);
	demo.Run();
	return 0;
}
