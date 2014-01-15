//Swing shader by Digger 2009/09/01

float BW;//blur weight
float tu;
float tv;

texture premap;
sampler2D PSampler = sampler_state
{
    texture = <premap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

texture curmap;
sampler2D CSampler = sampler_state
{
    texture = <curmap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

struct VS_OUTPUT
{
   float4 pos : POSITION;
   float2 tex : TEXCOORD0;
};

VS_OUTPUT RenderTarget_vs(float4 inPos : POSITION,
						  float2 tex   : TEXCOORD0)
{
   VS_OUTPUT out1 = (VS_OUTPUT) 0;
   out1.pos.xy = inPos.xy + float2(tu,tv);
   out1.pos.z = 1.0f;
   out1.pos.w = 1.0f;
   
   //out1.tex = tex;
   out1.tex = float2(0.5f, -0.5f) * inPos.xy + float2(0.5f, 0.5f);

   return out1;
}

float4 RenderTarget_ps(float2 tex : TEXCOORD0) : COLOR
{
	float4 curpicture = tex2D(CSampler,tex);
	float4 prepicture = tex2D(PSampler, tex);
	
	//a + s(b-a);
    //a = curpicture, b = prepicture, s = BW
	float4 re = lerp(curpicture,prepicture,BW);
	
	return re;
}

technique Swing
{
   pass p0
   {
      VertexShader = compile vs_2_0 RenderTarget_vs();
      PixelShader  = compile ps_2_0 RenderTarget_ps();      
   }
}