// Class1.cpp
#include "pch.h"
#include "CppCounter.h"
#include <windows.h>

using namespace CppThreadingComponent;
using namespace Platform;
using namespace Windows::System::Threading;

CppCounter::CppCounter()
{
	// First, initialize count to zero
	this->count = 0;

	// Next, create a thread that increments a counter and calls our onTimerTick function ten times a second

	// We do this by first creating a lambda that is what gets run inside the thread
	auto lambda = [this](IAsyncAction^ operation){
		// Run until the thread is canceled
		while (this->threadHandle->Status != AsyncStatus::Canceled) {
			// Sleep 1/10th of a second.  Don't worry about this function call, or ask me during office hours.
			WaitForSingleObjectEx(GetCurrentThread(), 100, FALSE);

			// Increment
			this->count++;

			// Invoke our event
			this->onTimerTick(this->count);
		}
	};

	// Now take that lambda, wrap it in a WorkItemHaandler object as ThreadPool::RunAsync expects, and watch it go!
	this->threadHandle = ThreadPool::RunAsync(ref new WorkItemHandler(lambda));
}

CppCounter::~CppCounter() {
	this->threadHandle->Cancel();
}