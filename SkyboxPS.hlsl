#include "Functions.hlsli"

textureCUBE skybox : register( t0 );

SamplerState skyboxSampler : register( s0 );

float4 main(float4 fragmentColor : COLOR, float4 surfaceNormal : NORMAL, float4 worldPosition : POSITION, float2 uvs : TEXCOORD, float4 localPosition : LOCAL_POSITION) : SV_TARGET
{
	return 	skybox.Sample(skyboxSampler, normalize(float4(localPosition.xyz, 0)).xyz);
}