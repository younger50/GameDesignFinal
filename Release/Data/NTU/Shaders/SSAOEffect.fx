// global constants
float4x4 mWVP;
float4x4 mWorld;
float4x4 mWV;
float    fFar;
float    fNear;
float2   fPixelSize;

float sampleRange = 18.0f;        // sampling range in pixels
float dist = 6.0f;                // distance range in 3D
float tanFov2 = 0.4663f;          // tan(fov/2)
float aspect = 1.6f;           // camera aspect ratio

texture fullResMap;
sampler fullResMapSampler = sampler_state
{
   Texture = <fullResMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = LINEAR;
   AddressU  = Clamp;
   AddressV  = Clamp;
   AddressW  = Clamp; 
};

// a depth map
texture depthMap;
sampler depthSampler = sampler_state
{
   Texture = <depthMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = LINEAR;
   AddressU  = Clamp;
   AddressV  = Clamp;
   AddressW  = Clamp;   
};

// a normal map
texture normalMap;
sampler normalSampler = sampler_state
{
   Texture = <normalMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU  = Clamp;
   AddressV  = Clamp;
   AddressW  = Clamp;   
};

float2 sampleRhombus[12] = { float2(-0.33, 0.0), // Left
                             float2( 0.33, 0.0), // Right
                             float2( 0.0, -0.33), // Top
                             float2( 0.0, 0.33), // Bottom
                             float2(-0.66, 0.0),
                             float2( 0.66, 0.0),
                             float2( 0.0, -0.66),
                             float2( 0.0, 0.66),
                             float2(-1.0, 0.0),
                             float2( 1.0, 0.0),
                             float2( 0.0, -1.0),
                             float2( 0.0, 1.0) };

float2 sampleSquare[12] = { float2(-0.233, 0.233), // Left-Bottom
                            float2( 0.233, 0.233), // Right-Bottom
                            float2(-0.233, -0.233), // Left-Top
                            float2( 0.233, -0.233), // Right-Top
                            float2(-0.466, 0.466),
                            float2( 0.466, 0.466),
                            float2(-0.466, -0.466),
                            float2( 0.466, -0.466),
                            float2(-0.707, 0.707),
                            float2( 0.707, 0.707),
                            float2(-0.707, -0.707),
                            float2( 0.707, -0.707) };

//== SSAO Clear Depth Map ===========================================
// vertex shader output for depthmap
struct VS_CLEARDEPTH_OUTPUT
{
    float4 pos : POSITION;
};


// vertex shader for clearing depthmap
VS_CLEARDEPTH_OUTPUT VS_SSAO_DepthClear(float4 inPos : POSITION)
{
    // initialize the output
    VS_CLEARDEPTH_OUTPUT out1 = (VS_CLEARDEPTH_OUTPUT) 0;
    
    // pass to pixel shader
    out1.pos.x = inPos.x - fPixelSize.x;
    out1.pos.y = inPos.y + fPixelSize.y;
    out1.pos.z  = 0.5f;
    out1.pos.w  = 1.0f;
    
    return out1;
}

// pixel shader for clearing depthmap
float4 PS_SSAO_DepthClear() : COLOR
{
   return -fFar;
}


//== SSAO Depth Map =================================================
// vertex shader output for depthmap
struct VS_DEPTH_OUTPUT
{
    float4 posP  : POSITION;
    float4 posV  : TEXCOORD0;   
};


// vertex shader for depthmap
VS_DEPTH_OUTPUT VS_SSAO_Depth(float4 inPos     : POSITION)
{
   // initialize the output
   VS_DEPTH_OUTPUT out1 = (VS_DEPTH_OUTPUT) 0;
   
   // transform the vertex to project/view space
   out1.posP = mul(inPos, mWVP);
   out1.posV = mul(inPos, mWV);
   
   return out1;
}


// pixel shader for depthmap
float4 PS_SSAO_Depth(float4 posV : TEXCOORD0) : COLOR
{
    return posV.z;
}


//== SSAO Normal Map ================================================
// vertex shader output for normalmap
struct VS_NORMAL_OUTPUT
{
   float4 pos   : POSITION;
   float3 norm  : TEXCOORD1;    
};


// vertex shader for depthmap
VS_NORMAL_OUTPUT VS_SSAO_Normal(float4 inPos     : POSITION,
                                float3 inNorm    : NORMAL)
{
    // initialize the output
    VS_NORMAL_OUTPUT out1 = (VS_NORMAL_OUTPUT) 0;
   
    // transform the vertex to project(screen)-space
    out1.pos = mul(inPos, mWVP);
   
    // transform the vertex normal vector to view-space
    out1.norm = mul(inNorm, mWV);
   
    return out1;
}


// pixel shader for depthmap
float4 PS_SSAO_Normal(float3 norm : TEXCOORD1) : COLOR
{
   // get the normal map
   return float4(normalize(norm)*0.5f + 0.5f, 1.0f);
}


//== Do SSAO ========================================================
// vertex shader output for DoSSAO
struct VS_DOSSAO_OUTPUT
{
    float4 pos : POSITION;
    float2 tex : TEXCOORD0;
};


// vertex shader for DoSSAO
VS_DOSSAO_OUTPUT VS_DoSSAO(float4 inPos : POSITION,
                           float2 inTex : TEXCOORD0 )
{
    // initialize the output
    VS_DOSSAO_OUTPUT out1 = (VS_DOSSAO_OUTPUT) 0;
    
    // pass to pixel shader
    //out1.pos.xy = inPos.xy;
    out1.pos.x  = inPos.x - fPixelSize.x;
    out1.pos.y  = inPos.y + fPixelSize.y;
    out1.pos.z  = 0.5f;
    out1.pos.w  = 1.0f;
    out1.tex    = inTex;
    
    return out1;
}


// pixel shader for DoSSAO
float4 PS_DoSSAO(float2 tex : TEXCOORD0) : COLOR
{
   float2 texS;
    
   float srcDepth = tex2D(depthSampler, tex).x;
   float3 srcPos;
   srcPos.z = srcDepth;
   srcPos.x = (1.0f - tex.x*2.0f)*tanFov2*srcPos.z;
   srcPos.y = (tex.y*2.0f - 1.0f)*tanFov2*srcPos.z/aspect;
   float3 norm = tex2D(normalSampler, tex).xyz*2.0f - 1.0f;
    
   float occlusion = 0.0f;
   float dstDepth;
   float distance;
   float3 dstPos, sNormal, vec;
   int j;
    
   for (j = 0; j < 12; j++) {
      texS = tex + sampleRhombus[j]*fPixelSize*sampleRange;
      dstDepth = tex2D(depthSampler, texS).x;
      dstPos.z = dstDepth;
      dstPos.x = (1.0f - texS.x*2.0f)*tanFov2*dstPos.z;
      dstPos.y = (texS.y*2.0f - 1.0f)*tanFov2*dstPos.z/aspect;
      vec = dstPos - srcPos;
      occlusion += dot(norm, normalize(vec))*saturate(1.0f - pow(length(vec)/dist, 2.0f));
        
      texS = tex + sampleSquare[j]*fPixelSize*sampleRange;
      dstDepth = tex2D(depthSampler, texS).x;
      dstPos.z = dstDepth;
      dstPos.x = (1.0f - texS.x*2.0f)*tanFov2*dstPos.z;
      dstPos.y = (texS.y*2.0f - 1.0f)*tanFov2*dstPos.z/aspect;
      vec = dstPos - srcPos;
      occlusion += dot(norm, normalize(vec))*saturate(1.0f - pow(length(vec)/dist, 2.0f));
   }
   occlusion = clamp(occlusion/24.0f, 0.0f, 1.0f);
    
   float4 color = tex2D(fullResMapSampler, tex);
   color *= (1.0f - occlusion);
   return color;
}


//== Technique ======================================================
// depth map technique
technique SSAO_DepthClear
{
   pass P0
   {         
      // compile shaders
      VertexShader = compile vs_3_0 VS_SSAO_DepthClear();
      PixelShader  = compile ps_3_0 PS_SSAO_DepthClear();
   }
}

technique SSAO_Depth
{
   pass P0
   {         
      // compile shaders
      VertexShader = compile vs_3_0 VS_SSAO_Depth();
      PixelShader  = compile ps_3_0 PS_SSAO_Depth();
   }
}

// normal map technique
technique SSAO_Normal
{
   pass P0
   {         
      // compile shaders
      VertexShader = compile vs_3_0 VS_SSAO_Normal();
      PixelShader  = compile ps_3_0 PS_SSAO_Normal();
   }
}

// do ssao technique
technique DoSSAO
{
   pass P0
   {
      // compile shaders
      VertexShader = compile vs_3_0 VS_DoSSAO();
      PixelShader  = compile ps_3_0 PS_DoSSAO();
   }
}