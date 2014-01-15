// This shader implements Toon shading with warp texture
// C.Wang 0123, 2013
//

float4x4 mWVP;
float width;
float4x4 mWorld;
float4x4 mWorldInv;
float4 mainLightPosition;
float3 lineColor;
float4 dif;
float4 mainLightColor;
float blur;
bool beColorMap;


texture colorMap;
sampler colorMapSampler = sampler_state
{
   Texture = <colorMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = LINEAR;
   AddressU  = Clamp;
   AddressV  = Clamp;
};


texture warpMap;
sampler warpMapSampler = sampler_state
{
   Texture = <warpMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = LINEAR;
   AddressU  = Clamp;
   AddressV  = Clamp;
};


// vertex shader output
struct VS_OUTPUT
{
   float4 pos : POSITION;
   float2 tex0 : TEXCOORD0;
   float3 norm : TEXCOORD1;
   float3 lgt : TEXCOORD2;
};


// the vertex shader of "GenSilhouette"
VS_OUTPUT VSGenSilhouette(float4 inPos : POSITION,
                          float3 inNorm : NORMAL,
                          float2 inTex : TEXCOORD0)
{
   VS_OUTPUT out1 = (VS_OUTPUT) 0;

   float4 a;

   a.xyz = inPos.xyz + inNorm*width;
   a.w = 1.0;
   
   // transform the vertex
   out1.pos = mul(a, mWVP);
   out1.tex0 = inTex;

   return out1;
}


// the pixel shader of "GenSilhouette"
float4 PSGenSilhouette(float2 tex0 : TEXCOORD0,
                       float3 norm : TEXCOORD1,
                       float3 lgt  : TEXCOORD2) : COLOR0
{
   float4 oCol;

   // output silhouette color
   oCol.rgb = lineColor.rgb;
   
   // the opacity
   oCol.a = dif.a;

   return oCol;
}


// the vertex shader of "Cartoon"
VS_OUTPUT VSCartoon(float4 inPos : POSITION,
                    float3 inNorm : NORMAL,
                    float2 inTex : TEXCOORD0)
{
   float4 a; 
   VS_OUTPUT out1 = (VS_OUTPUT) 0;

   // convert the vertex from local to world space      
   a = mul(inPos, mWorld);
   
   // get the vertex in screen space
   out1.pos = mul(inPos, mWVP);
   
   // prepare the normal, lighting direction, camera vector for pixel shader
   out1.norm = normalize(mul((float3x3) mWorldInv, inNorm));
   out1.lgt = normalize(mainLightPosition.xyz - a.xyz);
   out1.tex0 = inTex;

   return out1;
}


// the pixel shader of "Cartoon"
float4 PSCartoon(float2 vTex  : TEXCOORD0,
                 float3 vNorm : TEXCOORD1,
                 float3 vLgt  : TEXCOORD2) : COLOR0
{
   float4 oCol;

   // be sure to normalize the vectors
   float3 normDir = normalize(vNorm);
   float3 lgtDir = normalize(vLgt);

   // N dot L      
   float diff = 0.5f + 0.5f*dot(normDir, lgtDir);
   float3 diff3 = float3(0.0f, 0.0f, 0.0f);
   float2 uv = float2(0.5f, 0.5f);
   float v = saturate(blur);

   uv = float2(diff, v*0.95);
   diff3 = tex2D(warpMapSampler, uv).rgb;

   // reflection model
   oCol.rgb = mainLightColor.rgb*diff3*dif.rgb;

   // the opacity
   oCol.a = dif.a;

   if (beColorMap > 0) {
      float4 colorT = tex2D(colorMapSampler, vTex);
      oCol *= colorT;
   }   
   
   return oCol; 
}


// Cartoon Shader
technique Toon
{
   pass P0
   {
      cullMode = CW;
      VertexShader = compile vs_3_0 VSGenSilhouette();
      PixelShader  = compile ps_3_0 PSGenSilhouette();
   }

   pass P1
   {
      cullMode = CCW;
      VertexShader = compile vs_3_0 VSCartoon();
      PixelShader = compile ps_3_0 PSCartoon();
   }
}
