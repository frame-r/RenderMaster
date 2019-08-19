

cbuffer Frame : register(b0)
{
	uint tex_in_width;
	uint tex_in_height;
};

Texture2D tex_in : register(t0);
RWTexture2D<float4> tex_out : register(u0);


//
// Mipmpap generation
// Rounding down rule:
// Example: 15 -> 7 -> 3 -> 1


#define GROUP_DIM_X 16
#define GROUP_DIM_Y 16
[numthreads(GROUP_DIM_X, GROUP_DIM_Y, 1)]
void mainCS(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	float3 color = float3(0, 0, 0);
	int2 idx = dispatchThreadId.xy * 2;
	float Width = float(tex_in_width / 2u);
	float Height = float(tex_in_height / 2u);
	float x = idx.x;
	float y = idx.y;

	if (dispatchThreadId.x > Width || dispatchThreadId.y > Height)
		return;

#ifdef X_EVEN && Y_EVEN

	color = 0.25f * (
		tex_in.Load(int3(idx.x + 0, idx.y + 0, 0)).rgb + 
		tex_in.Load(int3(idx.x + 1, idx.y + 0, 0)).rgb +
		tex_in.Load(int3(idx.x + 0, idx.y + 1, 0)).rgb +
		tex_in.Load(int3(idx.x + 1, idx.y + 1, 0)).rgb
		);
#elif !X_EVEN && !Y_EVEN
	
	float3 colors[3];

	float w0 = (float)(Width - x) / (2 * Width + 1);
	float w1 = (float)Width / (2 * Width + 1);
	float w2 = (float)(1 + x) / (2 * Width + 1);

	for (int i = 0; i < 3; i++)
	{
		colors[i] =
			w0 * tex_in.Load(int3(idx.x + 0, idx.y + i, 0)).rgb +
			w1 * tex_in.Load(int3(idx.x + 1, idx.y + i, 0)).rgb +
			w2 * tex_in.Load(int3(idx.x + 2, idx.y + i, 0)).rgb;
	}

	float h0 = (float)(Height - y) / (2 * Height + 1);
	float h1 = (float) Height / (2 * Height + 1);
	float h2 = (float)(1 + y) / (2 * Height + 1);

	color =
		h0 * colors[0] +
		h1 * colors[1] +
		h2 * colors[2];

#elif X_EVEN && !Y_EVEN
	float3 colors[3];

	for (int i = 0; i < 3; i++)
	{
		colors[i] =
			0.5 * tex_in.Load(int3(idx.x + 0, idx.y + i, 0)).rgb +
			0.5 * tex_in.Load(int3(idx.x + 1, idx.y + i, 0)).rgb;
	}

	float h0 = (float)(Height - y) / (2 * Height + 1);
	float h1 = (float)Height / (2 * Height + 1);
	float h2 = (float)(1 + y) / (2 * Height + 1);

	color =
		h0 * colors[0] +
		h1 * colors[1] +
		h2 * colors[2];

#else
	float3 colors[2];

	float w0 = (float)(Width - x) / (2 * Width + 1);
	float w1 = (float)Width / (2 * Width + 1);
	float w2 = (float)(1 + x) / (2 * Width + 1);

	for (int i = 0; i < 2; i++)
	{
		colors[i] =
			w0 * tex_in.Load(int3(idx.x + 0, idx.y + i, 0)).rgb +
			w1 * tex_in.Load(int3(idx.x + 1, idx.y + i, 0)).rgb +
			w2 * tex_in.Load(int3(idx.x + 2, idx.y + i, 0)).rgb;
	}

	color =
		0.5 * colors[0] +
		0.5 * colors[1];

#endif

	tex_out[dispatchThreadId.xy] = float4(color.rgb, 1);
}

