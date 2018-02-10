#include "Functions.hlsli"

cbuffer LIGHTS : register( b0 )
{
	POINT_LIGHT pointLights[16];
	SPOT_LIGHT spotLights[4];

	float4 directionalLightNormal;
	float4 directionalLightColor;

	float4 ambientLightColor;

	float4 cameraWorldPos;
}

texture2D albedoMap : register( t0 );
texture2D albedoMap2 : register( t1 );
texture2D normalMap : register( t2 );
texture2D specularMap : register( t3 );

SamplerState MeshTextureSampler : register( s0 );

float4 main( float4 colorFromRasterizer : COLOR, float4 normalFromRasterizer : NORMAL, float4 worldPosition : POSITION, float2 uvFromRasterizer : TEXCOORD ) : SV_TARGET
{
	float4 returnColor = (float4)0;

	// Calculate the texture color for the pixel coord
	float4 albedoColor = albedoMap.Sample(MeshTextureSampler, uvFromRasterizer) + albedoMap2.Sample(MeshTextureSampler, uvFromRasterizer);
	albedoColor = lerp(colorFromRasterizer, albedoColor, albedoColor.w);

	// Calculate light for the pixel coord
	float4 ambientLight = ambientLightColor * albedoColor;


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

	returnColor = lambertColor + specularColor;

	return saturate(returnColor);
}