#include <iostream>
#include "../lib/stb_image.h"
#include "CPUjpeg.h"

int main() {
    std::cout << "JPEG Compressor Starting..." << std::endl;
    int width, height, channels;

    unsigned char* pixels = sbti_load("testimg.png", &width, &height, &channels, 3);
    std::vector<unsigned char> pixels(raw_pixels, raw_pixels + width * height * 3)

    return 0;
}