#pragma once

#include <string>
#include <memory>
#include <any>

#include "./math.h" 

namespace Graphics {

    class Shader {

        public:

            unsigned int ID;
            std::string key;

            void SetFloat(const char* key, float value);
            void SetInt(const char* key, int value);
            void SetIntV(const char* key, int length, int* value);
            void SetVec2f(const char* key, float x, float y);
            void SetVec2f(const char* key, const Math::Vector2& value);
            void SetVec3f(const char* key, float x, float y, float z);
            void SetVec3f(const char* key, const Math::Vector3& value);
            void SetVec4f(const char* key, float r, float g, float b, float a);
            void SetVec4f(const char* key, const Math::Vector4& value);
            void SetMat4(const char* key, const Math::Matrix4& matrix);

            void Delete();
            void Update();
            
            //includes version and precision float
            static const std::string PreProcessorUtility(bool webgl = false);
            //includes [ MAX_TEXTURES, vec4 SPAGHYETI_WEBGL_TEXTURE_SLOT(int slot, vec2 uv), ivec2 SPAGHYETI_WEBGL_TEXTURE_SIZE(int slot) ]
            static const std::string TextureUtility(bool webgl = false);
            static void InitBaseShaders(); 
            static void Load(const std::string& key, const char* vertShader, const char* fragShader, const char* geomPath = nullptr);
            static void UnLoad(const std::string& key);
            static std::shared_ptr<Shader> Get(const std::string& key);
    
            Shader() = default;
            ~Shader() = default;

        private:

            enum class UniformType { INT, INTV, FLOAT, VEC2, VEC3, VEC4, MAT4 };

            struct Uniform { 
                UniformType type;
                std::string key; 
                std::any value;
                int length;
            };

            std::vector<Uniform> m_uniforms;

            const bool Generate(const std::string& key, const char* vertexPath, const char* fragmentPath, const char* geomShader = nullptr);
    };

}


