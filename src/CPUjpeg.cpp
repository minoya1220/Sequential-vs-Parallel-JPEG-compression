#pragma once
#include <iostream>
#include "../lib/stb_image.h"

constexpr float cos_pi_8, sin_3pi_8 = 0.923879532511f;
constexpr float cos_3pi_8, sin_pi_8 = 0.382683432365f;
constexpr float cos_pi_4, sin_pi_4 = 0.707106781187f;

constexpr float cos_pi_16 = 0.980785280403f;
constexpr float sin_pi_16 = 0.195090322016f;
constexpr float cos_3pi_16 = 0.831469612303f;
constexpr float sin_3pi_16 = 0.55557023302f;

// Luminance (Y) quantization table
const int JPEG_LUMA_QUANT_TABLE[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}
};

// Chrominance (CbCr) quantization table
const int JPEG_CHROMA_QUANT_TABLE[8][8] = {
    {17, 18, 24, 47, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99}
};

union BlockVals {
    float valsF[8][8];
    int valsI[8][8];
};

// defines an 8x8 block of pixels and the transformation operations necessary to implement JPEG compression
struct Block {
    float vals[8][8];
    

    bool luma;
    Block(vector<vector<float>>& input, int rowStart, int colStart, int width, int height, bool luma) {
        this->luma = luma;
        if (rowStart < 0 || colStart < 0 || rowStart >= width || colStart >= height) {
            printf("Index out of bounds error\n");
        }
        if (width < 8 || height < 8) { // default padding for images with a width or height less than 8
            for(int i = 0; i < 8; i++) {
                for(int j = 0; j < 8; j++) {
                    int i_new = (rowStart + i >= height) ? height - 1 : rowStart + i;
                    int j_new = (colStart + j >= width) ? width - 1 : colStart + j;
                    vals[i][j] = input[i_new][j_new];
                }
            }
        }
        else {  // reflection padding for large images, prevents edge artifacts
            for(int i = 0; i < 8; i++) {
                for(int j = 0; j < 8 && colStart + j < width; j++) {
                    int j_new = (colStart + j >= width) ? 2 * width - colStart - j - 2 : colStart + j; 
                    int i_new = (rowStart + i >= height) ? 2 * height - rowStart - i - 2 : rowStart + i;
                    vals[i][j] = input[i_new][j_new];
                }
            }
        }
    }   

    // Performs the discrete cosine transform on the current block using loeffler method on both dimensions
    void blockDCT() {
        // runs loeffler 1D DCT on all rows of the block
        for (int i = 0; i < 8; i++) {
            loeffler1DDCT(vals[i]);
        }

        // runs loeffler 1D DCT on all columns of the block
        for (int i = 0; i < 8; i++) {
            float temp[8];
            for (int j = 0; j < 8; j++) {
                temp[j] = vals[j][i];
            }
            loeffler1DDCT(temp);
            for (int j = 0; j < 8; j++) {
                vals[j][i] = temp[j];
            }
        }
    }

    // Implements a 1D loeffler DCT on an array of size 8 (replaces input with output at the end)
    void loeffler1DDCT(float* input) {
        float butterfly[8];
        // populates the first 4 elements with symmetrical paired sums and last 4 with paired diffs
        for(int i = 0; i < 4; i++) {
            butterfly[i] = input[i] + input[7-i]; 
            butterfly[7-i] = input[i] - input[7-i];
        }

        
        float frequencies[8];

        // handles even frequencies by applying another butterfly to [0-3]
        // no loop necessary for 2 iterations
        frequencies[0] = butterfly[0] + butterfly[3];
        frequencies[3] = butterfly[0] - butterfly[3];
        
        frequencies[1] = butterfly[1] + butterfly[2];
        frequencies[2] = butterfly[1] - butterfly[2];

        // rotates in the frequency domain using derived angles from the DCT
        rotate(&frequencies[1], &frequencies[7], sin_pi_16, cos_pi_16);
        rotate(&frequencies[3], &frequencies[5], sin_3pi_16, cos_3pi_16);
        rotate(&frequencies[2], &frequencies[6], sin_pi_8, cos_pi_8);
        frequencies[4] = frequencies[4] * cos_pi_4; // scaled by 1/sqrt(2) for "self rotation"
        // no rotation for frequency 0

        // rearrange the frequencies into the correct order
        input[0] = frequencies[0];
        input[4] = frequencies[1];
        input[2] = frequencies[2];
        input[6] = frequencies[3];
        input[1] = frequencies[4];
        input[5] = frequencies[5];
        input[3] = frequencies[6];
        input[7] = frequencies[7];
    }

    // rotates (a,b) using a precomputed sin(theta) and cos(theta)
    void rotate(float& a, float& b, float sin, float cos) {
        float a_temp = cos*a - sin*b;
        b = sin*a + cos*b;
        a = a_temp;
    }

    
    // performs quantization on the current block using a predefined quantization matrix
    void quantize() {
        for(int i = 0; i < 8; i++) {
            for(int j = 0; j < 8; j++) {
                if (luma) {
                    vals[i][j] = vals[i][j] / JPEG_LUMA_QUANT_TABLE[i][j];
                } else {
                    vals[i][j] = vals[i][j] / JPEG_CHROMA_QUANT_TABLE[i][j];
                }
            }
        }
    }

    // performs run length encoding on the current block
    void runLengthEncode() {
        
    }

    // performs huffman encoding on the current block
    void huffmanEncode() {

    }
    
}



// converts one pixel's r, g, b values into luminance (y), blue chrominance (cb), and red chrominance (cr)
inline void rgbToYCbCr(unsigned char r, unsigned char g, unsigned char b, float& y, float& cb, float& cr) {
    y = 0.299f * r + 0.587f * g + 0.114f * b;
    cb = -0.1687f * r - 0.3313f * g + 0.5f * b + 128;
    cr = 0.5f * r - 0.4187f * g - 0.0813f * b + 128;
}

// Applies color space conversion across a full image
inline void fullImageToYCbCr(unsigned char* pixels, int width, int height, vector<vector<float>>& Y, vector<vector<float>>& Cb, vector<vector<float>>& Cr) {
    for(int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            unsigned char* index = pixels + (i * width + j) * 3; 
            rgbToYCbCr(*(index), *(index + 1), *(index + 2), Y[i][j], Cb[i][j], Cr[i][j]);
        }
    }
}

// reduces resolution of the Cr and Cb vectors using 4:2:0 chroma subsampling
inline void chromaSubsampling(vector<vector<float>>& Cb, vector<vector<float>>& Cr, int width, int height) {
    vector<vector<float>> cbSub(height/2, vector<float>(width/2));
    vector<vector<float>> crSub(height/2, vector<float>(width/2));
    for(int i = 0, r = 0; i < height - 1; i += 2, r++) {
        for(int j = 0, c = 0; j < width - 1; j += 2, c++) {
            cbSub[r][c] = (Cb[i][j] + Cb[i][j+1] + Cb[i+1][j] + Cb[i+1][j+1])/4.0f; // averages over 2x2 grid of pixels
            crSub[r][c] = (Cr[i][j] + Cr[i][j+1] + Cr[i+1][j] + Cr[i+1][j+1])/4.0f;
        }
    }
    Cb = cbSub;
    Cr = crSub;
}

// separates the input vectors into 8x8 blocks
inline void blocking8x8(vector<vector<float>>& Y, vector<vector<float>>& Cb, vector<vector<float>>& Cr, vector<vector<Block>>& YBlocks, vector<vector<Block>>& CbBlocks, vector<vector<Block>>& CrBlocks, int width, int height) {
    // because of added padding width and height will be divisible by 8
    for(int i = 0; i < width/8; i++) { 
        for(int j = 0; j < height/8; j++) {
            YBlocks[i][j] = new Block(Y, i*8, j*8, width, height, true);
            if (j % 2 == 0 && i % 2 == 0) {
                CbBlocks[i/2][j/2] = new Block(Cb, i*4, j*4, width, height, false); // *4 seems counter-intuitive but its because our index is already doubled
                CrBlocks[i/2][j/2] = new Block(Cr, i*4, j*4, width, height, false);
            }
        }
    }
}

// Sequential CPU implementation of JPEG compression
inline void jpegCPU(unsigned char* pixels, int width, int height) {
    vector<vector<float>> Y(height, vector<float>(width));
    vector<vector<float>> Cb(height, vector<float>(width));
    vector<vector<float>> Cr(height, vector<float>(width));
    

}





