#ifndef OBJMESH_HPP
#define OBJMESH_HPP

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertex {
    float x, y, z;        // position
    float r, g, b;        // color
    float nx, ny, nz;     // normal
    float s, t;           // texture coordinates
};

struct Triangle {
    Vertex vertices[3];
};

class OBJMesh {
public:
    OBJMesh();
    ~OBJMesh();

    bool LoadOBJ(const std::string& filename);
    void SetupBuffers(GLuint& vao, GLuint& vbo);
    size_t GetTriangleCount() const;
    GLuint GetTextureID() const { return m_textureID; }

private:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<Triangle> m_triangles;
    std::pair<int, int> ParseVertexIndices(const std::string& vertexStr) const;

    struct Material {
        std::string name;
        std::string diffuseTexture;
    };

    Material m_material;
    GLuint m_textureID;
    bool LoadMTL(const std::string& filename);
};

#endif
