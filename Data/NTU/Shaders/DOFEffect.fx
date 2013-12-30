/*---------------------------------------------------------
  HDR Shader integerated with Depth-of-field & Motion Blur
  
  (C)2008, Chaun-Chang Wang, All Rights Reserved
 ----------------------------------------------------------*/
// for depth-of-field
float inside = 50.0f;       // DOF distance
float outside = 650.0f;
float dist = 300.0f;
float2 pixelSizeHigh;

// contains poisson-distributed positions on the unit circle
float2 poisson[9] = { float2(  0.0,   0.0),
                      float2(  0.67,  0.0),
                      float2( -0.67,  0.0),
                      float2(  0.0,  -0.67),
                      float2(  0.67, -0.67),
                      float2( -0.67, -0.67),
                      float2(  0.0,   0.67),
                      float2(  0.67,  0.67),
                      float2( -0.67,  0.67)};

float2 vMaxCoC = float2(4.0f, 8.0f);

// for the depthmap generation
float4x4 mWVP;
float4x4 mWV;

// textures
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

texture depthMap;
sampler depthMapSampler = sampler_state
{
   Texture = <depthMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = LINEAR;
   AddressU  = Clamp;
   AddressV  = Clamp;
};


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
VS_OUTPUTScreen VSDoDOF(float4 pos : POSITION, float2 tex : TEXCOORD0)
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
float4 PSDoDOF(float2 tex : TEXCOORD0) : COLOR0
{      
   // convert depth into blur widths in pixels
   float centerDepth = tex2D(depthMapSampler, tex).x;
   float discRadius = abs(centerDepth*vMaxCoC.y - vMaxCoC.x);
      
   float4 color;
   if (discRadius <= 1) {
      color = tex2D(fullResMapSampler, tex);
   }
   else {
      float4 cOut = 0.0f;
      float depT = 0.0f;
      float2 range = pixelSizeHigh*discRadius;
      for (int t = 0; t < 9; t++) {
         // fetch high-res tap
         float2 coordHigh = tex + (range*poisson[t]);
         float4 tapHigh = tex2D(fullResMapSampler, coordHigh);
         if (tapHigh.a == 0)
         {
            cOut = tex2D(fullResMapSampler, tex);
            depT = 1;
            break;
         }
         
         // fetch the depth on tap
         float dep = tex2D(depthMapSampler, coordHigh).x;

         // apply leaking reduction: lower weight for taps that are
         // closer than the center tap and in focus
         dep = (dep >= centerDepth) ? 1.0 : abs(dep*2.0 - 1.0);

         // accumulate
         cOut += tapHigh*dep;
         depT += dep;
      }
      color = cOut/depT;
   }
      
   // return result
   return color;
}


/*---------------------------------------
  vertex output for depth map generation
 ----------------------------------------*/
struct VS_OUTPUT_Depth
{
   float4 pos : POSITION;
   float dep : TEXCOORD0;
};


/*---------------------------------------
  vertex shader for depth map generation
 ----------------------------------------*/
VS_OUTPUT_Depth VSGenDepth(float4 pos : POSITION)
{
   VS_OUTPUT_Depth vsOut = (VS_OUTPUT_Depth) 0; 

   // output position
   float4 p;
   
   vsOut.pos = mul(pos, mWVP);
   p = mul(pos, mWV);
   
   // depth value - TheFly's z is negative in view space when in front of the camera
   vsOut.dep = -p.z;
   return vsOut;
}


/*--------------------------------------
  pixel shader for depth map generation
 ---------------------------------------*/
float4 PSGenDepth(float depth : TEXCOORD0) : COLOR
{
   float f;   
   if (depth < dist) {
      // scale depth value between near blur distance and focal
      // distance to [-1, 0] range
      f = (depth - dist)/(dist - inside);
   }
   else {
      // scale depth value between focal distance and far blur
      // distance to [0, 1] range
      f = (depth - dist)/(outside - dist);
   }
   
   float depData = f*0.5f + 0.5f;
   return float4(depData, depData, depData, 1.0f);
}


// all techniques
technique DoDOF
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSDoDOF();
      PixelShader  = compile ps_3_0 PSDoDOF();
   }
}


technique GenDepth
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSGenDepth();
      PixelShader  = compile ps_3_0 PSGenDepth();
   }
}