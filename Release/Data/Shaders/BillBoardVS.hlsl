// This shader implements billboard rendering
//

cbuffer cbFrame : register(b0)
{
   matrix mWVP                : packoffset(c0);           // matrix from local to screen space
};


// vertex shader input
struct VS_INPUT
{
   float4 inPos  : POSITION;
   float2 inTex0 : TEXCOORD0;
};


// vertex shader output
struct VS_OUTPUT
{
   float2 tex0 : TEXCOORD0;
   float4 pos  : SV_POSITION;
};


// the vertex shader
VS_OUTPUT VSMain(VS_INPUT in1)
{
   VS_OUTPUT out1 = (VS_OUTPUT) 0;
   
   // get the vertex in screen space
   out1.pos = mul(in1.inPos, mWVP);
   out1.tex0 = in1.inTex0;

   return out1;
}
