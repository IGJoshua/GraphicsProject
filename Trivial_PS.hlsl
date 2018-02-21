#include "Functions.hlsli"

texture2D albedoMap : register( t0 );
texture2D albedoMap2 : register( t1 );
texture2D normalMap : register( t2 );
texture2D specularMap : register( t3 );
textureCUBE reflectionMap : register( t4 );

SamplerState MeshTextureSampler : register( s0 );
SamplerState ReflectionTextureSampler : register( s1 );

float4 main( float4 colorFromRasterizer : COLOR, float4 normalFromRasterizer : NORMAL, float4 worldPosition : POSITION, float2 uvFromRasterizer : TEXCOORD, float4 localPosition : LOCAL_POSITION, float3x3 tbn : TBN ) : SV_TARGET
{
	float4 returnColor = (float4)0;

	// Calculate the texture color for the pixel coord
	float4 sample1 = albedoMap.Sample(MeshTextureSampler, uvFromRasterizer);
	float4 sample2 = albedoMap2.Sample(MeshTextureSampler, uvFromRasterizer);
	float4 albedoColor = lerp(sample1, sample2, sample2.w);
	albedoColor = lerp(colorFromRasterizer, albedoColor, albedoColor.w);

	// Calculate light for the pixel coord
	float4 ambientLight = ambientLightColor * albedoColor;

	// Fix the normal to take normal map into account
	normalFromRasterizer = float4(mul((float3)normalMap.Sample(MeshTextureSampler, uvFromRasterizer), tbn), 0);

	// Point light
	float4 pointLightColor = CalculatePointLight(pointLights[0], worldPosition, normalFromRasterizer);

	// Spot light
	float4 spotLightColor = CalculateSpotLight(spotLights[0], worldPosition, normalFromRasterizer);

	float4 directionalLight = CalculateDirectionalLight(directionalLightNormal, directionalLightColor, normalFromRasterizer);

	float4 totalLight = saturate(directionalLight + ambientLight + pointLightColor + spotLightColor);

	float4 lambertColor = totalLight * albedoColor;

	float4 specularColor = CalculateSpecularLight(directionalLightNormal, directionalLightColor, worldPosition, normalFromRasterizer, cameraWorldPos);

	float4 spotDirection = float4((worldPosition - spotLights[0].lightPosition).xyz, 0.0);
	specularColor += CalculateSpecularLight(spotDirection, spotLights[0].lightColor, worldPosition, normalFromRasterizer, cameraWorldPos)
		* saturate(dot(normalize(spotDirection), normalize(spotLights[0].lightNormal)) - 0.2);

	float4 pointDirection = float4((worldPosition - pointLights[0].lightPosition).xyz, 0.0);
	specularColor += CalculateSpecularLight(pointDirection, float4(pointLights[0].lightColor, 1.0), worldPosition, normalFromRasterizer, cameraWorldPos);
	specularColor *= specularMap.Sample(MeshTextureSampler, uvFromRasterizer);

	float4 reflectionColor = float4(reflectionMap.Sample(ReflectionTextureSampler, reflect(worldPosition - cameraWorldPos, normalFromRasterizer).xyz).xyz, 1.0)
		* specularMap.Sample(MeshTextureSampler, uvFromRasterizer);

	returnColor = lambertColor + specularColor + reflectionColor;

	return saturate(returnColor);
}