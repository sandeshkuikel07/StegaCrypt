# StegaCrypt

StegaCrypt is a steganography tool that implements various Least Significant Bit (LSB) methods to hide messages within BMP image files. The project includes three different LSB implementations, each offering different approaches to message concealment.

## Features

- Three different steganographic methods:
  - **LSB**: Classic Least Significant Bit implementation
  - **LSB2**: Enhanced version using XOR of last two bits
  - **LSB3**: Advanced version using XOR of last three bits
- Support for BMP image files
- Message embedding and extraction capabilities
- Minimal image degradation
- Performance metrics (Total pixel changes and Mean Squared Error)

## Requirements

- C compiler (GCC recommended)
- BMP image files for testing

## Installation

Clone the repository:

```bash
git clone https://github.com/sandeshkuikel07/StegaCrypt.git
cd StegaCrypt
```

## Usage

To use the steganography tool, follow these steps:

1. Compile the program :
(for e.g)
```bash
./gcc lsb1.c -o lsb1
```

2. Run the program with the desired method and parameters:

```bash
./stegacrypt <method> <input_image> <output_image> -- msg
for e.g -- (./stegacrypt/lsb1  image.bmp encoded.bmp )

```

3. Extract the message from the stego image:

```bash
./stegacrypt <method> <stego_image> <extracted_message>
(./stegacrypt lsb1 encoded.bmp ext.txt)
```

## Metrics

The program calculates and displays the following metrics:

- Total pixel changes
- Mean Squared Error


## Implementation Details

### LSB
- Modifies only the least significant bit of each byte
- Classic approach with direct bit manipulation

### LSB2
- Uses XOR of last two bits
- Provides better message concealment than basic LSB
- Minimizes color changes in the image

### LSB3
- Implements XOR of last three bits
- Offers enhanced security through additional bit complexity
- Maintains reasonable image quality while increasing concealment

## Limitations

- Only works with BMP image files
- Maximum message size is 999 characters
- Image must be large enough to accommodate the message
- No encryption of the hidden message (only steganography)



