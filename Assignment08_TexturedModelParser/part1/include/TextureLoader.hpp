#ifndef TEXTURE_LOADER_HPP
#define TEXTURE_LOADER_HPP

#include <string>
#include <glad/glad.h>

class TextureLoader {
public:
    static GLuint LoadPPM(const std::string& filepath);
private:
    static void ReadImageData(const std::string& filepath, int& width, int& height, unsigned char*& data);
};

#endif
