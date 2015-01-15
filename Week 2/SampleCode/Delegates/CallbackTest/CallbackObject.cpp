// Class1.cpp
#include "pch.h"
#include "CallbackObject.h"
#include <math.h>

using namespace CallbackTest;
using namespace Platform;

CallbackObject::CallbackObject(SingleFloatCallback^ userCallback)
{
	// Initialize everything
	this->counter = 0;
	this->setCallback(userCallback);
}

void CallbackObject::setCallback(SingleFloatCallback^ userCallback)
{
	this->callback = userCallback;
}

void CallbackObject::process() {
	// Do some useless math to generate a float we'll emit to the callback
	this->counter += 3;
	float data = sqrtf(this->counter*this->counter + 10.0f);

	// Call the callback just as if it were a function
	this->callback(data);
}