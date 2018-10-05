

cbuffer PerApplication : register(b3)
{
    float4x4 projectionMatrix;
	float4x4 invProjectionMatrix;
}


struct VertexInputType
{
	uint vertexID : SV_VertexID;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 clipPos : TEXCOORDS0;
};

PixelInputType VSMain(VertexInputType input)
{
    PixelInputType output; 
	
	if (input.vertexID > 2) 
	{ 
		output.position = float4(-1, -1, 0, 1);
	}
	else 
	{
		output.position.x = (float)(input.vertexID >> 1) * 4.0 - 1.0;
		output.position.y = (float)(input.vertexID & 1) * 4.0 - 1.0;
		output.position.z = 0.0;
		output.position.w = 1.0;
	}
    
    output.clipPos = output.position;
    return output;
}

//SV_POSITION passed into PS is in pixel coordinates with 0,0 starting top left
float4 PSMain(PixelInputType input) : SV_TARGET
{
    //float4 c = pow(float4(float2(input.position.x / 1280, input.position.y / 720), 0, 1), 2.2);

    //float4 c = pow(float4(input.clipPos.xy * 0.5 + 0.5, 0, 1), 2.2);


    input.clipPos.z = 0.7;
    float4 viewPos = mul(input.clipPos, invProjectionMatrix);
    viewPos.xyz /= viewPos.w;

    viewPos.z = 0;
    float4 c = pow(float4(viewPos.xyz, 1), 2.2);
    return c;
}