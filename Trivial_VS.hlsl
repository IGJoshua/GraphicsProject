struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float2 padding : PADDING;
};

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float4 normal : NORMAL;
	float2 uv : TEXCOORD;
	float4 projectedCoordinate : SV_POSITION;
};

cbuffer CAMERA : register( b0 )
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

cbuffer OBJECT : register( b1 )
{
	float4x4 worldMatrix;
}

OUTPUT_VERTEX main( INPUT_VERTEX fromVertexBuffer )
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	
	sendToRasterizer.projectedCoordinate = mul(mul(mul(fromVertexBuffer.coordinate, worldMatrix), viewMatrix), projectionMatrix);
	sendToRasterizer.colorOut = fromVertexBuffer.color;
	sendToRasterizer.normal = mul(fromVertexBuffer.normal, worldMatrix);
	sendToRasterizer.uv = fromVertexBuffer.uv;

	return sendToRasterizer;
}