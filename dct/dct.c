#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jpeglib.h>

#define BLOCK_SIZE 8
#define MSG_TERMINATOR '\0'
#define QUANT_FACTOR 50  // Quality factor for JPEG

typedef struct {
    unsigned char *data;
    int width;
    int height;
    int channels;
} Image;

// DCT matrix
double dct_matrix[BLOCK_SIZE][BLOCK_SIZE];

// Initialize DCT matrix
void init_dct_matrix() {
    int i, j;
    double scale;
    
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            if (i == 0)
                scale = sqrt(1.0 / BLOCK_SIZE);
            else
                scale = sqrt(2.0 / BLOCK_SIZE);
            
            dct_matrix[i][j] = scale * cos((2 * j + 1) * i * M_PI / (2 * BLOCK_SIZE));
        }
    }
}

// Apply DCT to 8x8 block
void apply_dct(double block[BLOCK_SIZE][BLOCK_SIZE], double dct[BLOCK_SIZE][BLOCK_SIZE]) {
    double temp[BLOCK_SIZE][BLOCK_SIZE];
    int i, j, k;
    
    // Matrix multiplication with DCT matrix
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            temp[i][j] = 0;
            for (k = 0; k < BLOCK_SIZE; k++) {
                temp[i][j] += dct_matrix[i][k] * block[k][j];
            }
        }
    }
    
    // Second matrix multiplication
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            dct[i][j] = 0;
            for (k = 0; k < BLOCK_SIZE; k++) {
                dct[i][j] += temp[i][k] * dct_matrix[j][k];
            }
        }
    }
}

// Embed message in DCT coefficients
void embed_message(double dct[BLOCK_SIZE][BLOCK_SIZE], char bit) {
    // Modify middle frequency coefficients
    // Using (4,1) coefficient for embedding
    if (bit == '1') {
        dct[4][1] = floor(dct[4][1] / QUANT_FACTOR) * QUANT_FACTOR + QUANT_FACTOR * 0.75;
    } else {
        dct[4][1] = floor(dct[4][1] / QUANT_FACTOR) * QUANT_FACTOR + QUANT_FACTOR * 0.25;
    }
}

// Extract message from DCT coefficients
char extract_bit(double dct[BLOCK_SIZE][BLOCK_SIZE]) {
    double coef = dct[4][1];
    double mod = fmod(coef, QUANT_FACTOR);
    return (mod > QUANT_FACTOR/2) ? '1' : '0';
}

// Main steganography functions
void encode_message(const char *input_path, const char *output_path, const char *message) {
    struct jpeg_decompress_struct cinfo_in;
    struct jpeg_compress_struct cinfo_out;
    struct jpeg_error_mgr jerr;
    FILE *infile, *outfile;
    
    // Initialize JPEG decompression
    cinfo_in.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo_in);
    
    if ((infile = fopen(input_path, "rb")) == NULL) {
        fprintf(stderr, "Cannot open input file\n");
        return;
    }
    
    jpeg_stdio_src(&cinfo_in, infile);
    jpeg_read_header(&cinfo_in, TRUE);
    jpeg_start_decompress(&cinfo_in);
    
    // Initialize output JPEG
    cinfo_out.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo_out);
    
    if ((outfile = fopen(output_path, "wb")) == NULL) {
        fprintf(stderr, "Cannot create output file\n");
        return;
    }
    
    jpeg_stdio_dest(&cinfo_out, outfile);
    
    // Process 8x8 blocks and embed message
    init_dct_matrix();
    
    // ... (embedding logic using DCT coefficients)
    
    // Cleanup
    jpeg_finish_decompress(&cinfo_in);
    jpeg_destroy_decompress(&cinfo_in);
    jpeg_finish_compress(&cinfo_out);
    jpeg_destroy_compress(&cinfo_out);
    fclose(infile);
    fclose(outfile);
}

void decode_message(const char *input_path, const char *output_path) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile, *outfile;
    
    // Initialize JPEG decompression
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    
    if ((infile = fopen(input_path, "rb")) == NULL) {
        fprintf(stderr, "Cannot open input file\n");
        return;
    }
    
    if ((outfile = fopen(output_path, "w")) == NULL) {
        fprintf(stderr, "Cannot create output file\n");
        return;
    }
    
    // ... (extraction logic using DCT coefficients)
    
    // Cleanup
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    fclose(outfile);
}

int main(int argc, char **argv) {
    if (argc != 4 && argc != 3) {
        printf("~~~~~~~~~DCT Steganography for JPEG images~~~~~~~~~\n"
               "To hide message:   %s <Input JPEG> <Output JPEG> <Message File>\n"
               "To extract message: %s <Input JPEG> <Output File>\n", 
               argv[0], argv[0]);
        return 1;
    }
    
    int action = (argc == 4) ? 1 : 0;  // 1 for encode, 0 for decode
    
    if (action) {
        // Read message from file
        FILE *msg_file = fopen(argv[3], "r");
        if (!msg_file) {
            fprintf(stderr, "Cannot open message file\n");
            return 1;
        }
        
        // Get message
        char message[1024];
        fgets(message, sizeof(message), msg_file);
        fclose(msg_file);
        
        encode_message(argv[1], argv[2], message);
    } else {
        decode_message(argv[1], argv[2]);
    }
    
    return 0;
}