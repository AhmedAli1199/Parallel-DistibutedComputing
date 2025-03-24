__kernel void convolution(__global const float* input, __global float* output, const int M, __constant float* mykernel) {
    int gx = get_global_id(0);  // Column
    int gy = get_global_id(1);  // Row

    if (gx < M && gy < M) {
        float sum = 0.0f;
        for (int ki = -1; ki <= 1; ki++) {
            for (int kj = -1; kj <= 1; kj++) {
                int img_x = gx + kj;
                int img_y = gy + ki;
                float pixel = (img_x >= 0 && img_x < M && img_y >= 0 && img_y < M)
                            ? input[img_y * M + img_x] : 0.0f;
                sum += pixel * mykernel[(ki + 1) * 3 + (kj + 1)];
            }
        }
        output[gy * M + gx] = sum;
    }
}