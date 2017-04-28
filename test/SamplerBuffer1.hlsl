Buffer<float4> boneMatrices[2][3];

float3x4 getBoneMatrix(uint idx)
{
    float4 row0 = boneMatrices[1][2][idx * 3 + 0];
    float4 row1 = boneMatrices[1][2][idx * 3 + 1];
    float4 row2 = boneMatrices[1][2][idx * 3 + 2];
    
    return float3x4(row0, row1, row2);
}

float4 main() : SV_Position
{
	return mul(getBoneMatrix(0), float4(1, 2, 3, 4));
}