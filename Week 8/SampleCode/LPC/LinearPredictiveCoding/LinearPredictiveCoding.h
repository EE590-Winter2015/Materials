#pragma once

namespace LinearPredictiveCoding
{
    public ref class LinearPredictiveCoder sealed
    {
    public:
        LinearPredictiveCoder();

		Platform::Array<float>^ getLPCCoeffs( const Platform::Array<float>^ data, float * lpcGain, unsigned int numCoeffs );
    };
}