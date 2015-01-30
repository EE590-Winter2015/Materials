// Class1.cpp
#include "pch.h"
#include "SignalGenerator.h"
#include <math.h>

using namespace InteropCppComponent;
using namespace Platform;

SignalGenerator::SignalGenerator()
{
	float z = 10.0f;
	auto myLambda = [this](float x) {
		return z*x + 1.0f;
	};

	float returnValue = myLambda(2.0f);
}


Platform::Array<float>^ SignalGenerator::makeSinWave(int N, float freq, float startingPhase)
{
	Platform::Array<float>^ data = ref new Platform::Array<float>(N);
	float * internalData = data->Data;
	for (int i = 0; i < N; ++i) {
		internalData[i] = sinf(i * 2 * 3.1425926f *freq / N + startingPhase);
	}
	return data;
}