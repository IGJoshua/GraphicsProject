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

float4 CalculatePointLight(POINT_LIGHT pl, float4 fragmentPosition, float4 surfaceNormal)
{
	float4 lightColor = (float4)0;

	float4 lightDirection = normalize(pl.lightPosition - fragmentPosition);
	float4 lightRatio = saturate(dot(lightDirection, surfaceNormal));
	float4 pointLightAttenuation = 1 - saturate(length(pl.lightPosition - fragmentPosition) / pl.lightRadius);
	pointLightAttenuation *= pointLightAttenuation;
	lightColor = lightRatio * float4(pl.lightColor, 1.0f) * pointLightAttenuation;

	return lightColor;
}

float4 CalculateSpotLight(SPOT_LIGHT sp, float4 fragmentPosition, float4 surfaceNormal)
{
	float4 lightColor = (float4)0;

	float4 spotToFrag = normalize(fragmentPosition - sp.lightPosition);
	float normalAttenuation = saturate(dot(surfaceNormal, -spotToFrag));
	float coneAttenuation = 1.0f - saturate((sp.innerRadius - dot(normalize(sp.lightNormal), spotToFrag)) / (sp.innerRadius - sp.outerRadius));
	coneAttenuation *= coneAttenuation;
	float lightAttenuation = normalAttenuation * coneAttenuation;
	lightColor = lightAttenuation * sp.lightColor;


	return lightColor;
}

float4 CalculateDirectionalLight(float4 lightNormal, float4 lightColor, float4 surfaceNormal)
{
	float4 returnColor = (float4)0;

	returnColor = saturate(dot(normalize(-lightNormal), surfaceNormal)) * lightColor;

	return returnColor;
}

float4 CalculateSpecularLight(float4 lightDirection, float4 lightColor, float4 fragmentPosition, float4 surfaceNormal, float4 cameraWorldPos)
{
	float4 specularColor = (float4)0;

	float specularPow = 256;
	float specularIntensity = 0.7;

	float4 specularViewDir = normalize(cameraWorldPos - fragmentPosition);
	float4 specularHalfVector = normalize(normalize(-lightDirection) + specularViewDir);
	float intensity = max(pow(saturate(dot(surfaceNormal, specularHalfVector)), specularPow), 0);
	specularColor = lightColor * specularIntensity * intensity;

	return specularColor;
}