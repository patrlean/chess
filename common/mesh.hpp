#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "common/stb_image.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    // Mesh Data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Functions
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Draw(GLuint shaderProgram);
    ~Mesh();

private:
    // Render data
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};

#endif // MESH_HPP

