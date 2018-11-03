#define WORKGROUP_SIZE 256

__kernel void DotProduct(__global int *src1, __global int *src2, __global int *dst, int len) {
  int gid = get_global_id(0);
  __local int buffer[WORKGROUP_SIZE];

  // ��ԭ�ӷ�ʽ�� dst ��ַ���������㣨ִֻ��һ�Σ�ʹ��ȫ�ֹ������ʶ��
  if (gid == 0)
    atomic_xchg(dst, 0);
  if (gid < WORKGROUP_SIZE)
    buffer[gid] = 0;

  // ��ȡ�������е�ÿ��������
  int lid = get_local_id(0);

  // ��仺����
  buffer[lid] += src1[gid] * src2[gid];

  // ���еĹ�����ִ�е�����ȴ��Ծֲ��������ķ������
  barrier(CLK_LOCAL_MEM_FENCE);

  // ֻ���ڵ�һ��������ִ�е�ʱ���ȡ�������������ӵ� dst ָ���λ��
  if (lid == 0) {
    int sum = 0;
    for (int i = 0; i < WORKGROUP_SIZE; i++)
      sum += buffer[i];
    atomic_add(dst, sum);
  }
}