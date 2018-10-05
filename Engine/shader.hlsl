
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
    float3 worldPos : TEXCOORD0;
};

PixelInputType VSMain(VertexInputType input)
{
    PixelInputType output;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    input.position.xyz *= scale;
    output.position = mul(input.position, localToWorld);
    output.worldPos = output.position.xyz;
    output.position = mul(output.position, viewMatrix); //this viewMatrix is designed for directX clip space, cannot extract camera position from it
    output.position = mul(output.position, projectionMatrix); 

    output.normal = mul((float3x3) worldToLocal, input.normal.xyz);
    
    // Store the input color for the pixel shader to use.
    output.color = input.color;
    
    return output;
}

float4 PSMain(PixelInputType input) : SV_TARGET
{
    input.normal = normalize(input.normal);

    float3 lightDir = normalize(float3(0, 0, 0.5));
    float3 lightCol = float3(1, 1, 1);
    float3 worldViewDir = normalize(worldCameraPos.xyz - input.worldPos);
    float3 halfDir = normalize(worldViewDir + lightDir);

    //float3 diffuse = float3(1, 0.84, 0) * 0.6; //gold
    float3 diffuse = float3(0.75, 0.75, 0.75) * 0.6; //silver
    //float3 diffuse = float3(0.89, 0.871, 0.858) * 0.6; //chrome
    float specular = 16.0f;

    float spec = pow(max(dot(input.normal, halfDir), 0.0), specular) * 0.7;

    float4 c = float4(diffuse, 1);
    c.rgb = max(dot(lightDir, input.normal), 0.1) * c.rgb;
    c.rgb += (spec * lightCol);

    half3 worldReflectionVector = normalize(2 * (dot(worldViewDir, input.normal) * input.normal) - worldViewDir);
 

    //c.rgb += skyMap.SampleLevel(skyMapSampler, input.normal, 6);
    float3 specularResult = skyMap.SampleLevel(skyMapSampler, worldReflectionVector, 4);
    //c.rgb += skyMap.SampleLevel(skyMapSampler, input.normal, 8);

    //float3 indirectDiffuse = skyMap.SampleLevel(skyMapSampler, input.normal, 6).rgb;

    //manual SH evaluation
    float3 indirectDiffuse = 0;
    float3 coeff0 = float3(0.570073, 0.428121, 0.364313);
    float3 coeff1 = float3(0.0946767, 0.0674416, 0.0816214);
    float3 coeff2 = float3(0.00730727, -0.00290015, -0.00670748);
    float3 coeff3 = float3(-0.124746, -0.0434827, 0.000920065);
    float3 coeff4 = float3(0.0301852, 0.0217055, 0.0227959);
    float3 coeff5 = float3(-0.0461149, -0.0406321, -0.0414945);
    float3 coeff6 = float3(0.0599109, 0.0608293, 0.0498371);
    float3 coeff7 = float3(0.00125457, 0.00269775, 0.00291185);
    float3 coeff8 = float3(0.119228, 0.085035, 0.0687854);

    indirectDiffuse += coeff0;
    indirectDiffuse += coeff1 * input.normal.x;
    indirectDiffuse += coeff2 * input.normal.z;
    indirectDiffuse += coeff3 * input.normal.y;
    indirectDiffuse += coeff4 * input.normal.x * input.normal.z;
    indirectDiffuse += coeff5 * input.normal.y * input.normal.z;
    indirectDiffuse += coeff6 * input.normal.x * input.normal.y;
    indirectDiffuse += coeff7 * (3 * input.normal.z * input.normal.z - 1);
    indirectDiffuse += coeff8 * (input.normal.x * input.normal.x - input.normal.y * input.normal.y);

    indirectDiffuse = pow(indirectDiffuse, 2.2);

    float3 diffuseResult = indirectDiffuse * diffuse * (1 / 3.14);

    //c.rgb = 0.7 * diffuseResult + 0.3 * specularResult;
    c.rgb = 0.96 * diffuseResult + 0.04 * specularResult;

    return c;
}