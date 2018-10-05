
//it seems like :register(b_) is being ignored, the order of decleration determines how the compiler orders these at the moment, $Globals is always first
cbuffer PerDraw : register(b1)
{
    float4x4 localToWorld;
    float4x4 worldToLocal;
}

cbuffer PerFrame : register(b2)
{
    float4x4 viewMatrix;
    float4 worldCameraPos;
}

cbuffer PerApplication : register(b3)
{
    float4x4 projectionMatrix;
}

TextureCube skyMap;
SamplerState skyMapSampler;

float4 color1;
float scale;
float scale2;

struct VertexInputType
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 color : COLOR;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float3 vertexPos : TEXCOORD0;
};

PixelInputType VSMain(VertexInputType input)
{
    PixelInputType output;
    output.vertexPos = normalize(input.position.xyz);
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    input.position.xyz *= scale;
    output.position = mul(input.position, localToWorld);
    output.position = mul(output.position, viewMatrix); //view matrix is world to camera, to get camera position, we need camera to world, so inverse view
    output.position = mul(output.position, projectionMatrix); 

    output.normal = mul((float3x3) worldToLocal, input.normal.xyz);
    
    // Store the input color for the pixel shader to use.
    output.color = input.color;
    
    return output;
}

float4 PSMain(PixelInputType input) : SV_TARGET
{
    float4 c = 1;
    //c.rgb = float3(0.7, 0, 0.8);
    c.rgb = skyMap.Sample(skyMapSampler, input.vertexPos);

    return c;
}