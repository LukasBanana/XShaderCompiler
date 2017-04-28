[numthreads(8, 8, 1)]
void main()
{
    GroupMemoryBarrier();
    GroupMemoryBarrierWithGroupSync();
    DeviceMemoryBarrier();
    DeviceMemoryBarrierWithGroupSync();
} 