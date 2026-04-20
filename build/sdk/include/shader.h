#pragma once

#include <string>

#include "./math.h" 

namespace Graphics {

    class Shader {

        public:

            unsigned int ID;
            std::string key;

            void SetFloat(const char* name, float value, bool useShader = true);
            void SetInt(const char* name, int value, bool useShader = true);
            void SetIntV(const char* name, int length, int* value, bool useShader = true);
            void SetVec2f(const char* name, float x, float y, bool useShader = true);
            void SetVec2f(const char* name, const Math::Vector2& value, bool useShader = true);
            void SetVec3f(const char* name, float x, float y, float z, bool useShader = true);
            void SetVec3f(const char* name, const Math::Vector3& value, bool useShader = true);
            void SetVec4f(const char* name, float r, float g, float b, float a, bool useShader = true);
            void SetVec4f(const char* name, const Math::Vector4& value, bool useShader = true);
            void SetMat4(const char* name, const Math::Matrix4& matrix, bool useShader = true);

            void Delete();
            
            static void InitBaseShaders(); 
            static void Load(const std::string& key, const char* vertShader, const char* fragShader, const char* geomPath = nullptr);
            static void UnLoad(const std::string& key);
            static const Shader& Get(const std::string& key);
    
            Shader() = default;
            ~Shader() = default;

        private:

            const bool Generate(const std::string& key, const char* vertexPath, const char* fragmentPath, const char* geomShader = nullptr);

    };

}


