// LinearPredictiveCoding.cpp
#include "pch.h"
#include "LinearPredictiveCoding.h"
#include <stdlib.h>

using namespace LinearPredictiveCoding;
using namespace Platform;

LinearPredictiveCoder::LinearPredictiveCoder()
{
}

void getLinearCoefficients2(float *selectedOutput, int sizeOfSelectedOutput, int numberOfLinearCoeff, float * linearCoeff, float * lpGain)
{
    float *aut = (float*) malloc((numberOfLinearCoeff+1)*sizeof(float));
	//float *linearCoeff = (float *) malloc(numberOfLinearCoeff*sizeof(float));
	//lpGain = (float *) realloc(lpGain, runCounter*sizeof(float));

	float error;
	int i,j;
	
	//autocorrelation and p+1 lag coefficients
	j = numberOfLinearCoeff+1;
	while(j--!=0)
	{
		float d = 0.0f;
		for (i=j; i<sizeOfSelectedOutput; i++) 
		{
			d+=selectedOutput[i]*selectedOutput[i-j];
		}
			aut[j]=d;
	}
		
	//generate lpc coefficients from autocorrelation values
	error = aut[0];
	
	for (i=0;i<numberOfLinearCoeff;i++)
	{
		float r=-aut[i+1];
		if (error==0)
		{
			for (int k=0;k<numberOfLinearCoeff;k++)
				linearCoeff[k]=0.0f;
			return;
		}
		
		for (j=0;j<i;j++)
			r-=linearCoeff[j]*aut[i-j];
		r/=error;
		
		linearCoeff[i] = r;
		for (j=0;j<i/2;j++)
		{
			float tmp = linearCoeff[j];
			linearCoeff[j]+=r*linearCoeff[i-1-j];
			linearCoeff[i-1-j]+=r*tmp;
		}
		if (i%2!=0)
			linearCoeff[j]+=linearCoeff[j]*r;
		error*=(float)(1.0-r*r);
	}
	
//	printf("Error2: %f\n",error);
    
    //lpGain contains the lp gain from the latest frame of audio data 
	*lpGain = error;
    
    //printf ("%f\n",lpGain);
	free(aut);
	//free(linearCoeff);
}


Platform::Array<float>^ LinearPredictiveCoder::getLPCCoeffs( const Platform::Array<float>^ data, float * lpcGain, unsigned int numCoeffs ) {
	Platform::Array<float>^ lpcCoeffs = ref new Platform::Array<float>(numCoeffs);
	getLinearCoefficients2( data->Data, data->Length, numCoeffs, lpcCoeffs->Data, lpcGain );
	return lpcCoeffs;
}

