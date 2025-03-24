#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include <vector>
#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class Image {
public:
    int width, height, channels;
    unsigned char* data;

    Image(const char* filename) {
        data = stbi_load(filename, &width, &height, &channels, 0);
    }

    Image(int width, int height, int channels, unsigned char* data) {
        this->width = width;
        this->height = height;
        this->channels = channels;
        this->data = data;
    }

    void save(const char* filename) {
        stbi_write_png(filename, width, height, channels, data, width * channels);
    }

    ~Image() {
        stbi_image_free(data);
    }
};

float** Image_to_Matrix_Grayscale(Image image) {
    float** matrix = new float*[image.height];
    for (int i = 0; i < image.height; i++) {
        matrix[i] = new float[image.width];
    }
    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {
            if (image.channels >= 3) {
                int idx = (i * image.width + j) * image.channels;
                matrix[i][j] = 0.299f * image.data[idx] + 0.587f * image.data[idx + 1] + 0.114f * image.data[idx + 2];
            } else {
                matrix[i][j] = (float)image.data[i * image.width + j];
            }
        }
    }
    return matrix;
}

void deleteMatrix(float** matrix, int height) {
    for (int i = 0; i < height; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;
}

int main() {
    const int M = 512;
    float kernel_data[] = {1, 0, -1, 1, 0, -1, 1, 0, -1};

    Image input_image("cat.jpg");
    if (!input_image.data || input_image.width != M || input_image.height != M || input_image.channels < 3) {
        std::cout << "Failed to load " << M << "x" << M << " RGB image" << std::endl;
        return 1;
    }

    float** grayscale_matrix = Image_to_Matrix_Grayscale(input_image);
    std::vector<float> input_matrix(M * M);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            input_matrix[i * M + j] = grayscale_matrix[i][j];
        }
    }
    deleteMatrix(grayscale_matrix, M);

    std::vector<float> output_matrix(M * M);

    cl_int err;
    cl_uint num_platforms;
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    if (err != CL_SUCCESS || num_platforms == 0) {
        std::cerr << "No platforms found: " << err << std::endl;
        return 1;
    }
    std::cout << "Number of platforms: " << num_platforms << std::endl;

    std::vector<cl_platform_id> platforms(num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms.data(), NULL);

    cl_platform_id platform = platforms[0];
    char platform_name[128];
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
    std::cout << "Platform selected: " << platform_name << std::endl;

    cl_uint num_devices;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    if (err != CL_SUCCESS || num_devices == 0) {
        std::cerr << "No devices found: " << err << std::endl;
        return 1;
    }
    std::cout << "Number of devices: " << num_devices << std::endl;

    std::vector<cl_device_id> devices(num_devices);
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devices.data(), NULL);
    cl_device_id device = devices[0];
    char device_name[128];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    std::cout << "Device selected: " << device_name << std::endl;

    char device_version[128];
    clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(device_version), device_version, NULL);
    std::cout << "OpenCL Version: " << device_version << std::endl;

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (!context) {
        std::cerr << "Context creation failed: " << err << std::endl;
        return 1;
    }

    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    if (!queue) {
        std::cerr << "Queue creation failed: " << err << std::endl;
        return 1;
    }

    std::ifstream file("kernel.cl");
    if (!file.is_open()) {
        std::cerr << "Failed to open kernel.cl" << std::endl;
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    const char* source_str = source.c_str();
    size_t source_size = source.length();
    cl_program program = clCreateProgramWithSource(context, 1, &source_str, &source_size, &err);
    if (!program) {
        std::cerr << "Program creation failed: " << err << std::endl;
        return 1;
    }

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log.data(), NULL);
        std::cerr << "Build error: " << log.data() << std::endl;
        return 1;
    }

    cl_mem input_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, M * M * sizeof(float), input_matrix.data(), &err);
    cl_mem output_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, M * M * sizeof(float), NULL, &err);
    cl_mem kernel_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 9 * sizeof(float), kernel_data, &err);
    if (!input_buffer || !output_buffer || !kernel_buffer) {
        std::cerr << "Buffer creation failed: " << err << std::endl;
        return 1;
    }

    cl_kernel kernel = clCreateKernel(program, "convolution", &err);
    if (!kernel) {
        std::cerr << "Kernel creation failed: " << err << std::endl;
        return 1;
    }

    
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buffer);
    
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buffer);
     clSetKernelArg(kernel, 2, sizeof(int), &M);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &kernel_buffer);
    

    size_t global_size[2] = {static_cast<size_t>(M), static_cast<size_t>(M)};
    size_t local_size[2] = {16, 16};
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, local_size, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cerr << "Kernel execution failed: " << err << std::endl;
        return 1;
    }

    err = clEnqueueReadBuffer(queue, output_buffer, CL_TRUE, 0, M * M * sizeof(float), output_matrix.data(), 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cerr << "Read buffer failed: " << err << std::endl;
        return 1;
    }

    //Normalize the output matrix
    float min_val = output_matrix[0], max_val = output_matrix[0];
    for (int i = 0; i < M * M; i++) {
        if (output_matrix[i] < min_val) min_val = output_matrix[i];
        if (output_matrix[i] > max_val) max_val = output_matrix[i];
    }

    unsigned char* output_data = new unsigned char[M * M];
    for (int i = 0; i < M * M; i++) {
        float val = (output_matrix[i] - min_val) * 255.0f / (max_val - min_val);
        output_data[i] = static_cast<unsigned char>(val < 0 ? 0 : (val > 255 ? 255 : val));
    }

    Image output_image(M, M, 1, output_data);
    output_image.save("output.png");
    delete[] output_data;

    std::cout << "Edge detection completed, output saved as output.png" << std::endl;

    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    clReleaseMemObject(kernel_buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}