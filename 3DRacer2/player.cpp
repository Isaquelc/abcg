#include "player.hpp"

#include <fmt/core.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <glm/gtc/matrix_inverse.hpp>
#include <filesystem>

// Explicit specialization of std::hash for Vertex
namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const noexcept {
        const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
        const std::size_t h2{std::hash<glm::vec3>()(vertex.normal)};
        return h1 ^ h2;
    }
    };
}  // namespace std

void Player::loadDiffuseTexture(std::string_view path) {
    if (!std::filesystem::exists(path)) return;

    abcg::glDeleteTextures(1, &m_diffuseTexture);
    m_diffuseTexture = abcg::opengl::loadTexture(path);
}

void Player::loadObj(std::string_view path, bool standardize) {
    const auto basePath{std::filesystem::path{path}.parent_path().string() + "/"};

    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.mtl_search_path = basePath;  // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path.data(), readerConfig)) {
        if (!reader.Error().empty()) {
        throw abcg::Exception{abcg::Exception::Runtime(
            fmt::format("Failed to load model {} ({})", path, reader.Error()))};
        }
        throw abcg::Exception{
            abcg::Exception::Runtime(fmt::format("Failed to load model {}", path))};
    }

    if (!reader.Warning().empty()) {
        fmt::print("Warning: {}\n", reader.Warning());
    }

    const auto& attrib{reader.GetAttrib()};
    const auto& shapes{reader.GetShapes()};
    const auto& materials{reader.GetMaterials()};

    m_vertices.clear();
    m_indices.clear();

    m_hasNormals = false;
    m_hasTexCoords = false;

    // A key:value map with key=Vertex and value=index
    std::unordered_map<Vertex, GLuint> hash{};

    // Loop over shapes
    for (const auto& shape : shapes) {
        // Loop over indices
        for (const auto offset : iter::range(shape.mesh.indices.size())) {
        // Access to vertex
        const tinyobj::index_t index{shape.mesh.indices.at(offset)};

        // Vertex position
        const int startIndex{3 * index.vertex_index};
        const float vx{attrib.vertices.at(startIndex + 0)};
        const float vy{attrib.vertices.at(startIndex + 1)};
        const float vz{attrib.vertices.at(startIndex + 2)};

        // Vertex normal
        float nx{};
        float ny{};
        float nz{};
        if (index.normal_index >= 0) {
            m_hasNormals = true;
            const int normalStartIndex{3 * index.normal_index};
            nx = attrib.normals.at(normalStartIndex + 0);
            ny = attrib.normals.at(normalStartIndex + 1);
            nz = attrib.normals.at(normalStartIndex + 2);
        }

        // Vertex texture coordinates
        float tu{};
        float tv{};
        if (index.texcoord_index >= 0) {
            m_hasTexCoords = true;
            const int texCoordsStartIndex{2 * index.texcoord_index};
            tu = attrib.texcoords.at(texCoordsStartIndex + 0);
            tv = attrib.texcoords.at(texCoordsStartIndex + 1);
        }

        Vertex vertex{};
        vertex.position = {vx, vy, vz};
        vertex.normal = {nx, ny, nz};
        vertex.texCoord = {tu, tv};

        // If hash doesn't contain this vertex
        if (hash.count(vertex) == 0) {
            // Add this index (size of m_vertices)
            hash[vertex] = m_vertices.size();
            // Add this vertex
            m_vertices.push_back(vertex);
        }

        m_indices.push_back(hash[vertex]);
        }
    }

    // Use properties of first material, if available
    if (!materials.empty()) {
        const auto& mat{materials.at(0)};  // First material
        m_Ka = glm::vec4(mat.ambient[0], mat.ambient[1], mat.ambient[2], 1);
        m_Kd = glm::vec4(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1);
        m_Ks = glm::vec4(mat.specular[0], mat.specular[1], mat.specular[2], 1);
        m_shininess = mat.shininess;

        if (!mat.diffuse_texname.empty())
        loadDiffuseTexture(basePath + mat.diffuse_texname);
    } else {
        // Default values
        m_Ka = {0.1f, 0.1f, 0.1f, 1.0f};
        m_Kd = {0.7f, 0.7f, 0.7f, 1.0f};
        m_Ks = {1.0f, 1.0f, 1.0f, 1.0f};
        m_shininess = 25.0f;
    }

    if (standardize) {
        this->standardize();
    }

    if (!m_hasNormals) {
        computeNormals();
    }
}

void Player::standardize() {
    // Center to origin and normalize largest bound to [-1, 1]

    // Get bounds
    glm::vec3 max(std::numeric_limits<float>::lowest());
    glm::vec3 min(std::numeric_limits<float>::max());
    for (const auto& vertex : m_vertices) {
        max.x = std::max(max.x, vertex.position.x);
        max.y = std::max(max.y, vertex.position.y);
        max.z = std::max(max.z, vertex.position.z);
        min.x = std::min(min.x, vertex.position.x);
        min.y = std::min(min.y, vertex.position.y);
        min.z = std::min(min.z, vertex.position.z);
    }

    // Center and scale
    const auto center{(min + max) / 2.0f};
    const auto scaling{2.0f / glm::length(max - min)};
    for (auto& vertex : m_vertices) {
        vertex.position = (vertex.position - center) * scaling;
    }
}

void Player::computeNormals() {
    // Clear previous vertex normals
    for (auto& vertex : m_vertices) {
        vertex.normal = glm::zero<glm::vec3>();
    }

    // Compute face normals
    for (const auto offset: iter::range<int>(0, m_indices.size(), 3)) {
        // Get face vertices
        Vertex& a{m_vertices.at(m_indices.at(offset + 0))};
        Vertex& b{m_vertices.at(m_indices.at(offset + 1))};
        Vertex& c{m_vertices.at(m_indices.at(offset + 2))};

        // Compute normal
        const auto edge1{b.position - a.position};
        const auto edge2{a.position - b.position};
        const glm::vec3 normal{glm::cross(edge1, edge2)};

        // Acumulate on vertices
        a.normal += normal;
        b.normal += normal;
        c.normal += normal;
    }

    // Normalize
    for (auto& vertex : m_vertices) {
        vertex.normal = glm::normalize(vertex.normal);
    }

    m_hasNormals = true;
}

void Player::initializeGL(GLuint program) {
    terminateGL();
    m_translation = glm::vec3(0.0f, 0.0f, -5.0f);
    m_angle = 180.0f;

    m_program = program;

    // Generate VBO
    abcg::glGenBuffers(1, &m_VBO);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);
    abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Generate EBO
    abcg::glGenBuffers(1, &m_EBO);
    abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);
    abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Create VAO
    abcg::glGenVertexArrays(1, &m_VAO);

    // Bind vertex attributes to current VAO
    abcg::glBindVertexArray(m_VAO);

    abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    const GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};
    if (positionAttribute >= 0) {
        abcg::glEnableVertexAttribArray(positionAttribute);
        abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    }

    const GLint normalAttribute{abcg::glGetAttribLocation(program, "inNormal")};
    if (normalAttribute >= 0) {
        abcg::glEnableVertexAttribArray(normalAttribute);
        GLsizei offset{sizeof(glm::vec3)};
        abcg::glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offset));
    }

    const GLint texCoordAttribute{abcg::glGetAttribLocation(program, "inTexCoord")};
    if (texCoordAttribute >= 0) {
        abcg::glEnableVertexAttribArray(texCoordAttribute);
        GLsizei offset{sizeof(glm::vec3) + sizeof(glm::vec3)};
        abcg::glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                                    sizeof(Vertex),
                                    reinterpret_cast<void*>(offset));
    }


    abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
    abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    // End of binding to current VAO
    abcg::glBindVertexArray(0);
}

void Player::paintGL() {
    abcg::glUseProgram(m_program);

    // Get location of uniform variables (could be precomputed)
    GLint modelMatrixLoc{abcg::glGetUniformLocation(m_program, "modelMatrix")};
    GLint normalMatrixLoc{abcg::glGetUniformLocation(m_program, "normalMatrix")};
    GLint shininessLoc{abcg::glGetUniformLocation(m_program, "shininess")};
    GLint KaLoc{abcg::glGetUniformLocation(m_program, "Ka")};
    GLint KdLoc{abcg::glGetUniformLocation(m_program, "Kd")};
    GLint KsLoc{abcg::glGetUniformLocation(m_program, "Ks")};
    GLint diffuseTexLoc{abcg::glGetUniformLocation(m_program, "diffuseTex")};
    GLint mappingModeLoc{abcg::glGetUniformLocation(m_program, "mappingMode")};  

    m_playerPos = glm::mat4{1.0f};
    m_playerPos = glm::translate(m_playerPos, m_translation); // moves player slightly forward
    m_playerPos = glm::rotate(m_playerPos, glm::radians(m_angle), glm::vec3(0.0f, 1.0f, 0.0f)); // no initial rotation
    m_playerPos = glm::scale(m_playerPos, glm::vec3(1.0f)); // no further scaling

    // Set uniform variables of the current object
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &m_playerPos[0][0]);
    abcg::glUniform1i(diffuseTexLoc, 0);
    abcg::glUniform1i(mappingModeLoc, m_mappingMode);

    abcg::glUniform1f(shininessLoc, m_shininess);
    abcg::glUniform4fv(KaLoc, 1, &m_Ka.x);
    abcg::glUniform4fv(KdLoc, 1, &m_Kd.x);
    abcg::glUniform4fv(KsLoc, 1, &m_Ks.x);

    abcg::glBindVertexArray(m_VAO);

    m_camera.computeViewMatrix();
    auto modelViewMatrix{glm::mat3(m_camera.m_viewMatrix * m_playerPos)};
    glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
    abcg::glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

    abcg::glActiveTexture(GL_TEXTURE0);
    abcg::glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);

    // Set minification and magnification parameters
    abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Set texture wrapping parameters
    abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);

    abcg::glBindVertexArray(0);
    abcg::glUseProgram(0);
}

void Player::update(const GameData &gameData, float deltaTime) {
    // Move
    if (gameData.m_input[static_cast<size_t>(Input::Left)] && m_translation.x>-2) {
        m_translation.x -= 5.0 * deltaTime;
        if (m_angle < 180) {
            m_angle = 180;
        }
        else if (m_angle < 200) {
            m_angle += 200 * deltaTime;
        }
        // m_angle = 225;
    }
    if (gameData.m_input[static_cast<size_t>(Input::Right)] && m_translation.x<2) {
        m_translation.x += 5.0 * deltaTime;
        if (m_angle > 180) {
            m_angle = 180;
        }
        else if (m_angle > 160) {
            m_angle -= 200 * deltaTime;
        }
        // m_angle = 135;
    }
    if (! gameData.m_input[static_cast<size_t>(Input::Left)] && 
        ! gameData.m_input[static_cast<size_t>(Input::Right)] &&
        m_angle > 180)
        m_angle -= 200 * deltaTime;
    if (! gameData.m_input[static_cast<size_t>(Input::Left)] && 
        ! gameData.m_input[static_cast<size_t>(Input::Right)] &&
        m_angle < 180)
        m_angle += 200 * deltaTime;
}

void Player::terminateGL() {
    abcg::glDeleteBuffers(1, &m_EBO);
    abcg::glDeleteBuffers(1, &m_VBO);
    abcg::glDeleteVertexArrays(1, &m_VAO);
}