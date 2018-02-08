struct POINT_LIGHT
{
	float4 lightPosition;
	float3 lightColor;
	float  lightRadius;
};

struct SPOT_LIGHT
{
	float4 lightPosition;
	float4 lightNormal;
	float4 lightColor;
	float outerRadius;
	float innerRadius;
	float2 padding;
};

cbuffer LIGHTS : register( b0 )
{
	POINT_LIGHT pointLights[16];
	SPOT_LIGHT spotLights[4];

	float4 directionalLightNormal;
	float4 directionalLightColor;

	float4 ambientLightColor;
}

texture2D albedoMap : register( t0 );
texture2D normalMap : register( t1 );
texture2D specularMap : register( t2 );

SamplerState MeshTextureSampler : register( s0 );

float4 main( float4 colorFromRasterizer : COLOR, float4 normalFromRasterizer : NORMAL, float4 worldPosition : POSITION, float2 uvFromRasterizer : TEXCOORD ) : SV_TARGET
{
	float4 returnColor = (float4)0;

	// Calculate the texture color for the pixel coord
	float4 albedoColor = albedoMap.Sample(MeshTextureSampler, uvFromRasterizer);
	albedoColor = lerp(colorFromRasterizer, albedoColor, albedoColor.w);

	// Calculate light for the pixel coord
	float4 ambientLight = ambientLightColor * albedoColor;

	float4 directionalLight = saturate(dot(normalize(-directionalLightNormal), normalFromRasterizer)) * directionalLightColor;

	// Point light
	float4 lightDirection = normalize(pointLights[0].lightPosition - worldPosition);
	float4 lightRatio = saturate(dot(lightDirection, normalFromRasterizer));
	float4 pointLightAttenuation = 1 - saturate(length(pointLights[0].lightPosition - worldPosition) / pointLights[0].lightRadius);
	pointLightAttenuation *= pointLightAttenuation;
	float4 pointLightColor = lightRatio * float4(pointLights[0].lightColor, 1.0f) * pointLightAttenuation;

	// Spot light
	float4 spotToFrag = normalize(worldPosition - spotLights[0].lightPosition);
	float normalAttenuation = saturate(dot(normalFromRasterizer, -spotToFrag));
	float coneAttenuation = 1.0f - saturate((spotLights[0].innerRadius - dot(normalize(spotLights[0].lightNormal), spotToFrag)) / (spotLights[0].innerRadius - spotLights[0].outerRadius));
	coneAttenuation *= coneAttenuation;
	float lightAttenuation = normalAttenuation * coneAttenuation;
	float4 spotLightColor = lightAttenuation * spotLights[0].lightColor;

	float4 totalLight = saturate(directionalLight + ambientLight + pointLightColor + spotLightColor);

	returnColor = totalLight * albedoColor;

	return saturate(returnColor);
}