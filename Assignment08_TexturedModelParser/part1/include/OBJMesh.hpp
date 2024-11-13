#ifndef OBJMESH_HPP
#define OBJMESH_HPP

#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertex {
    float x, y, z;        // Position
    float r, g, b;        // Color
    float nx, ny, nz;     // Normal
    float s, t;           // Texture coordinates

    // Default constructor
    Vertex() : x(0), y(0), z(0), r(0), g(0), b(0),
               nx(0), ny(0), nz(0), s(0), t(0) {}

    // Constructor with initialization
    Vertex(float x_, float y_, float z_,
           float r_, float g_, float b_,
           float nx_, float ny_, float nz_,
           float s_, float t_) :
           x(x_), y(y_), z(z_),
           r(r_), g(g_), b(b_),
           nx(nx_), ny(ny_), nz(nz_),
           s(s_), t(t_) {}

    // Additional constructor for position only
    Vertex(float x_, float y_, float z_) :
           x(x_), y(y_), z(z_),
           r(1), g(1), b(1),
           nx(0), ny(0), nz(0),
           s(0), t(0) {}
};

struct Triangle {
    Vertex vertices[3];

    Triangle() : vertices() {} // Initialize array
};

struct Material {
    std::string name;
    std::string diffuseTexture;

    Material() : name(), diffuseTexture() {} // Initialize strings
};

class OBJMesh {
private:
    std::vector<Triangle> m_triangles;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    Material m_material;
    GLuint m_textureID;
    std::string m_pendingTexturePath;

    bool LoadMTL(const std::string& filename);
    std::tuple<int, int, int> ParseVertexIndices(const std::string& vertexStr) const;

public:
    OBJMesh();
    ~OBJMesh();

    bool LoadOBJ(const std::string& filename);
    bool LoadTextures();
    void SetupBuffers(GLuint& vao, GLuint& vbo);
    size_t GetTriangleCount() const;
    GLuint GetTextureID() const { return m_textureID; }

    // Add some helper functions
    bool HasTexture() const { return m_textureID != 0; }
    const std::vector<Triangle>& GetTriangles() const { return m_triangles; }
};

#endif // OBJMESH_HPP
