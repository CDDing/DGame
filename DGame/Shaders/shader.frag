#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32 : require
struct Material {
    int baseColorIndex;
    int normalMapIndex;
    int metallicRoughnessIndex;
    int emissiveIndex;

    vec4 baseColorFactor;

    float metallicFactor;
    float roughnessFactor;
    vec2 _padding1;

    vec3 emissiveFactor;
    float _padding2;
};

layout(set = 1, binding = 0) readonly buffer MaterialBuffer {
    Material materials[];
};
layout(set = 1, binding = 1) uniform sampler2D textures[];

layout(push_constant) uniform PushConstant {
    mat4 modelMatrix;
    uint64_t vertexAddress;
    int materialIndex;
    int pad;
} pushConst;


struct Light{
    vec3 color;
    float intensity;
    vec3 position;
    int type;
    vec3 direction;
    float innerCone, outerCone;
    float pad1,pad2,pad3;
};
layout(set = 0, binding = 0) uniform GlobalBuffer {
    mat4 view;
    mat4 projection;
    vec3 cameraPosition;
    float time;
    Light lights[10];
    uint numLights;
} ubo;

struct MatrixBuffer{
    mat4 view;
    mat4 projection;
    mat4 padding;
};
layout(set = 0, binding = 1) uniform sampler2D shadowMaps[4];
layout(set = 0, binding = 2) uniform samplerCube shadowCubeMaps[4];
layout(set = 0, binding = 3) uniform lightMatrixBuffer{
    MatrixBuffer directional[4];
    MatrixBuffer point[4][6];
    MatrixBuffer spot[4];
}lightMatrix;
layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outDepth;


const float PI = 3.14159265359;

vec3 getNormalFromMap()
{
    Material mat = materials[pushConst.materialIndex];

    int normalMapIndex = mat.normalMapIndex;

    vec3 tangentNormal = texture(textures[normalMapIndex], inUV).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(inWorldPos);
    vec3 Q2  = dFdy(inWorldPos);
    vec2 st1 = dFdx(inUV);
    vec2 st2 = dFdy(inUV);

    vec3 N   = normalize(inNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ----------------------------------------------------------------------------
void main() {
    Material mat = materials[pushConst.materialIndex];

    vec3 albedo = mat.baseColorFactor.xyz;

    if(mat.baseColorIndex >=0){
        albedo = texture((textures[mat.baseColorIndex]),inUV).xyz;
    }

    float metallic = 0.0f, roughness = 0.0f, ao = 1.0f;
    if(mat.metallicRoughnessIndex >= 0){
        ao = texture(textures[mat.metallicRoughnessIndex],inUV).r;
        roughness = texture(textures[mat.metallicRoughnessIndex],inUV).g;
        metallic = texture(textures[mat.metallicRoughnessIndex],inUV).b;
    }

    vec3 N = getNormalFromMap();
    vec3 V = normalize(ubo.cameraPosition - inWorldPos);


    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    int dCnt =0, pCnt =0, sCnt =0;
    for(int i = 0; i < ubo.numLights; ++i) 
    {
        vec3 L;
        float attenuation = 1.0;
        float shadow = 0.0;
        float NdotL;
        vec3 radiance;
        
        if (ubo.lights[i].type == 0) // Directional Light
        {
            L = normalize(-ubo.lights[i].direction);
            radiance = ubo.lights[i].color * ubo.lights[i].intensity;
            //shadow = directionalLightShadowCalculation(ubo.lights[i].position, dCnt);
            dCnt++;
        }
        else
        {
            vec3 lightDir = ubo.lights[i].position - inWorldPos;
            float distance = length(lightDir);
            L = normalize(lightDir);

            attenuation = 1.0 / (distance * distance);
            
            //shadow = pointLightShadowCalculation(ubo.lights[i].position, pCnt);
            pCnt++;
            if (ubo.lights[i].type == 2) // Spot Light
            {
                float cosTheta = dot(L, normalize(-ubo.lights[i].direction));
                float epsilon = ubo.lights[i].innerCone - ubo.lights[i].outerCone;
                float intensity = clamp((cosTheta - ubo.lights[i].outerCone) / epsilon, 0.0, 1.0);
                //attenuation *= intensity;
            }

            radiance = ubo.lights[i].color * ubo.lights[i].intensity * attenuation;


        }

        vec3 H = normalize(V + L);
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; 
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  

        NdotL = max(dot(N, L), 0.0);        


        Lo += (1.0 - shadow) * (kD * albedo / PI + specular) * radiance * NdotL; 
    }   
    
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    //color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    outColor= vec4(color, 1.0);

    //Draw Depth
    vec4 viewPos = ubo.view * vec4(inWorldPos, 1.0);
    float depthValue = viewPos.z;
    outDepth = vec4(depthValue, depthValue, depthValue, 1);


}
