// This shader implements Oren-Nayar shading with one color texture
// C.Wang 0624, 2013
//

float4x4 mWVP;
float4x4 mWorld;
float4 mainLightPosition;
float4 mainLightColor;
float4 mainLightDirection;
int mainLightType = 1;
float4 kd;
float4 ka;
float4 camPosition;
float4 skyColor = { 1.0f, 1.0f, 1.0f, 1.0f};           // sky color for ambient
float4 groundColor = { 0.2f, 0.2f, 0.2f, 0.2f};        // ground color for ambient
bool beColormap = false;
float rough = 0.7; 

texture sinTanMap;
sampler sinTanMapSampler = sampler_state
{
   Texture = <sinTanMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = NONE;

   AddressU = Clamp;
   AddressV = Clamp;   
};


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


struct VS_OUTPUT
{
   float4 pos  : POSITION;
   float2 tex0 : TEXCOORD0;
   float3 norm : TEXCOORD1;
   float3 lgt  : TEXCOORD2;
   float3 cam  : TEXCOORD3;
};
   
VS_OUTPUT OrenNayarVS(float4 inPos  : POSITION,
                      float3 inNorm : NORMAL,
                      float2 inTex  : TEXCOORD0)
{
   float4 wPos;
   VS_OUTPUT out1 = (VS_OUTPUT) 0;
      
   wPos = mul(inPos, mWorld);
   out1.pos = mul(inPos, mWVP);
   out1.norm = mul(inNorm, (float3x3) mWorld);

   // setup lighting direction
   if (mainLightType == 1) {
      out1.lgt = mainLightPosition.xyz - wPos.xyz;
   }
   else {
      out1.lgt = -mainLightDirection;
   }

   out1.cam = normalize(camPosition.xyz - wPos.xyz);
   out1.tex0 = inTex;

   return out1;
}
   
void OrenNayarPS(in float2 vTex  : TEXCOORD0,
                 in float3 vNorm : TEXCOORD1,
                 in float3 vLgt  : TEXCOORD2,
                 in float3 vCam  : TEXCOORD3,
                out float4 oCol  : COLOR0)
{
   float s2 = rough*rough;
   float A = 1 - 0.5*s2/(s2 + 0.33);
   float B = 0.45*s2/(s2 + 0.09);
   float3 skyDir = float3(0.0f, 0.0f, 1.0f);

   // calculate all vectors
   float3 N = normalize(vNorm);
   float3 L = normalize(vLgt);
   float3 V = normalize(vCam);
   float NL = dot(N, L);
   float VN = dot(V, N);
   
   // fetch texture with sin(alpha) + tan(alpha)
   float sinTan = tex2D(sinTanMapSampler, float2(saturate(NL), saturate(VN))).x;	
   
   float3 LProjected = normalize(L - NL*N);
   float3 VProjected = normalize(V - VN*N);
   float C = saturate(dot(LProjected, VProjected));

   // the ambient term
   float3 ambient = lerp(groundColor, skyColor, dot(N, skyDir)*0.5 + 0.5)*ka;
   
   //oCol.rgb = ambient + kd*saturate(NL * (A + B*C*sinTan))*mainLightColor;	
   oCol.rgb = ambient + kd*saturate(NL*(A + B*C*sinTan)*0.5 + 0.5)*mainLightColor;	
   oCol.a = kd.a;

   if (beColormap) {
      float4 colorT = tex2D(colorMapSampler, vTex);
      oCol = oCol*colorT;
   }
}

technique OrenNayarTex1
{
   pass P0
   {
      VertexShader = compile vs_3_0 OrenNayarVS();
      PixelShader = compile ps_3_0 OrenNayarPS();
   }
}