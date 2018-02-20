
cbuffer TIME : register( b2 )
{
	float totalTime;
	float3 padding;
}

float4 main(float4 color : COLOR, float4 nrm : NORMAL, float4 worldPosition : POSITION) : SV_TARGET
{
	color.x = worldPosition.y * sin(totalTime);
	color.y = (10 - worldPosition.y) * cos(totalTime);
	color.z = 0.25;
	return saturate(color);
}