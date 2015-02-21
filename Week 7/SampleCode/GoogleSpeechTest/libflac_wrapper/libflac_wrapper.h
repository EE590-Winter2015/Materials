#pragma once
#include "FLAC++/metadata.h"
#include "FLAC++/encoder.h"
#include <vector>

namespace libflac_wrapper
{

	//public delegate void CompressedDataReadyEvent( const Platform::Array<unsigned char>^ data, unsigned int numSamples );

    public ref class libFLAC sealed
    {
		friend class StreamEncoder;
    public:
        libFLAC();

		Platform::Array<unsigned char>^ compressAudio( const Platform::Array<float>^ data,  unsigned int sampleRate, unsigned int channels );

	private protected:
		void appendCompressedData( const FLAC__byte * data, size_t numBytes, unsigned int samples );

	private:
		std::vector<unsigned char> compressedData;
    };

	class StreamEncoder : public FLAC::Encoder::Stream {
	public:
		StreamEncoder( libFLAC^ libflac ) : FLAC::Encoder::Stream() {
			this->libflac = libflac;
		}
	protected:
		virtual FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame) {
			libflac->appendCompressedData( buffer, bytes, samples );
			return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
		}
	private:
		libFLAC^ libflac;
	};
}