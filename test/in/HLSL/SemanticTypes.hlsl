
// Semantic Types Test
// 30/05/2017

void main(
	uint vertID : SV_VertexID,
	uint instID : SV_InstanceID)
{
	++vertID;
	
	uint vertexID = vertID;
	uint instanceID = instID;
	
	uint i;
	
	i = vertID + instID;
	
	bool b0 = (vertID == 0u);
	bool b1 = (0u == vertID);
	bool b2 = (instID == 0u);
	bool b3 = (0u == instID);
}

