//Projective Mapping Distortion by Digger 2009/09/24

float4x4 WVP;
float4x4 mW;
float4x4 mCP;//main camera's project matrix
float4 CP;

//material
float  OW = 3.0f;//offset weight
float  BW = 1.0f;//blur weight

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

texture cmap;
sampler2D CSampler = sampler_state
{
   Texture = <cmap>;
   MinFilter = Linear;
   MagFilter = Linear;
   MipFilter = Linear;
   AddressU = Wrap;
   AddressV = Wrap;
};

/*-----------------------------*/
//PM Distortion
/*----------------------------*/
struct VS_OUTPUTFG
{
   float4 pos  : POSITION;
   float4 tex  : TEXCOORD0;
   float3 cam  : TEXCOORD1;
   float2 coTex : TEXCOORD2;
};

VS_OUTPUTFG FrostedGlass_vs(float4 pos  : POSITION,
							float2 tex  : TEXCOORD0,
						    float3 tan  : TANGENT,
						    float3 norm : NORMAL) 
{
	VS_OUTPUTFG out1 = (VS_OUTPUTFG) 0;
	
	//vertex to projection space
	out1.pos = mul(pos, WVP);

	//texture coordinate
	out1.coTex = tex;

    //tangent space matrix
    float3x3 wTT;
    wTT[0] = mul(tan, mW);
    wTT[1] = mul(cross(tan, norm), mW);
    wTT[2] = mul(norm, mW);
    
    //vertex in world space
	float4 a = mul(pos, mW);
	
    //view vector
    out1.cam = normalize(mul(wTT, CP.xyz - a.xyz));


    /*---------------------------*/  
    //projective map
    /*---------------------------*/ 
    float4 tex2 = mul(pos, mW);
    out1.tex = mul(tex2, mCP);//project texture corrdinate,use main camera's matrix so no need vertex's uv 
  
    //use camera's aspect,here is 350 / 123
    out1.tex.x = out1.tex.x * 0.5f + out1.tex.w * (0.5f + 0.5f / 350.0f);
    out1.tex.y = -out1.tex.y * 0.5f + out1.tex.w * (0.5f + 0.5f / 123.0f);
    out1.tex.z = 0;
 
	return out1;
}

float4 FrostedGlass_ps(float4 tex  : TEXCOORD0,
					   float3 cam  : TEXCOORD1,
					   float2 coTex : TEXCOORD2) : COLOR
{
	float4 color = 0;

	/*------------------*/
	//get normal map data
	/*------------------*/
	float3 V = normalize(cam);
	float3 N = 2.0f * tex2D(NSampler, coTex) - 1.0f;
	
	//Distortion
	//uv offset
	//float4 tex2 = tex;
	//tex2.xy += (V.xy - N.xy) * OW *10.0f;
	float2 offset = V.xy - N.xy;
	tex.xy += offset*OW*10.0f;
	
	return tex2Dproj(PSampler, tex)+ tex2Dproj(CSampler, tex)*1.2f;
}


technique PMD
{
	pass p0
	{ 
	  CullMode = none;
	  ZwriteEnable = true;
      VertexShader = compile vs_3_0 FrostedGlass_vs();
      PixelShader  = compile ps_3_0 FrostedGlass_ps();
	}	
}