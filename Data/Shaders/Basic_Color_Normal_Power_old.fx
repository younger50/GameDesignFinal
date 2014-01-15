
float4x4 mWVP;            // matrix from local to screen space
float4x4 mWorld;          // matrix from local to global space
float4 mainLightPosition; // position of the "mainLight"
float4 mainLightColor;    // color of the "mainLight"
float4 skyColor; 
float4 groundColor;
float4 amb;               // ambient component of the material
float4 dif;               // diffuse component of the material
float4 spe;               // specular component of the material
float power;              // material shineness
float4 camPosition;       // camera position
float  bumpy = 10.0f;

float4x4 mWorldInv;

texture ColorMap, NormalMap,SpecularMap;
sampler2D ColorMapSampler = sampler_state
{
   Texture = <ColorMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;
};

sampler2D NormalMapSampler = sampler_state
{
   Texture = <NormalMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;
};

sampler2D SpecularMapSampler = sampler_state
{
   Texture = <SpecularMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;   
};


// vertex shader output
struct VS_OUTPUT
{
   float4 pos  : POSITION;
   float2 tex0 : TEXCOORD0;
   float3 lgt  : TEXCOORD1;
   float3 cam  : TEXCOORD2;
   float3 sky  : TEXCOORD3;     // the sky vector in tangent space
   float3 nor  : TEXCOORD4;
   float3 tan  : TEXCOORD5;
   float3 binor : TEXCOORD6;
};

// the vertex shader
VS_OUTPUT PhongVS(float4 inPos   : POSITION, 
                  float3 inNorm  : NORMAL, 
                  float2 inTex   : TEXCOORD0,
                  float3 inTangU : TANGENT)
{
   //初始化輸出
   VS_OUTPUT out1 = (VS_OUTPUT) 0;

   // convert the vertex from local to global 將傳入的位置從LOCAL轉WORLD   
   float4 a = mul(inPos, mWorld);
   
   // get the vertex in screen space 把頂點位置轉到螢幕上
   out1.pos = mul(inPos, mWVP);
   
   // prepare the normal, lighting direction, camera vector for pixel shader 產生3*3矩陣轉至矩陣(World to Tangent)
   //[0]=U  [1]=V  [2]=W
   float3x3 wToTangent;
   wToTangent[0] = mul(inTangU, mWorld);
   wToTangent[1] = mul(cross(inTangU, inNorm), mWorld);
   wToTangent[2] = mul(inNorm, mWorld);
   
   //tangent space
   out1.tan =  wToTangent[0];
   out1.binor =  wToTangent[1];
   out1.nor =  wToTangent[2];
   
   // transform the rest data to tangent space  計算燈光與攝影機到gpos的單位向量(再tangent做標系統)
   out1.lgt = normalize(mainLightPosition.xyz - a.xyz);
   out1.cam = normalize(camPosition.xyz - a.xyz);
   
   // pass the texture coordinate 貼圖座標不要轉換
   out1.tex0 = inTex;

   // transform the sky vector to tangent space 轉換sky vector到tangent
   float3 sky = float3(0.0f, 0.0f, 1.0f);
   out1.sky = normalize(mul(wToTangent, sky));

   return out1;
}
   
// the pixel shader
void PhongPS(in float2 vTex : TEXCOORD0,
             in float3 vLgt : TEXCOORD1,
             in float3 vCam : TEXCOORD2,
             in float3 vSky : TEXCOORD3,    // the sky vector for tangent space
             in float3 nor  : TEXCOORD4,
			 in float3 tan  : TEXCOORD5,
			 in float3 binor : TEXCOORD6,
             out float4 oCol: COLOR0)
{   
   // be sure to normalize the vectors

   
   float3 norD = bumpy*(2.0f*tex2D(NormalMapSampler, vTex) - 1.0f);
   float3 N = normalize(nor);
   float3 T = normalize(tan);
   float3 B = normalize(binor);
   N += (norD.x * T + norD.y * B);
   N = normalize(N);
   
   //float3 N = (2.0f*tex2D(NormalMapSampler, vTex) - 1.0f);
   
   float3 L = normalize(vLgt);
   float3 V = normalize(vCam);
   float3 H = normalize(V + L);
   float3 skyDir = normalize(vSky);
   
   // N dot L 
   float diff = saturate(dot(N, L));
 
   // (N dot H)^n
   float spec = pow(saturate(dot(H, N)), power);
    
   //天光地光(環境光)
   float4 ambLgt = lerp( groundColor, skyColor, dot(N, skyDir)*0.5 + 0.5)*amb;

   //Color Map
   float3 colorDiffuse = tex2D(ColorMapSampler, vTex);
   
   //Specular Map
   float3 colorSpecular= tex2D(SpecularMapSampler, vTex);
   
 
   oCol.rgb=colorDiffuse*2.0f*(ambLgt*amb + mainLightColor*(diff*dif + colorSpecular*spec*spe)); 
   
   // the opacity
   oCol.a = dif.a;
}


// the associated D3D Effect technique
technique Basic_Color_Normal_Power
{
   pass P0
   {
      //AlphaBlendEnable = TRUE;
      
      //SrcBlend = SRCALPHA;
      //DestBlend = INVSRCALPHA;
      
      VertexShader = compile vs_2_0 PhongVS();
      PixelShader = compile ps_2_0 PhongPS();      
   }
}