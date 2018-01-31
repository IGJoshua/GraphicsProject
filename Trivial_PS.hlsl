float4 main( float4 colorFromRasterizer : COLOR, float4 normalFromRasterizer : NORMAL, float2 uvFromRasterizer : UV ) : SV_TARGET
{
	return colorFromRasterizer;
}