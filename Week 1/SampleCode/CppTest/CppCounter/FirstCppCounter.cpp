// Class1.cpp
#include "pch.h"
#include "FirstCppCounter.h"

using namespace CppCounter;
using namespace Platform;

// Implement the constructor, to do initialization of object state
FirstCppCounter::FirstCppCounter()
{
	this->total = 0;
}

// Implement incremenyBy, to have the behavior we want it to.
int FirstCppCounter::incrementBy(int amount)
{
	this->total += amount;
	return this->total;
}
