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
	float4 worldPosition : POSITION;
	float2 uv : TEXCOORD;
	float4 localPosition : LOCAL_POSITION;
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
	
	sendToRasterizer.localPosition = fromVertexBuffer.coordinate;
	sendToRasterizer.worldPosition = mul(fromVertexBuffer.coordinate, worldMatrix);
	sendToRasterizer.projectedCoordinate = mul(mul(sendToRasterizer.worldPosition, viewMatrix), projectionMatrix);
	sendToRasterizer.colorOut = fromVertexBuffer.color;
	sendToRasterizer.normal = mul(fromVertexBuffer.normal, worldMatrix);
	sendToRasterizer.uv = fromVertexBuffer.uv;

	return sendToRasterizer;
}