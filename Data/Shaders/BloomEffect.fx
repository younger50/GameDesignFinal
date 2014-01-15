/*---------------------------------------------------------
  Bloom Shader
  
  (C)2008-2009, Chaun-Chang Wang, All Rights Reserved
 ----------------------------------------------------------*/
float fIteration;
float2 pixelSize;
float exposureLevel;
float weight;

texture renderMap;
sampler renderMapSampler = sampler_state
{
   Texture = <renderMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = LINEAR;   
   AddressU  = Clamp;
   AddressV  = Clamp;
};

texture fullResMap;
sampler fullResMapSampler = sampler_state
{
   Texture = <fullResMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = LINEAR;   
   AddressU  = Clamp;
   AddressV  = Clamp;
};


// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUTScaleBuffer
{
   float4 pos : POSITION;
	float2 tex : TEXCOORD0;	
};

// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUTScaleBuffer VSScaleBuffer(float4 pos : POSITION, float2 tex : TEXCOORD0)
{
   VS_OUTPUTScaleBuffer out1 = (VS_OUTPUTScaleBuffer) 0;        
   out1.pos.xy = pos.xy + pixelSize;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;

   out1.tex = tex;
   return out1;
}

// -------------------------------------------------------------
// Pixel Shader (input channels):output channel
// -------------------------------------------------------------
float4 PSScaleBuffer(float2 tex	: TEXCOORD0) : COLOR			
				
{
   float4 rgba = tex2D(renderMapSampler, tex);
   float luminance = dot(rgba, float3(0.299f, 0.587f, 0.114f));

   return rgba*luminance;
}

// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUTBloom
{
   float4 pos         : POSITION;
   float2 topLeft     : TEXCOORD0;
   float2 topRight    : TEXCOORD1;
   float2 bottomRight : TEXCOORD2;
   float2 bottomLeft  : TEXCOORD3;	
};


// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUTBloom VSBloom(float4 pos : POSITION, float2 tex : TEXCOORD0)
{
   VS_OUTPUTBloom out1 = (VS_OUTPUTBloom) 0;        
   out1.pos.xy = pos.xy + pixelSize.xy;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;
   
   float2 halfPixelSize = pixelSize/2.0f;
   float2 dUV = pixelSize.xy*fIteration + halfPixelSize;
	
   // sample top left
   out1.topLeft = float2(tex.x - dUV.x, tex.y + dUV.y); 
	
   // sample top right
   out1.topRight = float2(tex.x + dUV.x, tex.y + dUV.y);
	
   // sample bottom right
   out1.bottomRight = float2(tex.x + dUV.x, tex.y - dUV.y);
	
   // sample bottom left
   out1.bottomLeft = float2(tex.x - dUV.x, tex.y - dUV.y);

   return out1;
}


// -------------------------------------------------------------
// Pixel Shader (input channels):output channel
// -------------------------------------------------------------
float4 PSBloom(float2 topLeft     : TEXCOORD0,
               float2 topRight    : TEXCOORD1,
               float2 bottomRight : TEXCOORD2,
               float2 bottomLeft  : TEXCOORD3) : COLOR0
{
   float4 addedBuffer = 0.0f;

   // sample top left
   addedBuffer = tex2D(renderMapSampler, topLeft);

   // sample top right
   addedBuffer += tex2D(renderMapSampler, topRight);
	
   // sample bottom right
   addedBuffer += tex2D(renderMapSampler, bottomRight);
	
   // sample bottom left
   addedBuffer += tex2D(renderMapSampler, bottomLeft);
	
   // average
   return addedBuffer *= 0.25f;
}

// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUTScreen
{
   float4 pos : POSITION;
   float2 tex : TEXCOORD0;
};

// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUTScreen VSScreen(float4 pos : POSITION, float2 tex : TEXCOORD0)
{
   VS_OUTPUTScreen out1 = (VS_OUTPUTScreen) 0;        
   out1.pos.xy = pos.xy;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;
	
   out1.tex = tex;
   return out1;
}


// -------------------------------------------------------------
// Pixel Shader (input channels):output channel
// -------------------------------------------------------------
float4 PSScreen(in float2 tex : TEXCOORD0) : COLOR0
{
   float4 fullScreenImage = tex2D(fullResMapSampler, tex);
   float4 blurredImage = tex2D(renderMapSampler, tex);
	
   float4 color = lerp(fullScreenImage, blurredImage, weight);
   return color*exposureLevel;
}


technique ScaleBuffer
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSScaleBuffer();
      PixelShader  = compile ps_3_0 PSScaleBuffer();
   }
}

technique Bloom
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSBloom();
      PixelShader  = compile ps_3_0 PSBloom();
   }
}

technique BlendBuffer
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSScreen();
      PixelShader  = compile ps_3_0 PSScreen();
   }
}


