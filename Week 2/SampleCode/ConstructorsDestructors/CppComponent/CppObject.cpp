// Class1.cpp
#include "pch.h"
#include "CppObject.h"
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

using namespace CppComponent;
using namespace Platform;


// Don't worry about this black magic too much.  Suffice to say it allows us to print out to the console when we really need to.
void dprintf(const wchar_t *format, ...)
{
	wchar_t buffer[1024];
	va_list args;
	va_start(args, format);
	_vsnwprintf_s(buffer, 1024, _TRUNCATE, format, args);
	va_end(args);
	buffer[1024 - 1] = '\0'; //prevent buffer overflow
	OutputDebugString(buffer);
}

CppObject::CppObject()
{
	dprintf(L"[C++] Creating CppObject\n");
	this->counter = 0;
}

CppObject::CppObject(int initialValue) {
	dprintf(L"[C++] Creating CppObject with initial value %d\n", this->counter);
	this->counter = initialValue;
}

CppObject::~CppObject() {
	dprintf(L"[C++] Destroying CppObject with counter set to %d\n", this->counter);
}

int CppObject::increment(int delta) {
	this->counter += delta;
	return this->counter;
}
