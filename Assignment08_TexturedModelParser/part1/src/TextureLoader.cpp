#include "TextureLoader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

GLuint TextureLoader::LoadPPM(const std::string& filepath) {
    std::cout << "\nAttempting to load PPM texture: " << filepath << std::endl;

    int width, height;
    unsigned char* data = nullptr;
    ReadImageData(filepath, width, height, data);

    if (!data) {
        std::cerr << "ERROR: Failed to load texture data" << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load the texture data
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        data
    );

    delete[] data;

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "ERROR: OpenGL error in LoadPPM: " << err << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}
void TextureLoader::ReadImageData(const std::string& filepath, int& width, int& height, unsigned char*& data) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "ERROR: Failed to open PPM file: " << filepath << std::endl;
        return;
    }
    std::cout << "Successfully opened PPM file" << std::endl;

    std::string magicNumber;
    file >> magicNumber;  // Read P3 or P6
    std::cout << "PPM format: " << magicNumber << std::endl;

    if (magicNumber != "P3" && magicNumber != "P6") {
        std::cerr << "ERROR: Invalid PPM format. Only P3 and P6 are supported." << std::endl;
        file.close();
        return;
    }
    bool isBinary = (magicNumber == "P6");

    // Skip comments and whitespace
    char c;
    while (file.get(c) && (c == ' ' || c == '\n' || c == '\r'));
    while (c == '#') {
        while (file.get(c) && c != '\n');
        while (file.get(c) && (c == ' ' || c == '\n' || c == '\r'));
    }
    file.unget();  // Put back the last character we read

    // Read dimensions
    file >> width >> height;

    // Read max color value
    int maxColorValue;
    file >> maxColorValue;
    std::cout << "Image dimensions: " << width << "x" << height << std::endl;
    std::cout << "Max color value: " << maxColorValue << std::endl;

    if (maxColorValue > 255) {
        std::cerr << "ERROR: Only 8-bit PPM files are supported" << std::endl;
        file.close();
        return;
    }

    // Skip any remaining whitespace before pixel data
    if (isBinary) {
        file.get();  // Skip single whitespace character after maxval
    }

    // Allocate memory for temporary image data
    unsigned char* copyData = new unsigned char[width * height * 3];
    std::cout << "Allocated memory for image data" << std::endl;

    if (isBinary) {
        std::cout << "Reading binary PPM data..." << std::endl;
        file.read(reinterpret_cast<char*>(copyData), width * height * 3);
    } else {
        std::cout << "Reading ASCII PPM data..." << std::endl;
        for (int i = 0; i < width * height * 3; ++i) {
            int value;
            file >> value;
            copyData[i] = static_cast<unsigned char>(value);
        }
    }

    // Allocate memory for flipped image data
    data = new unsigned char[width * height * 3];

    // Flip the image vertically and swap RGB to BGR
    unsigned int pos = (width * height * 3) - 1;
    for(int i = 0; i < width * height * 3; i += 3) {
        data[pos] = copyData[i + 2];     // Blue
        data[pos - 1] = copyData[i + 1]; // Green
        data[pos - 2] = copyData[i];     // Red
        pos -= 3;
    }

    delete[] copyData;

    if (file.fail()) {
        std::cerr << "ERROR: Error occurred while reading PPM file" << std::endl;
        delete[] data;
        data = nullptr;
    }
    file.close();

    std::cout << "Successfully read PPM data: " << width << "x" << height << " pixels" << std::endl;
}
