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

int main() {
    Image img("cat.jpg");
    cout<<img.width<<" "<<img.height<<" "<<img.channels<<endl;
    img.save("output.png");
    return 0;
}