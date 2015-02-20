Texture2D shaderTexture;
SamplerState SampleType;

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
	return shaderTexture.Sample(SampleType, input.tex);
}
