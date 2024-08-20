#pragma once

#include <string>

#include "./renderer.h"

class Shader {


    public:

        unsigned int ID;
        const char* key;

        void SetFloat(const char* name, float value, bool useShader = true);
        void SetInt(const char* name, int value, bool useShader = true);
        void SetVec2f(const char* name, float x, float y, bool useShader = true);
        void SetVec2f(const char* name, const glm::vec2& value, bool useShader = true);
        void SetVec3f(const char* name, float x, float y, float z, bool useShader = true);
        void SetVec3f(const char* name, const glm::vec3& value, bool useShader = true);
        void SetVec4f(const char* name, float x, float y, float z, float w, bool useShader = true);
        void SetVec4f(const char* name, const glm::vec4& value, bool useShader = true);
        void SetMat4(const char* name, const glm::mat4& matrix, bool useShader = true);

        void Delete();
        
        static void InitBaseShaders(); 
        static void Update(Camera* camera);
        static void Load(const std::string& key, const char* vertShader, const char* fragShader, const char* geomShader = nullptr);
        static void UnLoad(const std::string& key);
        static Shader& Get(const std::string& key);

        Shader() = default;
        ~Shader() = default;

    private:

        void Generate(const std::string& key, const char* vertexPath, const char* fragmentPath, const char* geomPath = nullptr);
        Shader& Use();

};

