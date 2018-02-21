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
	float3x3 tbnMatrix : TBN;
	float4 projectedCoordinate : SV_POSITION;
};

cbuffer CAMERA : register( b0 )
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float totalTime;
	float3 padding;
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

	float3 n = fromVertexBuffer.normal;// (float3)mul(fromVertexBuffer.normal, worldMatrix);// float3x3(transpose(inverse(worldMatrix))));
	float3 c1 = cross(n, float3(0, 0, 1));
	float3 c2 = cross(n, float3(0, 1, 0));
	float3 t = normalize(length(c1) > length(c2) ? c1 : c2);
	float3 b = normalize(cross(n, t));

	sendToRasterizer.tbnMatrix = float3x3((float3)mul(float4(t, 0), worldMatrix),
		(float3)mul(float4(b, 0), worldMatrix),
		(float3)mul(float4(n, 0), worldMatrix));

	return sendToRasterizer;
}