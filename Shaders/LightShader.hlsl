cbuffer CameraConstantBuffer : register(b0)
{
    row_major matrix view;
    row_major matrix projection;
    row_major matrix model;
    float3 cameraPosition;
};

struct RemLight
{
    float3 direction;
    float3 ambient;
    float3 diffuse;
    float3 specular;
};
cbuffer LightConstantBuffer : register(b1)
{
	RemLight remLight;
};

cbuffer LightCameraConstantBuffer : register(b2)
{   
    row_major matrix ViewProj[4];
    float4 Distances;
};

struct VS_IN
{
    float3 pos : POSITION0;
    float2 tex : TEXCOORD0;
    float4 normal : NORMAL0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 modelPos : POSITION;
    float4 lightViewPosition : POSITION1;
    float4 normal : NORMAL;
    float2 tex : TEXCOORD;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
 
    float4 modelPos = mul(float4(input.pos, 1.0f), model);
    output.pos = mul(mul(modelPos, view), projection);
    output.modelPos = modelPos;
    output.lightViewPosition = mul(modelPos, view);
    output.normal = mul(transpose(model), input.normal);
    output.tex = input.tex;
    
    return output;
}

Texture2D DiffuseMap                    : register(t0);
SamplerState Sampler                    : register(s0);

Texture2DArray ShadowMap                : register(t1);
SamplerComparisonState ShadowMapSampler : register(s1);

float3 CalcDirLight(RemLight remLight, float3 normal, float3 viewDir, float2 tex, float4 posViewProj, float layer);

float4 PSMain(PS_IN input) : SV_Target
{
    float3 norm    = normalize(input.normal);
    float3 viewDir = normalize(cameraPosition - input.modelPos.xyz);
    
    float layer = 3.0f;
    float depthVal = abs(input.lightViewPosition.z);
    for (int i = 0; i < 4; ++i)
    {
        if (depthVal < Distances[i])
        {
            layer = (float) i;
            break;
        }
    }
    
    float4 posViewProj = mul(input.modelPos, ViewProj[layer]);
    
    float3 result = CalcDirLight(remLight, norm, viewDir, input.tex, posViewProj, layer);

    return float4(result, 1.0f);
}

float IsLighted(float3 lightDir, float3 normal, float4 posViewProj, float layer);

float3 CalcDirLight(RemLight remLight, float3 normal, float3 viewDir, float2 tex, float4 posViewProj, float layer)
{
    float3 diffValue = DiffuseMap.Sample(Sampler, tex).rgb; 
    float3 lightDir = normalize( - remLight.direction);
    float  diff = max(dot(normal, lightDir), 0.0);
    float3 reflectDir = reflect( - lightDir, normal);
    float  spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    float3 ambient  = remLight.ambient         * diffValue;
    float3 diffuse  = remLight.diffuse  * diff * diffValue;
    float3 specular = remLight.specular * spec * diffValue;
    
    float1 isLighted = 1;   
    isLighted = IsLighted(lightDir, normal, posViewProj, layer);
    
    return (ambient + (diffuse + specular) * isLighted);
}


float IsLighted(float3 lightDir, float3 normal, float4 posViewProj, float layer)
{
    float ndotl = dot(normal, lightDir);
    float bias = clamp(0.005f * (1.0f - ndotl), 0.0f, 0.0005f);
    
    float3 projectTexCoord;

    projectTexCoord.x = posViewProj.x / posViewProj.w;
    projectTexCoord.y = posViewProj.y / posViewProj.w;
    projectTexCoord.z = posViewProj.z / posViewProj.w;

    projectTexCoord.x = projectTexCoord.x * 0.5 + 0.5f;
    projectTexCoord.y = projectTexCoord.y * -0.5 + 0.5f;
    
    float max_depth = ShadowMap.SampleCmpLevelZero(ShadowMapSampler, float3(projectTexCoord.x, projectTexCoord.y, layer), projectTexCoord.z);

    float currentDepth = (posViewProj.z / posViewProj.w);

    currentDepth = currentDepth - bias;
    
    if (max_depth < currentDepth)
    {
        return 0;
    }
    return max_depth;
}