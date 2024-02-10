#pragma once


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../../renderer/renderer.h"


class Shader {


public:

    unsigned int ID;
    const char* m_key;

    // utility functions

    void SetFloat(const char* name, float value, bool useShader = false);
    void SetInt(const char* name, int value, bool useShader = false);
    void SetVec2f(const char* name, float x, float y, bool useShader = false);
    void SetVec2f(const char* name, const glm::vec2 &value, bool useShader = false);
    void SetVec3f(const char* name, float x, float y, float z, bool useShader = false);
    void SetVec3f(const char* name, const glm::vec3 &value, bool useShader = false);
    void SetVec4f(const char* name, float x, float y, float z, float w, bool useShader = false);
    void SetVec4f(const char* name, const glm::vec4 &value, bool useShader = false);
    void SetMat4(const char* name, const glm::mat4 &matrix, bool useShader = false);

    void Generate(const char* vertexPath, const char* fragmentPath, const char* geomPath = nullptr);
    Shader& Use();
    
    static void InitBaseShaders();
    static void Update(Camera* camera);
    static void Load(const std::string &key, const char* vertShader, const char* fragShader, const char* geomShader);
    static Shader& GetShader(const std::string &key);

    Shader() = default;
    ~Shader() = default;

private:
            

    GLint transform_location;

};

