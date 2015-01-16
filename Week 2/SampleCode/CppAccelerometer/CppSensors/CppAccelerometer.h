#pragma once

using namespace Windows::Devices::Sensors;
using namespace Windows::Foundation;

namespace CppSensors
{
	public delegate void AccelerometerEvent(double x, double y, double z);

    public ref class CppAcc sealed
    {
	private:
		Accelerometer ^acc;
    public:
		CppAcc();

		// Event that we trigger when we have a new reading
		event AccelerometerEvent^ onReadingChanged;

		// Internal function that gets called by the Accelerometer thread when it has a new reading for us
		void accChanged(Accelerometer ^sender, AccelerometerReadingChangedEventArgs ^args);
	};
}