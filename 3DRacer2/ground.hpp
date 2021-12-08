#ifndef GROUND_HPP_
#define GROUND_HPP_

#include <vector>
#include <random>

#include "abcg.hpp"
#include "gamedata.hpp"

class OpenGLWindow;

class Ground {
    public:
        void loadDiffuseTexture(std::string_view path);
        void loadObj(std::string_view path, bool standardize = false);
        void initializeGL(GLuint program);
        void paintGL();
        void terminateGL();
        void update(const GameData &gameData, float deltaTime);

        [[nodiscard]] int getNumTriangles() const {
        return static_cast<int>(m_indices.size()) / 3;
        }
        
        [[nodiscard]] glm::vec4 getKa() const { return m_Ka; }
        [[nodiscard]] glm::vec4 getKd() const { return m_Kd; }
        [[nodiscard]] glm::vec4 getKs() const { return m_Ks; }
        [[nodiscard]] float getShininess() const { return m_shininess; }

        [[nodiscard]] bool isUVMapped() const { return m_hasTexCoords; }

    private:
        friend OpenGLWindow;

        static const int m_numGrounds{3};

        Camera m_camera;

        GLuint m_VAO{};
        GLuint m_VBO{};
        GLuint m_EBO{};
        GLuint m_program{};

        std::default_random_engine m_randomEngine;

        std::vector<Vertex> m_vertices;
        std::vector<GLuint> m_indices;

        std::array<glm::vec3, m_numGrounds> m_groundPositions;

        GLint m_modelMatrixLoc{};
        GLint m_colorLoc{};

        void standardize();
        void computeNormals();

        // Mapping mode
        // 0: triplanar; 1: cylindrical; 2: spherical; 3: from mesh
        int m_mappingMode{};  

        // Light and material properties
        glm::vec4 m_Ka; //{0.0f, 0.2f, 0.0f, 1.0f};
        glm::vec4 m_Kd; //{0.0f, 1.0f, 0.0f, 1.0f};
        glm::vec4 m_Ks; //{1.0f, 1.0f, 1.0f, 1.0f};
        float m_shininess{25.0f};   
        GLuint m_diffuseTexture{};     

        bool m_hasNormals{false};
        bool m_hasTexCoords{false};
};

#endif