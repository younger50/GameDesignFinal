/*-----------------------------------------------
  Depth Map Shaders
  
  (C)2011, Chaun-Chang Wang, All Rights Reserved
 ------------------------------------------------*/

// constants
float4x4 mWVP;                          // from world to projection matrix
float4x4 mWV;                           // from world to view matrix
float nearPlane = 1.0f;                 // near plane position
float farPlane = 10000.0f;              // far plane position
float farD = 1.0f;
float2 pixelSize;                       // half pixel size


/*----------------------------------------------
  vertex shader output for depth map generation
 -----------------------------------------------*/
struct VS_OUTPUTS_DMG
{
   float4 pos   : POSITION;
   float4 color : COLOR0;
};


/*---------------------------------
  generate depth map vertex shader
 ----------------------------------*/
VS_OUTPUTS_DMG VSGenerateDepthMap(float4 vPos : POSITION)
{
   VS_OUTPUTS_DMG out1 = (VS_OUTPUTS_DMG) 0;
   float depth;
   float4 vPosInView;

   out1.pos = mul(vPos, mWVP);
   vPosInView = mul(vPos, mWV);
   depth = abs(vPosInView.z) - nearPlane;
   out1.color = depth;
   return out1;
}


/*--------------------------------------
  generate unit depth map vertex shader
 ---------------------------------------*/
VS_OUTPUTS_DMG VSGenerateUnitDepthMap(float4 vPos : POSITION)
{
   VS_OUTPUTS_DMG out1 = (VS_OUTPUTS_DMG) 0;
   float depth;
   float4 vPosInView;

   out1.pos = mul(vPos, mWVP);
   vPosInView = mul(vPos, mWV);
   depth = (abs(vPosInView.z) - nearPlane)/(farPlane - nearPlane);
   out1.color = depth;
   return out1;
}


/*------------------------------------
  generate distance map vertex shader
 -------------------------------------*/
VS_OUTPUTS_DMG VSGenerateDistanceMap(float4 vPos : POSITION)
{
   VS_OUTPUTS_DMG out1 = (VS_OUTPUTS_DMG) 0;
   float depth;
   float4 vPosInView;

   out1.pos = mul(vPos, mWVP);
   vPosInView = mul(vPos, mWV);
   if (vPosInView.z < 0.0f) {
      depth = length(vPosInView.xyz);
   }
   else {
      depth = 0.0f;
   }
   out1.color = depth;
   return out1;
}


/*------------------------------
  clear depth map vertex shader
 -------------------------------*/
VS_OUTPUTS_DMG VSClearDepthMap(float4 vPos : POSITION)
{
   VS_OUTPUTS_DMG out1 = (VS_OUTPUTS_DMG) 0;
   out1.pos.xy = vPos.xy + pixelSize;
   out1.pos.z = 0.5f;
   out1.pos.w = 1.0f;
   out1.color = farD;
   return out1;
}


/*--------------------------------
  generate depth map pixel shader
 ---------------------------------*/
float4 PSGenerateDepthMap(in float4 pColor : COLOR0) : COLOR0
{
   float4 finalColor = pColor;
   return finalColor;
}


/*--------------------------
  techniques in this effect
 ---------------------------*/

// depth map generation shader
technique sDepthMap
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSGenerateDepthMap();
      PixelShader  = compile ps_3_0 PSGenerateDepthMap();
   }
}


// unit depth map generation shader
technique sUnitDepthMap
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSGenerateUnitDepthMap();
      PixelShader  = compile ps_3_0 PSGenerateDepthMap();
   }
}


// distance map generation shader
technique sDistanceMap
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSGenerateDistanceMap();
      PixelShader  = compile ps_3_0 PSGenerateDepthMap();
   }
}


// clear depth map background
technique sClearBackground
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSClearDepthMap();
      PixelShader  = compile ps_3_0 PSGenerateDepthMap();
   }
}
