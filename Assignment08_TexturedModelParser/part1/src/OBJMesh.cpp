#include "OBJMesh.hpp"
#include "TextureLoader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include "TextureLoader.hpp"

OBJMesh::OBJMesh() : m_textureID(0) {}

OBJMesh::~OBJMesh() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
}

bool OBJMesh::LoadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    // Add debug output
    std::cout << "Loading OBJ file: " << filename << std::endl;

    // Clear any existing data
    positions.clear();
    normals.clear();
    texCoords.clear();
    m_triangles.clear();

    std::string line;
    int vertexCount = 0;
    int normalCount = 0;
    int faceCount = 0;
    std::string mtlFile;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "mtllib") {
            iss >> mtlFile;
            // Get directory of OBJ file
            size_t lastSlash = filename.find_last_of("/\\");
            std::string directory = lastSlash != std::string::npos ?
                                  filename.substr(0, lastSlash + 1) : "";
            LoadMTL(directory + mtlFile);
        }
        else if (type == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back(glm::vec3(x, y, z));
            vertexCount++;
        }
        else if (type == "vn") {
            float nx, ny, nz;
            iss >> nx >> ny >> nz;
            normals.push_back(glm::normalize(glm::vec3(nx, ny, nz)));
            normalCount++;
        }
        else if (type == "vt") {
            float s, t;
            iss >> s >> t;
            texCoords.push_back(glm::vec2(s, 1.0f - t)); // Flip t coordinate
            normalCount++;
        }
        else if (type == "f") {
            std::string vertex1, vertex2, vertex3;
            iss >> vertex1 >> vertex2 >> vertex3;

            auto [v1, vn1] = ParseVertexIndices(vertex1);
            auto [v2, vn2] = ParseVertexIndices(vertex2);
            auto [v3, vn3] = ParseVertexIndices(vertex3);

            Triangle tri;
            // Convert to vertex format
            tri.vertices[0] = {
                positions[v1].x, positions[v1].y, positions[v1].z,
                0.7f, 0.7f, 0.7f,  // Default color
                normals[vn1].x, normals[vn1].y, normals[vn1].z,
                texCoords[v1].x, texCoords[v1].y
            };
            tri.vertices[1] = {
                positions[v2].x, positions[v2].y, positions[v2].z,
                0.7f, 0.7f, 0.7f,
                normals[vn2].x, normals[vn2].y, normals[vn2].z,
                texCoords[v2].x, texCoords[v2].y
            };
            tri.vertices[2] = {
                positions[v3].x, positions[v3].y, positions[v3].z,
                0.7f, 0.7f, 0.7f,
                normals[vn3].x, normals[vn3].y, normals[vn3].z,
                texCoords[v3].x, texCoords[v3].y
            };

            m_triangles.push_back(tri);
            faceCount++;
        }
    }

    std::cout << "Loaded OBJ with:" << std::endl;
    std::cout << "Vertices: " << vertexCount << std::endl;
    std::cout << "Normals: " << normalCount << std::endl;
    std::cout << "Faces: " << faceCount << std::endl;
    std::cout << "Triangles in mesh: " << m_triangles.size() << std::endl;
    return true;
}

std::pair<int, int> OBJMesh::ParseVertexIndices(const std::string& vertexStr) const {
    size_t slash1 = vertexStr.find('/');
    size_t slash2 = vertexStr.find('/', slash1 + 1);

    if (slash1 == std::string::npos) {
        // Only vertex position index
        return {std::stoi(vertexStr) - 1, 0};
    }

    // Extract vertex/texture/normal indices
    std::string vStr = vertexStr.substr(0, slash1);
    std::string vtStr = vertexStr.substr(slash1 + 1, slash2 - slash1 - 1);
    std::string vnStr = vertexStr.substr(slash2 + 1);

    int vIdx = std::stoi(vStr) - 1;
    int vtIdx = vtStr.empty() ? 0 : std::stoi(vtStr) - 1;
    int vnIdx = vnStr.empty() ? 0 : std::stoi(vnStr) - 1;

    return {vIdx, vnIdx};
}

bool OBJMesh::LoadMTL(const std::string& filename) {
    std::cout << "\nAttempting to load MTL file: " << filename << std::endl;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open MTL file: " << filename << std::endl;
        return false;
    }
    std::cout << "Successfully opened MTL file" << std::endl;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "newmtl") {
            iss >> m_material.name;
            std::cout << "Found material: " << m_material.name << std::endl;
        }
        else if (token == "map_Kd") {
            iss >> m_material.diffuseTexture;
            // Get directory of MTL file
            size_t lastSlash = filename.find_last_of("/\\");
            std::string directory = lastSlash != std::string::npos ?
                                  filename.substr(0, lastSlash + 1) : "";

            std::string fullTexturePath = directory + m_material.diffuseTexture;
            std::cout << "Found texture path: " << fullTexturePath << std::endl;

            // Load the texture using TextureLoader
            m_textureID = TextureLoader::LoadPPM(fullTexturePath);
            if (m_textureID == 0) {
                std::cerr << "ERROR: Failed to load texture: " << fullTexturePath << std::endl;
                return false;
            }
            std::cout << "Successfully loaded texture. TextureID: " << m_textureID << std::endl;
        }
    }

    return true;
}

void OBJMesh::SetupBuffers(GLuint& vao, GLuint& vbo) {
    std::vector<GLfloat> vertexData;

    for (const auto& triangle : m_triangles) {
        for (const auto& vertex : triangle.vertices) {
            vertexData.push_back(vertex.x);
            vertexData.push_back(vertex.y);
            vertexData.push_back(vertex.z);
            vertexData.push_back(vertex.r);
            vertexData.push_back(vertex.g);
            vertexData.push_back(vertex.b);
            vertexData.push_back(vertex.nx);
            vertexData.push_back(vertex.ny);
            vertexData.push_back(vertex.nz);
            vertexData.push_back(vertex.s);
            vertexData.push_back(vertex.t);
        }
    }

    // Create and bind VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and bind VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertexData.size() * sizeof(GL_FLOAT),
                 vertexData.data(),
                 GL_STATIC_DRAW);

    // Set up vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 11, (void*)0);
    // Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 11, (void*)(sizeof(GL_FLOAT) * 3));
    // Normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 11, (void*)(sizeof(GL_FLOAT) * 6));
    // Texture coordinates
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 11, (void*)(sizeof(GL_FLOAT) * 9));

    glBindVertexArray(0);
}

size_t OBJMesh::GetTriangleCount() const {
    return m_triangles.size();
}
