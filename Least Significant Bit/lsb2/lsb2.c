#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_MSG_SIZE 1000

// Get bit from byte at position
static int get_bit(unsigned char byte, int pos) {
    return (byte >> pos) & 1;
}

// LSB2 implementation - returns XOR of last two bits
static int get_lsb2(unsigned char byte) {
    return ((byte & 1) ^ ((byte >> 1) & 1));
}

// Modify byte to match desired LSB2 value while minimizing color change
static unsigned char modify_lsb2(unsigned char byte, int desired_bit) {
    int current_lsb2 = get_lsb2(byte);
    if (current_lsb2 != desired_bit) {
        // Only toggle LSB if needed, preserving color as much as possible
        byte ^= 1;
    }
    return byte;
}

// Calculate Header Offset
static int img_offset(FILE* img) {
    unsigned char header[14];
    fread(header, 1, 14, img);
    return header[10] | (header[11] << 8) | (header[12] << 16) | (header[13] << 24);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("~~~~~~~~~LSB2 Steganography~~~~~~~~~\n"
               "To hide message:   %s <Input BMP> <Output BMP>\n"
               "To extract message: %s <Input BMP> <Output File>\n", 
               argv[0], argv[0]);
        exit(1);
    }

    FILE *img_read, *msg_file, *img_write;
    // Check if the second argument ends with .bmp to determine action
    int action = (strstr(argv[2], ".bmp") != NULL);

    if ((img_read = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Error: Unable to open input image %s\n", argv[1]);
        return 1;
    }

    // For encryption (if output is .bmp)
    if (action) {
        if ((img_write = fopen(argv[2], "wb")) == NULL) {
            fprintf(stderr, "Error: Unable to create output image %s\n", argv[2]);
            fclose(img_read);
            return 1;
        }

        // Get message from user
        char message[MAX_MSG_SIZE];
        printf("Enter the message to hide (max %d characters):\n", MAX_MSG_SIZE-1);
        fgets(message, MAX_MSG_SIZE, stdin);
        int msg_length = strlen(message);
        
        // Remove newline if present
        if (message[msg_length-1] == '\n') {
            message[msg_length-1] = '\0';
            msg_length--;
        }

        // Copy header
        int offset = img_offset(img_read);
        rewind(img_read);
        unsigned char buffer[BUFFER_SIZE];
        int bytes_read = fread(buffer, 1, offset, img_read);
        fwrite(buffer, 1, bytes_read, img_write);

        // Write message length
        fwrite(&msg_length, sizeof(int), 1, img_write);

        // Embed message
        int changes = 0;
        for (int i = 0; i < msg_length; i++) {
            unsigned char msg_byte = message[i];
            for (int bit = 0; bit < 8; bit++) {
                unsigned char img_byte;
                fread(&img_byte, 1, 1, img_read);
                
                int desired_bit = get_bit(msg_byte, 7-bit);
                unsigned char new_byte = modify_lsb2(img_byte, desired_bit);
                
                if (new_byte != img_byte) changes++;
                fwrite(&new_byte, 1, 1, img_write);
            }
        }

        // Copy remaining image data
        while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, img_read)) > 0) {
            fwrite(buffer, 1, bytes_read, img_write);
        }

        printf("Message hidden successfully!\n");
        printf("Total pixel changes: %d\n", changes);
        printf("Mean Squared Error = %g\n", changes/(600.0*600.0));
    }
    // For decryption (if output is not .bmp)
    else {
        if ((msg_file = fopen(argv[2], "wb")) == NULL) {
            fprintf(stderr, "Error: Unable to create output file %s\n", argv[2]);
            fclose(img_read);
            return 1;
        }

        // Skip to data section
        int offset = img_offset(img_read);
        fseek(img_read, offset, SEEK_SET);
        
        // Read message length
        int msg_length;
        fread(&msg_length, sizeof(int), 1, img_read);

        // Extract message
        for (int i = 0; i < msg_length; i++) {
            unsigned char msg_byte = 0;
            for (int bit = 0; bit < 8; bit++) {
                unsigned char img_byte;
                fread(&img_byte, 1, 1, img_read);
                msg_byte |= (get_lsb2(img_byte) << (7-bit));
            }
            fwrite(&msg_byte, 1, 1, msg_file);
        }
        printf("Message extracted successfully to %s\n", argv[2]);
    }

    // Close files
    fclose(img_read);
    if (action) fclose(img_write);
    else fclose(msg_file);

    return 0;
}