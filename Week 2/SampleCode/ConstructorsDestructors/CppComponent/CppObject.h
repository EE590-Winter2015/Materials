#pragma once

namespace CppComponent
{
    public ref class CppObject sealed
    {
	private:
		// Declare our 
		int counter;
    public:
		// Declare multiple constructors, one with an implicit initialValue, and one with an explicitly provided one.
        CppObject();
		CppObject(int initialValue);

		// Declare a destructor (remember to always use "virtual")
		virtual ~CppObject();

		// Declare an increment() function which, as we are rapidly discovering, is my favorite method to write.
		int increment(int delta);
    };
}