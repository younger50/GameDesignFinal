/*---------------------------------------------------------
  HDR Shader version 2.0
  
  (C)2008-2009, Chaun-Chang Wang, All Rights Reserved
 ----------------------------------------------------------*/
float2 pixelSize;           // Data 1, 2
float exposureLevel = 2.0f; // Data 0
float weight = 0.7f;        // interpolation weight between the rendering and blurred result
bool beVignette = false;    // be vignette in the corner ?
float w2 = 3.0f;            // the white^2
float fIteration = 1.0f;    // the interation count 
float blurThreadold = 0.7f; // the threshold for blur weight

// textures
texture bloomMap;
sampler bloomMapSampler = sampler_state
{
   Texture = <bloomMap>;
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


/*------------------------------------
  vertex structure for image exposure
 -------------------------------------*/
struct VSRenderTargetOutput
{
   float4 pos : POSITION;
   float2 tex : TEXCOORD0;	
};


/*---------------------------------
  vertex shader for image exposure
 ----------------------------------*/
VSRenderTargetOutput VSRenderTarget(float4 pos : POSITION)
{
   VSRenderTargetOutput out1 = (VSRenderTargetOutput) 0;
   out1.pos.xy = pos.xy + pixelSize.xy;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;
	
   out1.tex = float2(0.5f, -0.5f)*pos.xy + 0.5f.xx;	
	
   return out1;
}


/*--------------------------------
  pixel shader for image exposure
 ---------------------------------*/
float4 PSExposure(float2 tex :TEXCOORD0): COLOR
{
   return exposureLevel*tex2D(fullResMapSampler, tex);
}



/*-----------------------------------------
  vertex output for downsampling the image
 ------------------------------------------*/
struct VS_OUTPUTScaleBuffer
{
   float4 pos : POSITION;
   float2 tex : TEXCOORD0;
};


/*-----------------------------------------
  vertex shader for downsampling the image
 ------------------------------------------*/
VS_OUTPUTScaleBuffer VSScaleBuffer(float4 pos : POSITION, float2 tex : TEXCOORD0)
{
   VS_OUTPUTScaleBuffer out1 = (VS_OUTPUTScaleBuffer) 0;        
   out1.pos.xy = pos.xy + pixelSize;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;

   out1.tex = tex;
   return out1;
}


/*----------------------------------------
  pixel shader for downsampling the image
 -----------------------------------------*/
float4 PSScaleBuffer(float2 tex :TEXCOORD0): COLOR				
{
   float4 rgba = tex2D(bloomMapSampler, tex);
   float luminance = dot(rgba.rgb, float3(0.299f, 0.587f, 0.114f));
   if (luminance > blurThreadold) {
      rgba = rgba*luminance;
   }
   else {
      rgba = float4(0.0, 0.0, 0.0, rgba.a);
   }
   return rgba;
}


/*-------------------------------------------------
  vertex output for blooming the downsampled image
 --------------------------------------------------*/
struct VS_OUTPUTBloom
{
   float4 pos         : POSITION;
   float2 topLeft     : TEXCOORD0;
   float2 topRight    : TEXCOORD1;
   float2 bottomRight : TEXCOORD2;
   float2 bottomLeft  : TEXCOORD3;	
};


/*-------------------------------------------------
  vertex shader for blooming the downsampled image
 --------------------------------------------------*/
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


/*------------------------------------------------
  pixel shader for blooming the downsampled image
 -------------------------------------------------*/
float4 PSBloom(float2 topLeft     : TEXCOORD0,
               float2 topRight    : TEXCOORD1,
               float2 bottomRight : TEXCOORD2,
               float2 bottomLeft  : TEXCOORD3) : COLOR0
{
   float4 addedBuffer = 0.0f;

   // sample top left
   addedBuffer = tex2D(bloomMapSampler, topLeft);

   // sample top right
   addedBuffer += tex2D(bloomMapSampler, topRight);
	
   // sample bottom right
   addedBuffer += tex2D(bloomMapSampler, bottomRight);
	
   // sample bottom left
   addedBuffer += tex2D(bloomMapSampler, bottomLeft);
	
   // average
   return addedBuffer *= 0.25f;
}


/*-------------------------------------
  vertex output for final tone mapping
 --------------------------------------*/
struct VS_OUTPUTScreen
{
   float4 pos : POSITION;
   float2 tex : TEXCOORD0;	
};


/*-------------------------------------
  vertex shader for final tone mapping
 --------------------------------------*/
VS_OUTPUTScreen VSTonemap(float4 pos : POSITION, float2 tex : TEXCOORD0)
{
   VS_OUTPUTScreen out1 = (VS_OUTPUTScreen) 0;        
   out1.pos.xy = pos.xy;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;
   out1.tex = tex;	
   return out1;
}


/*------------------------------------
  pixel shader for final tone mapping
 -------------------------------------*/
float4 PSTonemap(float2 tex : TEXCOORD0) : COLOR0
{
   float4 color = tex2D(fullResMapSampler, tex);
   float4 blurredImage = tex2D(bloomMapSampler, tex);
         
   // add the blurred one to the full sceen image
   color.rgb += blurredImage.rgb*weight;
   
   // apply tone mapping
   color.rgb = color.rgb/(color.rgb + 1.0f)*(1.0 + color.rgb/w2);
   
   // apply the vignette
   if (beVignette) {
      // range -0.5..0.5
      tex -= 0.5f;	
      float vignette = 1 - dot(tex, tex);
	
      // multiply color with vignette
      color.rgb = color.rgb*vignette;
   }
   
   return color;
}


// all techniques
technique Exposure
{
   pass P0
   {
      VertexShader = compile vs_2_0 VSRenderTarget();
      PixelShader  = compile ps_2_0 PSExposure();
   }
}


technique ScaleBuffer
{
   pass P0
   {
      VertexShader = compile vs_2_0 VSScaleBuffer();
      PixelShader  = compile ps_2_0 PSScaleBuffer();
   }
}


technique Bloom
{
   pass P0
   {
      VertexShader = compile vs_2_0 VSBloom();
      PixelShader  = compile ps_2_0 PSBloom();
   }
}


technique Tonemap
{
   pass P0
   {
      VertexShader = compile vs_2_0 VSTonemap();
      PixelShader  = compile ps_2_0 PSTonemap();
   }
}
