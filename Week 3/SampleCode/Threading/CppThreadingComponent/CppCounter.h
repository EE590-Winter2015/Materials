#pragma once

using namespace Windows::Foundation;

namespace CppThreadingComponent
{
	public delegate void TimerTickEvent(int count);

    public ref class CppCounter sealed
    {
	private:
		int count;
		IAsyncAction^ threadHandle;
    public:
		// Create and startup the thread
        CppCounter();

		// Cleanup everything, including stopping the thread
		virtual ~CppCounter();


		event TimerTickEvent^ onTimerTick;
    };
}