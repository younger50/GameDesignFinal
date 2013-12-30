// This shader implements skin deformation (2 weights)
// C.Wang 1230, 2005
//

float4x4 viewProj;         // the matrix from view to screen space
float4 mainLightPosition;  // main light position
float3 mainLightColor;     // main light color
float4 ambLgt;             // environment lighting
float4 amb;                // ambient component of the material
float4 dif;                // diffuse component of the material
float4x4 bone[16];         // in this shader we limit the bone number to 16
   
// vertex shader output data structure
struct VS_OUTPUT
{
   float4 pos : POSITION;  // vertex position
   float4 dif : COLOR0;    // vertex color
};

// vertex shader
VS_OUTPUT skinVS(float4 inPos : POSITION,
                 float3 inNorm : NORMAL,
                 float2 inTex : TEXCOORD0,
                 float4 boneData : TEXCOORD1)
{
   // initialize the output
   VS_OUTPUT o = (VS_OUTPUT) 0;
   
   int boneID1 = (int) boneData.x + 1;
   int boneID2 = (int) boneData.z + 1;
   float4x4 worldMatrix = bone[boneID1]*boneData.y + bone[boneID2]*boneData.w;
   
   // transform the vertex
   float4x4 m4x4 = mul(worldMatrix, viewProj);
      
   // calculate the normal & lighting vectors
   float4 a = mul(inPos, worldMatrix);
   float3 l = normalize(mainLightPosition.xyz - a.xyz);
   float3 n = normalize(mul(inNorm, (float3x3) worldMatrix));
      
   // calculate the N dot L for diffuse
   float d = saturate(dot(l, n));
      
   // transform the vertex to screen
   o.pos = mul(inPos, m4x4);
      
   // perform the lighting
   o.dif.rgb = ambLgt*amb + d*mainLightColor*dif;
      
   // set the opacity
   o.dif.a = dif.a;
   return o;
}


// the associated D3D Effect technique
technique skinDeform
{
   pass P0
   {
      VertexShader = compile vs_2_0 skinVS();
      PixelShader = NULL;
   }
}