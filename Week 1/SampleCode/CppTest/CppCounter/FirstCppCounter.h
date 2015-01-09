#pragma once

namespace CppCounter
{
    public ref class FirstCppCounter sealed
    {
	// Create a private section in this class declaration to store member variables
	private:
		int total;

	// Create a public section in this class declaration to store public API for this counter
    public:
		// Constructor, initializes object state (e.g. initialize total to zero)
		FirstCppCounter();

		// Increment internal counter by amount, then return total amount
		int incrementBy(int amount);
    };
}