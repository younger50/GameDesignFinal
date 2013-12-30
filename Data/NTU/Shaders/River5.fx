//River Shader by Digger 2009/10/07

//constants
float4x4 WVP;
float4x4 mW;
float4x4 mVP;//light camera's project matrix
float4 CP;
float4 LP;

//parameter
float  OW = 1.0f;//offset weight
float  BW = 1.0f;//blur weight
float  Timer;//translate uv
float  BSX = -0.04f;//normal map speed X


texture pMap;
sampler PSampler = sampler_state
{
   Texture = <pMap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;   
   AddressU = clamp;
   AddressV = clamp;
   AddressW = clamp;       
};

texture normalmap;
sampler2D NSampler = sampler_state
{
   Texture = <normalmap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;
};

texture reflectmap;
sampler2D RSampler = sampler_state
{
   Texture = <reflectmap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU  = Clamp;
   AddressV  = Clamp;
   //AddressW  = Clamp;   
};

/*-----------------------------*/
//PM Distortion
/*----------------------------*/
struct VS_OUTPUTFG
{
   float4 pos   : POSITION;
   float4 tex   : TEXCOORD0;
   float3 cam   : TEXCOORD1;
   float2 coTex : TEXCOORD2;
   float3 N     : TEXCOORD3;
   float3 tV    : TEXCOORD4;
   float3 lgt   : TEXCOORD5;
};

VS_OUTPUTFG RS_vs(float4 pos  : POSITION,
				  float2 tex  : TEXCOORD0,
				  float3 nor  : NORMAL,
				  float3 tan  : TANGENT) 
{
	VS_OUTPUTFG out1 = (VS_OUTPUTFG) 0;
	
	//vertex to projection space
	out1.pos = mul(pos, WVP);

	//texture coordinate
	out1.coTex = tex;

    //view vector
    float4 a = mul(pos, mW);
    out1.cam = normalize(CP.xyz - a.xyz);
    out1.lgt = normalize(LP.xyz - a.xyz);

	//local space
    out1.tex = pos;
    
    //normal in world space
    out1.N = mul(nor, (float3x3)mW);
    
    //tangent matrix
    float3x3 mTT;
    mTT[0] = mul(tan, (float3x3)mW);
    mTT[1] = mul(cross(tan,nor), (float3x3)mW);
    mTT[2] = out1.N;
    
    out1.tV = normalize(mul(mTT, out1.cam));
	
	return out1;
}

float4 RS_ps(float4 tex   : TEXCOORD0,
			 float3 cam   : TEXCOORD1,
			 float2 coTex : TEXCOORD2,
			 float3 vN    : TEXCOORD3,
			 float3 tV    : TEXCOORD4,
			 float3 lgt   : TEXCOORD5) : COLOR
{
	float4 color = 0;

	/*------------------*/
	//get normal map data
	/*------------------*/
	float3 V = normalize(cam);
	float3 L = normalize(lgt);
	
	//tanslate coTex
	float  cycle = fmod(Timer, 100.0f);
	float2 vTex = coTex;
	vTex.x += (cycle * BSX);//uv's u
	
	float3 N = 2.0f * tex2D(NSampler, vTex) - 1.0f;
	
	//local to projection space
	float4 p = mul(tex, WVP);
	
	//texture coordinate
	float2 uv = (p.xy / p.w) * float2(0.5f, -0.5f) + 0.5f;
	
	float2 reUV = uv;
	
	//Distortion uv
	float2 offset = V.xy - N.xy;

	/*------------------
		CubeMap
	------------------*/
	reUV.x = 1.0f - reUV.x;
	
	reUV += offset*0.02f;//0.04 = distortion value
	
	//noise
	float x = coTex.x * coTex.y * 123456 *Timer;
	x = fmod(x,13) * fmod(x, 123);
	float dx = fmod(x, 0.01f);
	float dy = fmod(x, 0.012f);
	float4 RColor = tex2D(RSampler, reUV +float2(dx,dy)*0.4f);//0.4 = blur weight
		
	return RColor;
}


technique RS
{
	pass p0
	{ 
      VertexShader = compile vs_3_0 RS_vs();
      PixelShader  = compile ps_3_0 RS_ps();
	}	
}