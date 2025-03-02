#pragma once
#include <iostream>
#include "../lib/stb_image.h"

// Sequential CPU implementation of JPEG compression
inline void jpegCPU(unsigned char* pixels, int width, int height) {
    // TODO: handle padding
    vector<vector<float>> Y(height, vector<float>(width));
    vector<vector<float>> Cb(height, vector<float>(width));
    vector<vector<float>> Cr(height, vector<float>(width));
    
    for(int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            unsigned char* index = pixels + (i * width + j) * 3; 
            rgbToYCbCr(*(index), *(index + 1), *(index + 2), Y[i][j], Cb[i][j], Cr[i][j]);
        }
    }
}

// converts one pixel's r, g, b values into luminance (y), blue chrominance (cb), and red chrominance (cr)
inline void rgbToYCbCr(unsigned char r, unsigned char g, unsigned char b, float& y, float& cb, float& cr) {
    y = 0.299f * r + 0.587f * g + 0.114f * b;
    cb = -0.1687f * r - 0.3313f * g + 0.5f * b + 128;
    cr = 0.5f * r - 0.4187f * g - 0.0813f * b + 128;
}

// reduces resolution of the Cr and Cb vectors using 4:2:0 chroma subsampling
inline void chromaSubsampling(vector<vector<float>>& Cb, vector<vector<float>>& Cr, int width, int height) {
    vector<vector<float>> cbSub(height/2, vector<float>(width/2));
    vector<vector<float>> crSub(height/2, vector<float>(width/2));
    for(int i = 0, r = 0; i < height - 1; i += 2) {
        for(int j = 0, c = 0; j < width - 1; j += 2) {
            cbSub[r][c] = (Cb[i][j] + Cb[i][j+1] + Cb[i+1][j] + Cb[i+1][j+1])/4; // averages over 2x2 grid of pixels
            crSub[r][c] = (Cr[i][j] + Cr[i][j+1] + Cr[i+1][j] + Cr[i+1][j+1])/4;
            c++;
        }
        r++;
    }
    Cb = cbSub;
    Cr = crSub;
}

inline void blocking8x8(vector<vector<float>>& Cb, vector<vector<float>>& Cr, int width, int height) {
    
}





