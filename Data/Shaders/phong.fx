
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

float4x4 mWorldInv;

texture ColorMap,NormalMap;
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

// vertex shader output
struct VS_OUTPUT
{
   float4 pos  : POSITION;
   float2 tex0 : TEXCOORD0;
   float3 lgt  : TEXCOORD1;
   float3 cam  : TEXCOORD2; 
   float4 norm : TEXCOORD3; 
};

// the vertex shader
VS_OUTPUT PhongVS(float4 inPos   : POSITION, 
                  float3 inNorm  : NORMAL, 
                  float2 inTex   : TEXCOORD0,
                  float3 inTangU : TANGENT)
{
   float4 gpos;
   VS_OUTPUT out1 = (VS_OUTPUT) 0;

   // convert the vertex from local to global      
   gpos = mul(inPos, mWorld);
   
   // get the vertex in screen space
   out1.pos = mul(inPos, mWVP);
   
   // prepare the normal, lighting direction, camera vector for pixel shader
  
   out1.norm.xyz = normalize(mul((float3x3) mWorldInv, inNorm));  
   
   float3x3 wToTangent;
   wToTangent[0] = mul(inTangU, mWorld);
   wToTangent[1] = mul(cross(inTangU, inNorm), mWorld);
   wToTangent[2] = mul(inNorm, mWorld);
   
   // transform the rest data to tangent space
   out1.lgt = normalize(mul(wToTangent, mainLightPosition.xyz - gpos.xyz));
   out1.cam = normalize(mul(wToTangent, camPosition.xyz - gpos.xyz));
   
   // pass the texture coordinate
   out1.tex0 = inTex; 

   return out1;
}
   
// the pixel shader
void PhongPS(in float2 vTex : TEXCOORD0,
             in float3 vLgt  : TEXCOORD1,
             in float3 vCam  : TEXCOORD2,
             out float4 oCol : COLOR0)
{
   // be sure to normalize the vectors
   float4 normDir;
   
   normDir.xyz = (2.0f*tex2D(NormalMapSampler, vTex) - 1.0f);
 
   normDir.w=0.5f*normDir.z+0.5f; 
   float3 lgtDir = normalize(vLgt);
   float3 halfDir = normalize(lgtDir + normalize(vCam));

   // N dot L      
   float diff = saturate(dot(normDir, lgtDir));
   
   // (N dot H)^n
   float spec = pow(saturate(dot(normDir, halfDir)), power);
     
   // phong reflection model
  
  float4 ambLgt = lerp( groundColor , skyColor, normDir.w );
 
  float3 colorT = tex2D(ColorMapSampler, vTex);
  //oCol = ambLgt*amb + diff*mainLightColor*dif + spec*mainLightColor;
  oCol.rgb=colorT.rgb*(ambLgt*1.5f*amb + diff*mainLightColor*dif) + spec*mainLightColor;
  
  // the opacity
  oCol.a = dif.a;
}


// the associated D3D Effect technique
technique Phong
{
   pass P0
   {
      //AlphaBlendEnable = TRUE;
      VertexShader = compile vs_2_0 PhongVS();
      PixelShader = compile ps_2_0 PhongPS();      
   }
}