struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : UV;
};

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
	float4 normal : NORMAL;
	float2 uv : UV;
};

OUTPUT_VERTEX main( INPUT_VERTEX fromVertexBuffer )
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	
	sendToRasterizer.projectedCoordinate.xyzw = fromVertexBuffer.coordinate.xyzw;
	sendToRasterizer.colorOut.xyzw = fromVertexBuffer.color.xyzw;
	sendToRasterizer.normal.xyzw = fromVertexBuffer.normal.xyzw;
	sendToRasterizer.uv.xy = fromVertexBuffer.uv.xy;

	return sendToRasterizer;
}