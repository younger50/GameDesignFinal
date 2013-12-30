float4x4 mWVP;          
float4x4 mWorld;
float4x4 mWorldInv;         
float4 mainLightPosition;
float4 mainLightColor;   
float4 skyColor; 
float4 groundColor;
float4 camPosition;
float4 amb;              
float4 dif;               
float4 spe;              
float power;            
float LightMapIntensity;

texture ColorMap,NormalMap,SpecularMap,LightMap;
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
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = NONE;
   AddressU = Clamp;
   AddressV = Clamp;  
};

sampler2D LightMapSampler = sampler_state
{
   Texture = <LightMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = NONE;
   AddressU = Clamp;
   AddressV = Clamp;  
};


struct VS_OUTPUT
{
   float4 pos  : POSITION;
   float2 tex0 : TEXCOORD0;
   float2 tex1 : TEXCOORD1;
   float3 lgt  : TEXCOORD2;
   float3 cam  : TEXCOORD3; 
};


VS_OUTPUT Basic_Color_Normal_Specular_LgVS( float4 inPos   : POSITION, 
						float3 inNorm  : NORMAL, 
						float2 inTex   : TEXCOORD0,
						float2 inTex1  : TEXCOORD1,
						float3 inTangU : TANGENT   )
{  
   VS_OUTPUT out1 = (VS_OUTPUT) 0;
     
   float4 gpos = mul(inPos, mWorld);
   
   out1.pos = mul(inPos, mWVP);
   
   float3x3 wToTangent;
   
   wToTangent[0] = mul(inTangU, mWorld);
   
   wToTangent[1] = mul(cross(inTangU, inNorm), mWorld);
   
   wToTangent[2] = mul(inNorm, mWorld);
   
   out1.lgt = normalize(mul(wToTangent, mainLightPosition.xyz - gpos.xyz));
   
   out1.cam = normalize(mul(wToTangent, camPosition.xyz - gpos.xyz));
   
   out1.tex0 = inTex; 
   
   out1.tex1 = inTex1;

   return out1;
}
   

void Basic_Color_Normal_Specular_LgPS( in  float2 vTex : TEXCOORD0,
				   in  float2 vTex1: TEXCOORD1,
                   in  float3 vLgt : TEXCOORD2,
                   in  float3 vCam : TEXCOORD3,
                   out float4 oCol : COLOR0    )
{
   float4 N;
   
   N.xyz= (2.0f*tex2D(NormalMapSampler, vTex) - 1.0f);
   
   N.w = 0.5f*N.z+0.5f;
   
   float3 L = normalize(vLgt);
   
   float3 V = normalize(vCam);
   
   float3 H = normalize(L + V);
     
   float diff = saturate(dot( N, L));
   
   float3 lightT = saturate(tex2D(LightMapSampler, vTex1)+LightMapIntensity);
   
   float4 speT = tex2D(SpecularMapSampler, vTex);
   
   float spec = pow(saturate(dot(N, H)), power);
     
   float4 ambLgt = lerp( groundColor , skyColor, N.w);
 
   float3 colorT = tex2D(ColorMapSampler, vTex);
   
   oCol.rgb = colorT*1.4*(ambLgt*amb + diff*mainLightColor*dif)+ spec*mainLightColor*speT*spe;
  
   oCol.rgb = oCol.rgb*lightT;
   oCol.a = dif.a;
}


technique Basic_Color_Normal_Specular_Lg
{
   pass P0
   {
      VertexShader = compile vs_2_0 Basic_Color_Normal_Specular_LgVS();
      PixelShader = compile ps_2_0 Basic_Color_Normal_Specular_LgPS();      
   }
}