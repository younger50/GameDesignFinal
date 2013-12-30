float4x4 mWVP;            // matrix from local to screen space
float4x4 mWorld;          // matrix from local to global space
float4 mainLightPosition; // position of the "mainLight"
float4 mainLightColor;    // color of the "mainLight"
float4 skyColor; 
float4 groundColor;
float4 amb;               // ambient component of the material
float4 dif;               // diffuse component of the material
float4 spe;               // specular component of the material
float power;              // material shineness
float4 camPosition;       // camera position
float  bumpy = 10.0f;
float4x4 mWV;             // from world to view matrix
float4x4 mWorldInv;
float nearPlane = 5.0f;                 // near plane position
float farPlane = 100000.0f;              // far plane position
float2 pixelSize = { 1.0f/1024.0f, -1.0f/768.0f} ;                       // half pixel size

texture ColorMap, NormalMap, SpecularMap, DepthMap;

// Texture samplers
sampler2D ColorMapSampler = sampler_state
{
   Texture = <ColorMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;
};

sampler2D NormalMapSampler = sampler_state
{
   Texture = <NormalMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;
};

sampler2D SpecularMapSampler = sampler_state
{
   Texture = <SpecularMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;   
};


sampler2D DepthMapSampler = sampler_state
{
   Texture = <DepthMap>;
   MinFilter = None;
   MagFilter = None;
   MipFilter = None;
   AddressU = Wrap;
   AddressV = Wrap;   
};


texture ReflectionMap;
samplerCUBE cubeMapSampler = sampler_state
{
   Texture = <ReflectionMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   AddressU  = Clamp;
   AddressV  = Clamp;
   AddressW  = Clamp;   
};


struct VS_OUTPUT
{
   float4 vPosH : POSITION0;
   float2 vTexCoord : TEXCOORD0;
   float3 vTangentW : TEXCOORD1; // TBN output
   float3 vBinormalW : TEXCOORD2;
   float3 vNormalW : TEXCOORD3;
   float3 vLgtDir : TEXCOORD4;
   float3 vCamDir : TEXCOORD5;
   float3 vSkyDir : TEXCOORD6;
};

struct VS_OUTPUT_DEPTH
{
   float4 vPosH       : POSITION0;
   float2 vTexCoord   : TEXCOORD0;
   float3 vTangentW   : TEXCOORD1; // TBN output
   float3 vBinormalW  : TEXCOORD2;
   float3 vNormalW    : TEXCOORD3;
   float3 vLgtDir     : TEXCOORD4;
   float3 vCamDir     : TEXCOORD5;
   float3 vSkyDir     : TEXCOORD6;
   float4 vDepth      : TEXCOORD7;
};


VS_OUTPUT VS_Program(float4 vPosL : POSITION0,
                     float3 vNormalL : NORMAL0,
                     float2 vTexCoord : TEXCOORD0,
                     float3 vTangent : TANGENT)
{
    VS_OUTPUT vsOut = (VS_OUTPUT)0;
    
    vsOut.vPosH = mul(vPosL,mWVP);
    
    float3 vPosW = mul(vPosL,mWorld);
    
    // Transform TBN to World space
    float3x3 mTBN_W;
    mTBN_W[0] = mul(vTangent, (float3x3)mWorld);
    mTBN_W[1] = mul(cross(vNormalL,vTangent), (float3x3)mWorld);
    mTBN_W[2] = mul(vNormalL, (float3x3)mWorld);
    
    vsOut.vTangentW = mTBN_W[0];
    vsOut.vBinormalW = mTBN_W[1];
    vsOut.vNormalW = mTBN_W[2];
    
    // Transform light vector and camera direction to tangent space
    vsOut.vLgtDir = normalize(mul(mTBN_W,mainLightPosition.xyz - vPosW));
    vsOut.vCamDir = normalize(mul(mTBN_W,camPosition.xyz - vPosW));
    
    // Store the texture coordinate
    vsOut.vTexCoord = vTexCoord;
    
    vsOut.vSkyDir = float3(0.0f,0.0f,1.0f);
    
    //vsOut.vSkyDir = normalize(mul(mTBN_W,vsOut.vSkyDir));
    
    return vsOut;
    
}


float4 PS_Program(VS_OUTPUT inVS) : COLOR0
{
    // Get Normal which in tangent space from Normal Map
    float3 vNormalT = normalize((2.0f*tex2D(NormalMapSampler, inVS.vTexCoord) - 1.0f))*1.8f;
    float3 vNormalW = normalize(inVS.vNormalW);
     
    // Interpolated direction vector may not be normalized,so must normalize it manually (all in tangent space)
    float3 vL = normalize(inVS.vLgtDir);
    float3 vCamDir = normalize(inVS.vCamDir);
    float3 vHalfDir = normalize(vL+vCamDir);// calculates the half vector(Blinn) for specular component
    float3 vSkyDir = normalize(inVS.vSkyDir);
    
    float fDifIntensity = saturate(dot(vHalfDir,vNormalT)); // Diffuse component intensity    
    float fSpecIntensity = pow(max(dot(vNormalT,vHalfDir),0.0f),power); // Specular intensity
   
    float3 vAmbLgt = lerp(groundColor,skyColor,saturate(dot(vNormalW,vSkyDir))); // Ambent component (Sky color and G color)
    
    // Color map color
    float3 vColMap = tex2D(ColorMapSampler, inVS.vTexCoord);
    
    // Specular map
    float3 vSpecMap = tex2D(SpecularMapSampler, inVS.vTexCoord);
    
    float4 vFinalColor;
    
    vFinalColor.rgb = vColMap*(vAmbLgt + mainLightColor*fDifIntensity*dif + vSpecMap*fSpecIntensity*spe);
    
    vFinalColor.a = dif.a;
    
    
   return vFinalColor;
}


VS_OUTPUT_DEPTH VS_Program_Depth(float4 vPosL : POSITION0,
                                 float3 vNormalL : NORMAL0,
                                 float2 vTexCoord : TEXCOORD0,
                                 float3 vTangent : TANGENT)
{
    VS_OUTPUT_DEPTH vsOut = (VS_OUTPUT_DEPTH) 0;
    
    float4 pos = mul(vPosL, mWVP);
    float4 vPosW = mul(vPosL, mWorld);
    vsOut.vPosH = pos;
    
    // Transform TBN to World space
    float3x3 mTBN_W;
    mTBN_W[0] = mul(vTangent, (float3x3)mWorld);
    mTBN_W[1] = mul(cross(vNormalL,vTangent), (float3x3)mWorld);
    mTBN_W[2] = mul(vNormalL, (float3x3)mWorld);
    
    vsOut.vTangentW = mTBN_W[0];
    vsOut.vBinormalW = mTBN_W[1];
    vsOut.vNormalW = mTBN_W[2];
    
    // Transform light vector and camera direction to tangent space
    vsOut.vLgtDir = normalize(mul(mTBN_W,mainLightPosition.xyz - vPosW));
    vsOut.vCamDir = normalize(mul(mTBN_W,camPosition.xyz - vPosW));
    
    // Store the texture coordinate
    vsOut.vTexCoord = vTexCoord;
    
    vsOut.vSkyDir = float3(0.0f,0.0f,1.0f);

    // for depth checking
    float depth;
    float4 vPosInView;

    vPosInView = mul(vPosL, mWV);
    depth = (abs(vPosInView.z) - nearPlane)/(farPlane - nearPlane);

    vsOut.vDepth.x = depth;
    vsOut.vDepth.yz = float2(0.5f, -0.5f)*(pos.xy/pos.ww) + 0.5f.xx; // - pixelSize;
        
    return vsOut;
    
}


float4 PS_Program_Depth(VS_OUTPUT_DEPTH inVS) : COLOR0
{
    float ds = tex2D(DepthMapSampler, inVS.vDepth.yz).r;
    if (inVS.vDepth.x > ds) {
       discard;
    }

    // Get Normal which in tangent space from Normal Map
    float3 vNormalT = normalize((2.0f*tex2D(NormalMapSampler, inVS.vTexCoord) - 1.0f))*1.8f;
    float3 vNormalW = normalize(inVS.vNormalW);
     
    // Interpolated direction vector may not be normalized,so must normalize it manually (all in tangent space)
    float3 vL = normalize(inVS.vLgtDir);
    float3 vCamDir = normalize(inVS.vCamDir);
    float3 vHalfDir = normalize(vL+vCamDir);   // calculates the half vector(Blinn) for specular component
    float3 vSkyDir = normalize(inVS.vSkyDir);
    
    float fDifIntensity = saturate(dot(vHalfDir,vNormalT)); // Diffuse component intensity    
    float fSpecIntensity = pow(max(dot(vNormalT,vHalfDir),0.0f),power); // Specular intensity
   
    float3 vAmbLgt = lerp(groundColor,skyColor,saturate(dot(vNormalW,vSkyDir))); // Ambent component (Sky color and G color)
    
    // Color map color
    float3 vColMap = tex2D(ColorMapSampler, inVS.vTexCoord);
    
    // Specular map
    float3 vSpecMap = tex2D(SpecularMapSampler, inVS.vTexCoord);
    
    float4 vFinalColor;
    
    vFinalColor.rgb = vColMap*(vAmbLgt + mainLightColor*fDifIntensity*dif + vSpecMap*fSpecIntensity*spe);
    vFinalColor.a = dif.a;
    
    return vFinalColor;
}


struct VS_OUTPUT_AAA
{
   float4 vPosH : POSITION0;
   float2 vTexCoord : TEXCOORD0;
   float3 vTangentW : TEXCOORD1; // TBN output
   float3 vBinormalW : TEXCOORD2;
   float3 vNormalW : TEXCOORD3;
   float3 vLgtDir : TEXCOORD4;
   float3 vCamDir : TEXCOORD5;
   float3 vSkyDir : TEXCOORD6;
   float3 refl : TEXCOORD7;
};


VS_OUTPUT_AAA VS_AAA(float4 vPosL : POSITION0,
                     float3 vNormalL : NORMAL0,
                     float2 vTexCoord : TEXCOORD0,
                     float3 vTangent : TANGENT)
{
    VS_OUTPUT_AAA vsOut = (VS_OUTPUT_AAA)0;
    
    vsOut.vPosH = mul(vPosL,mWVP);
    
    float3 vPosW = mul(vPosL,mWorld);
    
    // Transform TBN to World space
    float3x3 mTBN_W;
    mTBN_W[0] = mul(vTangent, (float3x3)mWorld);
    mTBN_W[1] = mul(cross(vNormalL,vTangent), (float3x3)mWorld);
    mTBN_W[2] = mul(vNormalL, (float3x3)mWorld);
    
    vsOut.vTangentW = mTBN_W[0];
    vsOut.vBinormalW = mTBN_W[1];
    vsOut.vNormalW = mTBN_W[2];
    
    // Transform light vector and camera direction to tangent space
    vsOut.vLgtDir = normalize(mul(mTBN_W,mainLightPosition.xyz - vPosW));
    vsOut.vCamDir = normalize(mul(mTBN_W,camPosition.xyz - vPosW));
    
    // Store the texture coordinate
    vsOut.vTexCoord = vTexCoord;
    
    vsOut.vSkyDir = float3(0.0f,0.0f,1.0f);

   // find the incidence vector (from camera)
   float3 inc = normalize(vPosW.xyz - camPosition.xyz);
   float3 norm = mul(vNormalL, mWorld);
   
   // calculate the reflection vector for environment cubemap
   float3 refl = normalize(reflect(inc, norm));
   
   // convert it to y-up and left-hand system since TheFly use z-up and right hand system
   // from z-up to y-up : x = x, y = z, z = -y;
   // from right-hand to left-hand : z = -z;
   vsOut.refl.xyz = refl.xzy;
        
    return vsOut;
    
}


float4 PS_AAA(VS_OUTPUT_AAA inVS) : COLOR0
{
    // Get Normal which in tangent space from Normal Map
    float3 vNormalT = normalize((2.0f*tex2D(NormalMapSampler, inVS.vTexCoord) - 1.0f))*1.8f;
    float3 vNormalW = normalize(inVS.vNormalW);
     
    // Interpolated direction vector may not be normalized,so must normalize it manually (all in tangent space)
    float3 vL = normalize(inVS.vLgtDir);
    float3 vCamDir = normalize(inVS.vCamDir);
    float3 vHalfDir = normalize(vL+vCamDir);// calculates the half vector(Blinn) for specular component
    float3 vSkyDir = normalize(inVS.vSkyDir);
    
    float fDifIntensity = saturate(dot(vHalfDir,vNormalT)); // Diffuse component intensity    
    float fSpecIntensity = pow(max(dot(vNormalT,vHalfDir),0.0f),power); // Specular intensity
   
    float3 vAmbLgt = lerp(groundColor,skyColor,saturate(dot(vNormalW,vSkyDir))); // Ambent component (Sky color and G color)
    
    // Color map color
    float3 vColMap = tex2D(ColorMapSampler, inVS.vTexCoord);
    
    // Specular map
    float3 vSpecMap = tex2D(SpecularMapSampler, inVS.vTexCoord);
    
    float4 cubeColor = texCUBE(cubeMapSampler, normalize(inVS.refl));    
    float4 vFinalColor;
    
    vFinalColor.rgb = vColMap*(vAmbLgt + mainLightColor*fDifIntensity*dif + vSpecMap*fSpecIntensity*spe) + cubeColor.rgb;
    
    vFinalColor.a = dif.a;
    
    
   return vFinalColor;
}


technique Basic_Color_Normal_Power
{
    pass P0
    {
        VertexShader=compile vs_3_0 VS_Program();
        PixelShader=compile  ps_3_0 PS_Program();
    
    }
}

technique Basic_Color_Normal_Power_Depth
{
    pass P0
    {
        VertexShader=compile vs_3_0 VS_Program_Depth();
        PixelShader=compile  ps_3_0 PS_Program_Depth();
    
    }
}


technique AAA
{
    pass P0
    {
        VertexShader=compile vs_3_0 VS_AAA();
        PixelShader=compile  ps_3_0 PS_AAA();
    
    }
}






