// This shader implements Billboard Rendering
//

cbuffer cbFrame : register(b0)
{
   float4 dif			 : packoffset(c0);    // diffuse component of the material
}


// pixel shader input
struct PS_INPUT
{
   float2 vTex0 : TEXCOORD0;
   float4 vPos  : SV_POSITION;
};

   
// the pixel shader
float4 PSMain(PS_INPUT in1) : SV_TARGET
{
   float4 rgba = dif;
   return rgba;
}