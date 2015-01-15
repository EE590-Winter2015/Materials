#pragma once

namespace CallbackTest
{
	// Declare a delegate signature that returns nothing, but takes in a floating point number
	public delegate void SingleFloatCallback(float data);


    public ref class CallbackObject sealed
    {	
	private:
		// Declare a variable that is a delegate with the signature defined above
		SingleFloatCallback^ callback;

		// We'll also have our own counter
		int counter;
    public:
		// Initialize an object with a certain callback
		CallbackObject(SingleFloatCallback^ userCallback);

		// Overwrite the callback with a new one
		void setCallback(SingleFloatCallback^ userCallback);

		// Do some useless math, and emit the result through the callback
		void process();
    };
}