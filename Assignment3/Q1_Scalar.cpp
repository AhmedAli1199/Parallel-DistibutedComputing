#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
using namespace std;

class Image{

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

float** Image_to_Matrix_Grayscale(Image image)
{
    float **matrix;
    matrix = new float*[image.height];
    for(int i = 0; i < image.height; i++)
    {
        matrix[i] = new float[image.width];
    }
    for(int i = 0; i < image.height; i++)
    {
        for(int j = 0; j < image.width; j++)
        {
            if(image.channels >= 3) {
                int idx = (i * image.width + j) * image.channels;
                // Standard grayscale conversion formula: 0.299R + 0.587G + 0.114B
                matrix[i][j] = 0.299f * image.data[idx] + 0.587f * image.data[idx+1] + 0.114f * image.data[idx+2];
            } else {
                // For already grayscale images
                matrix[i][j] = (float)image.data[i * image.width + j];
            }
        }
        
    }
    
    
    return matrix;
    

}

void convolve(float **matrix,float kernel[3][3], float **output,int width, int height)
{
    for (int i =0;i<height;i++)
    {
        for(int j=0;j<width;j++)
        {
            int sum = 0;
            for (int ki=0;ki<3;ki++)
            {
                for(int kj=0;kj<3;kj++)
                {   //Taking center of kernel as 1,1
                    int drow = ki - 1;
                    int dcol = kj - 1;
                    int newi = i + drow;
                    int newj = j + dcol;
                    if(newj>=0 && newj<width && newi>=0 && newi<height) // Zero padded boundary condition
                    {
                        sum += matrix[newi][newj]*kernel[ki][kj];
                    }

                }

            }

            output[i][j] = sum;
            cout<<output[i][j]<<" ";
        }
        cout<<endl;
    }

}

unsigned char* matrix_to_data(float **matrix, int width, int height)
{
    unsigned char* data = new unsigned char[width * height];
    
   // Find min and max values for normalization
   float min_val = matrix[0][0];
   float max_val = matrix[0][0];
   
   for(int i = 0; i < height; i++) {
       for(int j = 0; j < width; j++) {
           if(matrix[i][j] < min_val) min_val = matrix[i][j];
           if(matrix[i][j] > max_val) max_val = matrix[i][j];
       }
   }
   
   // Calculate normalization range
   float range = max_val - min_val;
   if(range == 0) range = 1; // Avoid division by zero
   
   // Normalize and convert to unsigned char
   for(int i = 0; i < height; i++) {
       for(int j = 0; j < width; j++) {
           // Normalize to 0-255 range
           float normalized = 255.0f * (matrix[i][j] - min_val) / range;
           data[i * width + j] = (unsigned char)normalized;
       }
   }
   
    
    return data;
}

int main() {
    Image img("cat.jpg");
    cout<<img.width<<" "<<img.height<<" "<<img.channels<<endl;
    float** matrix = Image_to_Matrix_Grayscale(img);
    
    float kernel[3][3] = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};
    float** output;
    output = new float*[img.height];
    for(int i = 0; i < img.height; i++)
    {
        output[i] = new float[img.width];
    }

    convolve(matrix,kernel,output,img.width,img.height);

    unsigned char* output_data = matrix_to_data(output, img.width, img.height);

    Image output_img(img.width,img.height,1,(unsigned char*)output_data);
    output_img.save("output.png");


    return 0;
}