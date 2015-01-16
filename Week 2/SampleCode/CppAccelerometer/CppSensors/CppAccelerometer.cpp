// Class1.cpp
#include "pch.h"
#include "CppAccelerometer.h"

using namespace CppSensors;
using namespace Platform;

CppAcc::CppAcc()
{
	// These lines should look very familiar, as they're almost identical to the C# way of initializing an Accelerometer
	this->acc = Accelerometer::GetDefault();
	this->acc->ReportInterval = this->acc->MinimumReportInterval;
	this->acc->ReadingChanged += ref new TypedEventHandler<Accelerometer ^, AccelerometerReadingChangedEventArgs ^>(this, &CppAcc::accChanged);
}


void CppAcc::accChanged(Accelerometer ^sender, AccelerometerReadingChangedEventArgs ^args)
{
	// accChanged() is just redirecting the function call form the Accelerometer object to our C# class
	this->onReadingChanged(args->Reading->AccelerationX, args->Reading->AccelerationY, args->Reading->AccelerationZ);
}
