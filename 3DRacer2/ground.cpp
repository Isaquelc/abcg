#include "ground.hpp"

#include <fmt/core.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <filesystem>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

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

void Ground::loadDiffuseTexture(std::string_view path) {
    if (!std::filesystem::exists(path)) return;

    abcg::glDeleteTextures(1, &m_diffuseTexture);
    m_diffuseTexture = abcg::opengl::loadTexture(path);
}

void Ground::loadObj(std::string_view path, bool standardize) {
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

void Ground::standardize() {
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

void Ground::computeNormals() {
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

void Ground::initializeGL(GLuint program) {
    terminateGL();
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
    const GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};\
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

    // for (const auto index : iter::range(m_numGrounds)) {
    //     auto &position{m_groundPositions.at(index)};
    //     position = glm::vec3(0.0f, 0.0f, -10.0f * index);
    // }
    auto &position1{m_groundPositions.at(0)};
    auto &position2{m_groundPositions.at(1)};
    auto &position3{m_groundPositions.at(2)};
    position1 = glm::vec3(0.0f, -0.215f, -200.0f);
    position2 = glm::vec3(0.0f, -0.215f, -100.0f);
    position3 = glm::vec3(0.0f, -0.215f, 0.0f);

    // End of binding to current VAO
    abcg::glBindVertexArray(0);
}

void Ground::paintGL() {
    abcg::glUseProgram(m_program);
    abcg::glBindVertexArray(m_VAO);

    // Get location of uniform variables (could be precomputed)
    const GLint modelMatrixLoc{abcg::glGetUniformLocation(m_program, "modelMatrix")};
    const GLint normalMatrixLoc{abcg::glGetUniformLocation(m_program, "normalMatrix")};
    const GLint shininessLoc{abcg::glGetUniformLocation(m_program, "shininess")};
    const GLint KaLoc{abcg::glGetUniformLocation(m_program, "Ka")};
    const GLint KdLoc{abcg::glGetUniformLocation(m_program, "Kd")};
    const GLint KsLoc{abcg::glGetUniformLocation(m_program, "Ks")};
    const GLint diffuseTexLoc{abcg::glGetUniformLocation(m_program, "diffuseTex")};
    const GLint mappingModeLoc{abcg::glGetUniformLocation(m_program, "mappingMode")}; 

    for (const auto index : iter::range(m_numGrounds)) {
        auto &position{m_groundPositions.at(index)};

        // compute model matrix of the current car
        glm::mat4 groundMatrix{1.0f};
        groundMatrix = glm::translate(groundMatrix, position); 
        groundMatrix = glm::rotate(groundMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        groundMatrix = glm::scale(groundMatrix, glm::vec3(1.0f, 1.0f, 0.50f));

        abcg::glUniform1f(shininessLoc, m_shininess);
        abcg::glUniform4fv(KaLoc, 1, &m_Ka.x);
        abcg::glUniform4fv(KdLoc, 1, &m_Kd.x);
        abcg::glUniform4fv(KsLoc, 1, &m_Ks.x);

        abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &groundMatrix[0][0]);
        abcg::glUniform1i(diffuseTexLoc, 0);
        abcg::glUniform1i(mappingModeLoc, m_mappingMode);

        m_camera.computeViewMatrix();
        const auto modelViewMatrix{glm::mat3(m_camera.m_viewMatrix * groundMatrix)};
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

        abcg::glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
    }

    abcg::glBindVertexArray(0);
    abcg::glUseProgram(0);
}

void Ground::update(const GameData &gameData, float deltaTime) {
    for (const auto index : iter::range(m_numGrounds)) {
        auto &position{m_groundPositions.at(index)};

        // Move enemy towards camera
        position.z += deltaTime * gameData.gameSpeed * 25.0f;

        // If this car is behind the camera, move it back with a new random x position and a slightly random z position
        if (position.z > 100.0f) {
            position.z = -200.0f;
        }
    }
}

void Ground::terminateGL() {
    abcg::glDeleteBuffers(1, &m_EBO);
    abcg::glDeleteBuffers(1, &m_VBO);
    abcg::glDeleteVertexArrays(1, &m_VAO);
}