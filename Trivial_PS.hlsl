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

cbuffer LIGHTS : register( b2 )
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

SamplerState MeshTextureSampler
{
	Filter = D3D11_FILTER_ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 main( float4 colorFromRasterizer : COLOR, float4 normalFromRasterizer : NORMAL, float2 uvFromRasterizer : TEXCOORD ) : SV_TARGET
{
	float4 returnColor;
	returnColor = colorFromRasterizer;

	// Calculate the texture color for the pixel coord
	//returnColor = returnColor + albedoMap.Sample(MeshTextureSampler, uvFromRasterizer);

	// Calculate light for the pixel coord

	return returnColor;
}