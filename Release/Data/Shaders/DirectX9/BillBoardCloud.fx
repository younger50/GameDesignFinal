/*--------------------------------------------------------------------
  Billboard Cloud Shader Using Instancing Feature of Shader Model 3.0
  
  (C)2011-2012, Chaun-Chang Wang, All Rights Reserved
 ---------------------------------------------------------------------*/

// constants
float4x4 mView;                                        // viewing matrix 
float4x4 mProjection;                                  // projection matrix 
float nearPlane = 1.0f;                                // near plane position
float farPlane = 1.0f;                                 // near plane position
bool beAlpha = true;

// textures
bool beMap0 = false;
bool beAlpha0 = false;
bool beAlpha1 = false;

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


texture distanceMap;
sampler2D distanceMapSampler = sampler_state
{
   Texture = <distanceMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;
};


/*----------------------------------------------
  vertex shader output for hard billboard cloud
 -----------------------------------------------*/
struct VS_OUTPUTS_HBC
{
   float4 pos   : POSITION;
   float4 color : COLOR0;
   float2 tex0  : TEXCOORD0;
};


/*----------------------------------------------
  vertex shader output for soft billboard cloud
 -----------------------------------------------*/
struct VS_OUTPUTS_SBC
{
   float4 pos   : POSITION;
   float4 color : COLOR0;
   float2 tex0  : TEXCOORD0;
   float3 posV  : TEXCOORD1;
   float2 dTex  : TEXCOORD2;
   float3 bbPos : TEXCOORD3;
   float2 bbRD  : TEXCOORD4;
};


/*-------------------------------------
  shader to find the billboard opacity
 --------------------------------------*/
float CalculateOpacity(float3 P, float3 Q, float r, float2 vTex, float density)
{
   float alpha = 0.0;
   float d = length(P.xy - Q.xy);
   if (d < r) {
      float Q1 = length(Q);
      float fMin = -nearPlane*Q1/Q.z;
      float w = sqrt(r*r - d*d);
      float F = Q1 - w;
      float B = Q1 + w;
      float Ds = tex2D(distanceMapSampler, vTex).r;
      float ds = min(Ds, B) - max(fMin, F);
      if (Ds > nearPlane) {
         alpha = 1.0f - exp(-density*(1- d/r)*ds);
      }
      else {
         alpha = 1.0f;
      }
   }
   return alpha;
}


/*-----------------------------------
  hard billboard cloud vertex shader
 ------------------------------------*/
VS_OUTPUTS_HBC VSHardBillBoard(float4 vPos  : POSITION,   // vertex position in local
                               float3 color : TEXCOORD0,  // 1st texture coordinate = vertex color
                               float2 vTex0 : TEXCOORD1,  // 2nd texture coordinate = texture uv
                               float4 pPos  : TEXCOORD2,  // 3rd texture coordinate = billboard position in world & particle life
                               float4 pTex  : TEXCOORD3)  // 4th texture coordinate = texture uv modifier
{
   VS_OUTPUTS_HBC out1 = (VS_OUTPUTS_HBC) 0;
   
   float4 vPosInView = mul(float4(pPos.xyz, 1.0f), mView) + float4(vPos.xyz, 0.0f);
   
   out1.pos = mul(vPosInView, mProjection);

   if (beAlpha) {
      out1.color = float4(color, pPos.w);
   }
   else {
      out1.color = float4(color*pPos.w, pPos.w);
   }
   out1.tex0 = float2(pTex.x, 1.0f - pTex.y - pTex.w) + vTex0*pTex.zw;
   return out1;
}


/*----------------------------------
  hard billboard cloud pixel shader
 -----------------------------------*/
float4 PSHardBillBoard(in float4 pColor : COLOR0,
                       in float2 pTex0  : TEXCOORD0) : COLOR0
{
   float4 finalColor;
   
   if (beMap0) {
      finalColor = tex2D(colorMapSampler, pTex0)*pColor;
   }
   else {
      finalColor = pColor;
   }

   if (beAlpha0 > 0) {
      float4 colorA = tex2D(alphaMapSampler, pTex0);
      finalColor *= colorA.rrrr;
   }

   return finalColor;
}


/*---------------------------------
  vertex shader for soft billboard
 ----------------------------------*/
VS_OUTPUTS_SBC VSSoftBillBoard(float4 vPos  : POSITION,   // vertex position in local
                               float3 color : TEXCOORD0,  // 1st texture coordinate = vertex color
                               float2 vTex0 : TEXCOORD1,  // 2nd texture coordinate = texture uv
                               float4 pPos  : TEXCOORD2,  // 3rd texture coordinate = billboard position in world & particle life
                               float4 pTex  : TEXCOORD3,  // 4th texture coordinate = texture uv modifier
                               float2 rd    : TEXCOORD4)  // 5th texture coordinate = billboard radius and density
{
   VS_OUTPUTS_SBC out1 = (VS_OUTPUTS_SBC) 0;
   
   float4 bbPosInView = mul(float4(pPos.xyz, 1.0f), mView);
   float4 vPosInView = bbPosInView + float4(vPos.xyz, 0.0f);
   
   float4 pos = mul(vPosInView, mProjection);

   out1.pos = pos;
   out1.posV = vPosInView.xyz;

   if (beAlpha) {
      out1.color = float4(color, pPos.w);
   }
   else {
      out1.color = float4(color*pPos.w, pPos.w);
   }

   out1.tex0 = pTex.xy + vTex0*pTex.zw;
   out1.bbPos = bbPosInView.xyz;
   out1.bbRD = rd;

   out1.dTex = float2(0.5f, -0.5f)*(pos.xy/pos.ww) + 0.5f.xx;	

   return out1;
}


/*--------------------------------
  pixel shader for soft billboard
 ---------------------------------*/
float4 PSSoftBillBoard(in float4 vColor : COLOR,
                       in float2 texUV  : TEXCOORD0,
                       in float3 pos3D  : TEXCOORD1, 
                       in float2 vTex   : TEXCOORD2,
                       in float3 bbPos  : TEXCOORD3,
                       in float2 bbRD   : TEXCOORD4) : COLOR0
{
   float alpha = CalculateOpacity(bbPos, pos3D, bbRD.x, vTex, bbRD.y);
   float4 finalColor;
   
   if (beAlpha) {
      finalColor = float4(vColor.rgb, vColor.a*alpha);
   }
   else {
      finalColor = alpha*vColor;
   }

   if (beMap0) {
      finalColor = tex2D(colorMapSampler, texUV)*finalColor;
   }

   if (beAlpha1 > 0) {
      float4 colorA = tex2D(alphaMapSampler, texUV);
      finalColor *= colorA.rrrr;
      finalColor = 1.0;
   }

   return finalColor;
}


/*--------------------------
  techniques in this effect
 ---------------------------*/

// hard billboard cloud shader
technique hBillboardCloud
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSHardBillBoard();
      PixelShader  = compile ps_3_0 PSHardBillBoard();
   }
}


// soft billboard cloud shader
technique sBillboardCloud
{
   pass P0
   {
      VertexShader = compile vs_3_0 VSSoftBillBoard();
      PixelShader  = compile ps_3_0 PSSoftBillBoard();
   }
}
