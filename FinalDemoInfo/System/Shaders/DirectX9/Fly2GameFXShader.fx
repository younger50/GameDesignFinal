// This shader implements game FX shaders
// C.Wang 0624, 2013
//

float4x4 mWVP;
float4 dif;
bool beColormap = true;
bool beAlpha = false;
   
texture colorMap;
sampler2D colorMapSampler = sampler_state
{
   Texture = <colorMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;
};


texture alphaMap;
sampler2D alphaMapSampler = sampler_state
{
   Texture = <alphaMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;
};

   
struct VS_OUTPUT
{
   float4 pos   : POSITION;
   float2 uv    : TEXCOORD0;
   float4 color : TEXCOORD1;
};
   

// vertex shader for PlateTex2 & GeometryTex2
VS_OUTPUT CommonTex2VS(float4 inPos   : POSITION,
                       float3 inColor : COLOR,
                       float2 inTex0  : TEXCOORD0)
{
   VS_OUTPUT out1 = (VS_OUTPUT) 0;

   out1.pos = mul(inPos, mWVP);
   out1.color = float4(inColor, dif.a);
   out1.uv = inTex0;

   return out1;
}


// the vertex shader for SwordTex2
VS_OUTPUT SwordTex2VS(float4 inPos : POSITION,
                      float4 inColor : COLOR,
                      float2 inTex0 : TEXCOORD0)
{
   VS_OUTPUT out1 = (VS_OUTPUT) 0;

   out1.pos = mul(inPos, mWVP);
   out1.color = inColor;
   out1.uv = inTex0;

   return out1;
}


// PlateTex2 pixel shader
void PlateTex2PS(float2 vTex   : TEXCOORD0,
                 float4 vColor : TEXCOORD1,
             out float4 oCol   : COLOR)
{
   if (beColormap > 0) {
      float4 colorT = tex2D(colorMapSampler, vTex);
      oCol = vColor*colorT;
   }
   else {
      oCol = vColor;
   }

   if (beAlpha > 0) {
      float4 colorA = tex2D(alphaMapSampler, vTex);
      oCol *= colorA.rrrr;
   }
}


// GeometryTex2 pixel shader
void GeometryTex2PS(float2 vTex   : TEXCOORD0,
                    float4 vColor : TEXCOORD1,
                out float4 oCol   : COLOR)
{
   if (beColormap > 0) {
      float4 colorT = tex2D(colorMapSampler, vTex);
      oCol = colorT;
      oCol.a = colorT.a*vColor.a;
   }
   else {
      oCol = float4(1, 1, 1, vColor.a);
   }

   if (beAlpha > 0) {
      float4 colorA = tex2D(alphaMapSampler, vTex);
      oCol *= colorA.rrrr;
   }
}


// SwordTex2 pixel shader
void SwordTex2PS(float2 vTex   : TEXCOORD0,
                 float4 vColor : TEXCOORD1,
             out float4 oCol   : COLOR)
{
   if (beColormap > 0) {
      float4 colorT = tex2D(colorMapSampler, vTex);
      oCol = vColor*colorT;
   }
   else {
      oCol = vColor;
   }

   if (beAlpha > 0) {
      float4 colorA = tex2D(alphaMapSampler, vTex);
      oCol *= colorA.rrrr;
   }
}


technique PlateTex2
{
   pass P0
   {
      VertexShader = compile vs_3_0 CommonTex2VS();
      PixelShader = compile ps_3_0 PlateTex2PS();
   }
}


technique GeometryTex2
{
   pass P0
   {
      VertexShader = compile vs_3_0 CommonTex2VS();
      PixelShader = compile ps_3_0 GeometryTex2PS();
   }
}


technique SwordTex2
{
   pass P0
   {
      VertexShader = compile vs_3_0 SwordTex2VS();
      PixelShader = compile ps_3_0 SwordTex2PS();
   }
}