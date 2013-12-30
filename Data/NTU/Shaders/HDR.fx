float2 horztapOffs[7];		// Gauss blur horizontal texture coordinate offsets
float2 verttapOffs[7];		// Gauss blur vertical texture coordinate offsets
float4 texelWeight[16];
float2 pixelSize;          // Data 1, 2
float exposureLevel;       // Data 0

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
VS_OUTPUTScaleBuffer VSScaleBuffer(float4 pos : POSITION)
{
   VS_OUTPUTScaleBuffer out1 = (VS_OUTPUTScaleBuffer) 0;
   out1.pos.xy = pos.xy + pixelSize;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;

   out1.tex = float2(0.5f, -0.5f)*pos.xy + 0.5f.xx;	

   return out1;
}

// -------------------------------------------------------------
// Pixel Shader (input channels): output channel
// -------------------------------------------------------------
float4 PSScaleBuffer(float2 tex :TEXCOORD0): COLOR				
{
   float4 rgba = tex2D(renderMapSampler, tex);
   float luminance = dot(rgba, float3(0.299f, 0.587f, 0.114f));
   
   return rgba*luminance;
}

// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUT_GaussX
{
   float4 pos : POSITION;
   float2 tap0 : TEXCOORD0;
   float2 tap1 : TEXCOORD1;    
   float2 tap2 : TEXCOORD2;
   float2 tap3 : TEXCOORD3;
   float2 tap1Neg : TEXCOORD4;
   float2 tap2Neg : TEXCOORD5;        
   float2 tap3Neg : TEXCOORD6;        
};

// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUT_GaussX VSGaussX(float4 pos : POSITION)
{
   VS_OUTPUT_GaussX out1 = (VS_OUTPUT_GaussX) 0;
   out1.pos.xy = pos.xy + pixelSize;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;
	
   float2 tex = float2(0.5f, -0.5f)*pos.xy + 0.5f.xx;	
	
   out1.tap0 = tex;
   out1.tap1 = tex + horztapOffs[1];	
   out1.tap2 = tex + horztapOffs[2];
   out1.tap3 = tex + horztapOffs[3];
   out1.tap1Neg = tex - horztapOffs[1];
   out1.tap2Neg = tex - horztapOffs[2];			
   out1.tap3Neg = tex - horztapOffs[3];			

   return out1;
}


// -------------------------------------------------------------
// Pixel Shader (input channels): output channel
// -------------------------------------------------------------
float4 PSGaussX(float2 tap0 : TEXCOORD0,
                float2 tap1 : TEXCOORD1,    
                float2 tap2 : TEXCOORD2,
                float2 tap3 : TEXCOORD3,
                float2 tap1Neg : TEXCOORD4,
                float2 tap2Neg : TEXCOORD5,        
                float2 tap3Neg : TEXCOORD6) : COLOR0
{
   float4 color[7];
   float4 colorSum = 0.0f;	

   // sample inner taps
   color[0] = tex2D(renderMapSampler, tap0); 
   color[1] = tex2D(renderMapSampler, tap1);
   color[2] = tex2D(renderMapSampler, tap1Neg);
   color[3] = tex2D(renderMapSampler, tap2); 
   color[4] = tex2D(renderMapSampler, tap2Neg);
   color[5] = tex2D(renderMapSampler, tap3);
   color[6] = tex2D(renderMapSampler, tap3Neg); 
	
   colorSum = color[0]*texelWeight[0];
   colorSum += color[1]*texelWeight[1];	
   colorSum += color[2]*texelWeight[1];
   colorSum += color[3]*texelWeight[2];
   colorSum += color[4]*texelWeight[2];			
   colorSum += color[5]*texelWeight[3];	
   colorSum += color[6]*texelWeight[3];	
	
   // compute texture coordinates for other taps
   float2 tap4 = tap0 + horztapOffs[4];
   float2 tap5 = tap0 + horztapOffs[5];
   float2 tap6 = tap0 + horztapOffs[6];
   float2 tap4Neg = tap0 - horztapOffs[4];
   float2 tap5Neg = tap0 - horztapOffs[5];
   float2 tap6Neg = tap0 - horztapOffs[6];
	
   // sample outer taps
   color[0] = tex2D(renderMapSampler, tap4); 
   color[1] = tex2D(renderMapSampler, tap4Neg);
   color[2] = tex2D(renderMapSampler, tap5); 
   color[3] = tex2D(renderMapSampler, tap5Neg); 
   color[4] = tex2D(renderMapSampler, tap6); 
   color[5] = tex2D(renderMapSampler, tap6Neg); 

   colorSum += color[0]*texelWeight[4];
   colorSum += color[1]*texelWeight[4];
   colorSum += color[2]*texelWeight[5];
   colorSum += color[3]*texelWeight[5];
   colorSum += color[4]*texelWeight[6];
   colorSum += color[5]*texelWeight[6];	

   return colorSum;
}

// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUT_GaussY
{
   float4 pos : POSITION;
   float2 tap0 : TEXCOORD0;
   float2 tap1 : TEXCOORD1;    
   float2 tap2 : TEXCOORD2;
   float2 tap3 : TEXCOORD3;
   float2 tap1Neg : TEXCOORD4;
   float2 tap2Neg : TEXCOORD5;        
   float2 tap3Neg : TEXCOORD6;        
};

// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUT_GaussY VSGaussY(float4 pos : POSITION)
{
   VS_OUTPUT_GaussY out1 = (VS_OUTPUT_GaussY) 0; 
   out1.pos.xy = pos.xy + pixelSize;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;

   float2 tex = float2(0.5f, -0.5f)*pos.xy + 0.5f.xx;	
   out1.tap0 = tex;
   out1.tap1 = tex + verttapOffs[1];	
   out1.tap2 = tex + verttapOffs[2];
   out1.tap3 = tex + verttapOffs[3];
   out1.tap1Neg = tex - verttapOffs[1];
   out1.tap2Neg = tex - verttapOffs[2];			
   out1.tap3Neg = tex - verttapOffs[3];			

   return out1;
}

// -------------------------------------------------------------
// Pixel Shader (input channels): output channel
// -------------------------------------------------------------
float4 PSGaussY(float2 tap0 : TEXCOORD0,
                float2 tap1 : TEXCOORD1,    
                float2 tap2 : TEXCOORD2,
                float2 tap3 : TEXCOORD3,
                float2 tap1Neg : TEXCOORD4,
                float2 tap2Neg : TEXCOORD5,        
                float2 tap3Neg : TEXCOORD6) : COLOR0
{
   float4 color[7];
   float4 colorSum = 0.0f;
	
   // sample inner taps
   color[0] = tex2D(renderMapSampler, tap0);
   color[1] = tex2D(renderMapSampler, tap1);
   color[2] = tex2D(renderMapSampler, tap1Neg);
   color[3] = tex2D(renderMapSampler, tap2);
   color[4] = tex2D(renderMapSampler, tap2Neg);
   color[5] = tex2D(renderMapSampler, tap3);
   color[6] = tex2D(renderMapSampler, tap3Neg);
	
   colorSum = color[0]*texelWeight[0];
   colorSum += color[1]*texelWeight[1];	
   colorSum += color[2]*texelWeight[1];
   colorSum += color[3]*texelWeight[2];
   colorSum += color[4]*texelWeight[2];			
   colorSum += color[5]*texelWeight[3];	
   colorSum += color[6]*texelWeight[3];	

   // compute texture coordinates for other taps
   float2 tap4 = tap0 + verttapOffs[4];
   float2 tap5 = tap0 + verttapOffs[5];
   float2 tap6 = tap0 + verttapOffs[6];
   float2 tap4Neg = tap0 - verttapOffs[4];
   float2 tap5Neg = tap0 - verttapOffs[5];
   float2 tap6Neg = tap0 - verttapOffs[6];

   // sample outer taps
   color[0] = tex2D(renderMapSampler, tap4);
   color[1] = tex2D(renderMapSampler, tap4Neg);
   color[2] = tex2D(renderMapSampler, tap5);
   color[3] = tex2D(renderMapSampler, tap5Neg);
   color[4] = tex2D(renderMapSampler, tap6);
   color[5] = tex2D(renderMapSampler, tap6Neg);

   colorSum += color[0]*texelWeight[4];
   colorSum += color[1]*texelWeight[4];
   colorSum += color[2]*texelWeight[5];
   colorSum += color[3]*texelWeight[5];
   colorSum += color[4]*texelWeight[6];
   colorSum += color[5]*texelWeight[6];	
	
   return colorSum;
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
VS_OUTPUTScreen VSScreen(float4 pos : POSITION)
{
   VS_OUTPUTScreen out1 = (VS_OUTPUTScreen) 0;        
   out1.pos.xy = pos.xy;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;
   out1.tex = float2(0.5f, -0.5f)*pos.xy + 0.5f.xx;	
   return out1;
}


// -------------------------------------------------------------
// Pixel Shader (input channels): output channel
// -------------------------------------------------------------
float4 PSScreen(float2 tex : TEXCOORD0) : COLOR0
{
   float4 fullScreenImage = tex2D(fullResMapSampler, tex);
   float4 blurredImage = tex2D(renderMapSampler, tex);

   // interpolate the full sceen image (0.4) with the blurred one (0.6)
   float4 color = lerp(fullScreenImage, blurredImage, 0.7f);

   // range -0.5..0.5
   //tex -= 0.5f;	
   //float vignette = 1 - dot(tex, tex);	
	
   // multiply color with vignette^4
   //color = color*vignette*vignette*vignette*vignette;
   color.a = 1.0f;
   
   // apply simple exposure (reference : 0.55)
   color *= exposureLevel;	
   return pow(color, 0.55f);
   
   //return blurredImage;
}


technique scaleBuffer
{
   pass P0
   {
      VertexShader = compile vs_2_0 VSScaleBuffer();
      PixelShader  = compile ps_2_0 PSScaleBuffer();
   }
}

technique gaussX
{
   pass P0
   {
      VertexShader = compile vs_2_0 VSGaussX();
      PixelShader  = compile ps_2_0 PSGaussX();
   }
}

technique gaussY
{
   pass P0
   {
      VertexShader = compile vs_2_0 VSGaussY();
      PixelShader  = compile ps_2_0 PSGaussY();
   }
}

technique screenblit
{
   pass P0
   {
      VertexShader = compile vs_2_0 VSScreen();
      PixelShader  = compile ps_2_0 PSScreen();
   }
}