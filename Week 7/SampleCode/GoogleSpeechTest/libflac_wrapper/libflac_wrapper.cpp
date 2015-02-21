// libflac_wrapper.cpp
#include "libflac_wrapper.h"

using namespace libflac_wrapper;
using namespace Platform;


libFLAC::libFLAC()
{
}

Array<unsigned char>^ libFLAC::compressAudio( const Array<float>^ data,  unsigned int sampleRate, unsigned int channels ) {
	// This is paranoia, should always be empty when we start
	compressedData.clear();

	// Create our encoder
	StreamEncoder encoder( this );
	bool ok = true;
	ok &= encoder.set_compression_level( 5 );
	ok &= encoder.set_channels( channels );
	ok &= encoder.set_bits_per_sample( 16 );
	ok &= encoder.set_sample_rate( sampleRate );
	ok &= encoder.set_total_samples_estimate( data->Length/channels );

	// If we couldn't create an encoder with those parameters, give up
	if( !ok ) {
		throw ref new InvalidArgumentException("Couldn't create encoder with the given parameters!");
	}

	FLAC__StreamEncoderInitStatus init_status = encoder.init();
	if( init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK ) {
		throw ref new FailureException( "Could not encode data!" );
	}
	
	// Convert 32-bit floating point data into 16-bit values packed into a 32-bit array
	FLAC__int32 * tempBuff = new FLAC__int32[data->Length];
	float * inputData = data->Data;
	for( unsigned int i=0; i<data->Length; ++i ) {
		tempBuff[i] = (short)(inputData[i]*SHRT_MAX);
	}

	// Run the properly formatted data through the encoder
	ok = encoder.process_interleaved( tempBuff, data->Length );
	ok = encoder.finish();

	// Clean stuff up even before checking for errors!
	delete( tempBuff );
	
	// Create Array to return
	Array<unsigned char>^ ret = ref new Array<unsigned char>(compressedData.size());
	memcpy(ret->Data, &compressedData[0], compressedData.size());

	// Clean up our memory
	compressedData.clear();

	// Retuuuuurn!
	return ret;
}

void libFLAC::appendCompressedData( const FLAC__byte * data, size_t numBytes, unsigned int samples ) {
	// This is bordering on black magic.  I love it.
	compressedData.insert( compressedData.end(), data, data + numBytes );
}