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
float AlphaIntensity;

texture ColorMap, NormalMap;
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


struct VS_OUTPUT
{
   float4 pos  : POSITION;
   float2 tex0 : TEXCOORD0;
   float3 lgt  : TEXCOORD1;
   float3 cam  : TEXCOORD2; 
};


VS_OUTPUT Alpha_Color_NormalVS( float4 inPos   : POSITION, 
						float3 inNorm  : NORMAL, 
						float2 inTex   : TEXCOORD0,
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

   return out1;
}
   

void Alpha_Color_NormalPS( in  float2 vTex : TEXCOORD0,
                   in  float3 vLgt : TEXCOORD1,
                   in  float3 vCam : TEXCOORD2,
                   out float4 oCol : COLOR0    )
{
   float4 N;
   
   N.xyz= (2.0f*tex2D(NormalMapSampler, vTex) - 1.0f);
   
   N.w = 0.5f*N.z+0.5f;
   
   float3 L = normalize(vLgt);
   
   float3 V = normalize(vCam);
   
   float3 H = normalize(L + V);
     
   float diff = saturate(dot( N, L));
   
   float spec = pow(saturate(dot( N, H )), power);
     
   float4 ambLgt = lerp( groundColor , skyColor, N.w);
 
   float4 colorT = tex2D(ColorMapSampler, vTex);
   
   oCol.rgb = colorT.rgb*(ambLgt*amb + diff*mainLightColor*dif )+ spec*mainLightColor*spe;
  
   oCol.a = colorT.a*(AlphaIntensity+1);
}


technique Alpha_Color_Normal
{
   pass P0
   {
      AlphaTestEnable = TRUE;	
      AlphaFunc = GREATER;
      AlphaRef = 6;		
      AlphaBlendEnable = TRUE;	
      SrcBlend = SRCALPHA;
      DestBlend = INVSRCALPHA;
      VertexShader = compile vs_2_0 Alpha_Color_NormalVS();
      PixelShader = compile ps_2_0 Alpha_Color_NormalPS();      
   }
}