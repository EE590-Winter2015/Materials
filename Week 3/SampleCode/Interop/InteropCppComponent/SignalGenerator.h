#pragma once


namespace InteropCppComponent
{
    public ref class SignalGenerator sealed
    {
    public:
        SignalGenerator();

		Platform::Array<float>^ makeSinWave(int N, float freq, float startingPhase);
    };
}