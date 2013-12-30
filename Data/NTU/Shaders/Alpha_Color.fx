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

texture ColorMap;
sampler2D ColorMapSampler = sampler_state
{
   Texture = <ColorMap>;
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
   float3 norm : TEXCOORD3; 
};


VS_OUTPUT Alpha_ColorVS( float4 inPos : POSITION, 
						float3 inNorm: NORMAL, 
						float2 inTex : TEXCOORD0 )
{  
   VS_OUTPUT out1 = (VS_OUTPUT) 0;
     
   float4 gpos = mul(inPos, mWorld);
   
   out1.pos = mul(inPos, mWVP);
   
   out1.norm = normalize(mul((float3x3) mWorldInv, inNorm));
   
   out1.lgt = normalize(mainLightPosition.xyz - gpos.xyz);
   
   out1.cam = normalize(camPosition.xyz - gpos.xyz);
   
   out1.tex0 = inTex; 

   return out1;
}
   

void Alpha_ColorPS( in  float2 vTex : TEXCOORD0,
                   in  float3 vLgt : TEXCOORD1,
                   in  float3 vCam : TEXCOORD2,
                   in  float3 vNorm: TEXCOORD3,
                   out float4 oCol : COLOR0    )
{
   float4 N;
   
   N.xyz = normalize(vNorm);
   
   N.w = 0.5f*N.z+0.5f;
   
   float3 L = normalize(vLgt);
   
   float3 V = normalize(vCam);
   
   float3 H = normalize(L + V);
     
   float diff = saturate(dot( N, L));
   
   float spec = pow(saturate(dot( N, H )), power);
     
   float4 ambLgt = lerp( groundColor , skyColor, N.w);
 
   float4 colorT = tex2D(ColorMapSampler, vTex);
   
   oCol.rgb = colorT.rgb*(ambLgt*amb + diff*mainLightColor*dif) + spec*mainLightColor*spe;
  
   oCol.a = colorT.a*(AlphaIntensity/10+1);
}


technique Alpha_Color
{
   pass P0
   {
	  AlphaTestEnable = TRUE;	
      AlphaFunc = GREATER;
      AlphaRef = 6;		
      AlphaBlendEnable = TRUE;	
      SrcBlend = SRCALPHA;
      DestBlend = INVSRCALPHA;
      VertexShader = compile vs_2_0 Alpha_ColorVS();
      PixelShader = compile ps_2_0 Alpha_ColorPS();      
   }
}