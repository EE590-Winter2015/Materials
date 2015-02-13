//#include "pch.h"
#include "filter.h"
#include <string.h>
#include <math.h>

using namespace libfilter;

Filter::Filter( const Array<float>^ impulseResponse ) {
	this->N = impulseResponse->Length;
	this->impulseResponse = new float[this->N];

	// Copy impulseResponse in, backwards
	float * impulseResponseData = impulseResponse->Data;
	for( unsigned int i=0; i<N; ++i ) {
		this->impulseResponse[i] = impulseResponseData[(N-1) - i];
	}

	this->prevData = new float[this->N];
	memset( this->prevData, 0, sizeof(float)*this->N );

	this->idx = 0;
}

Filter::~Filter() {
	delete[] this->impulseResponse;
	delete[] this->prevData;
}

Array<float>^ Filter::filter( const Array<float>^ data ) {
	Array<float>^ result = ref new Array<float>(data->Length);
	// Just call filter(float) for each element of this float[]
	for( unsigned int i=0; i<data->Length; ++i )
		result[i] = filter(data[i]);
	return result;
}

float Filter::filter( float data ) {
	// Put data into prevData
	this->prevData[idx] = data;

	// Do dot product between prevData and impulseResponse:
	float result = 0.0f;
	for( unsigned int i=0; i<N; ++i ) {
		result += this->prevData[(i+idx)%N]*this->impulseResponse[i];
	}

	idx--;
	if( idx == -1 )
		idx = N-1;
	return result;
}

Array<float>^ Filter::getImpulseResponse() {
	return ref new Array<float>(this->impulseResponse, this->N);
}

/*
void Filter::updateImpulseResponse( const Array<float>^ impulseResponse ) {
	// If the length of the new impulseResponse is different, we need to grow both prevData as well as impulseResponse
	if( this->N != impulseResponse->Length ) {
		// Let's migrate prevData over
		float * oldPrevData = prevData;
		prevData = new float[impulseResponse->Length];
		
		if( impulseResponse->Length > this->N ) {
			// Set the new samples at the beginning equal to zero
			int pad = impulseResponse->Length - this->N;
			memset( prevData, 0, pad*sizeof(float) );

			// Copy into the rest of prevData the data that we have previously kept track of
			for( unsigned int i=0; i<this->N; ++i ) {
				prevData[pad+i] = oldPrevData[(i+idx)%N];
			}
			this->idx = pad;
		} else {
			// Copy in as much data as we now have room for
			int pad = this->N - impulseResponse->Length;
			for( unsigned int i = 0; i<impulseResponse->Length; ++i ) {
				prevData[i] = oldPrevData[(pad+i+idx)%N];
			}
			this->idx = N-1;
		}

		// Free our old prevData
		delete[] oldPrevData;

		// Next, let's re-allocate impulseResponse
		delete[] this->impulseResponse;
		this->impulseResponse = new float[impulseResponse->Length];
	}

	// Now, update the impulse response with the new data
	float * impulseResponseData = impulseResponse->Data;
	this->N = impulseResponse->Length;
	// Copy impulseResponse in, backwards
	for( unsigned int i=0; i<N; ++i ) {
		this->impulseResponse[i] = impulseResponseData[(N-1) - i];
	}
}
*/