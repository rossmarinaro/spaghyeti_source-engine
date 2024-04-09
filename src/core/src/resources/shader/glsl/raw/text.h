#pragma once

namespace Shaders {


static const char* textVertex =

    "#version 430\n"
    "layout(location = 0) in vec2 inPosition;\n" 
    "layout(location = 0) out vec2 texturePos;\n" 
    "uniform vec2 resolution;\n" 
    "uniform vec2 position;\n" 
    "uniform mat4 projection;\n" 
    "uniform vec2 size;\n" 
    "void main() {\n" 
        "vec2 pos = (inPosition * size / resolution);\n" 
        "pos = pos + (position / resolution);\n" 
        "gl_Position = projection * vec4(pos * 2.0 - vec2(1.0, 1.0), 0.0, 1.0);\n" 
        "texturePos = vec2(inPosition.x, (inPosition.y - 1.0) * -1.0);\n" 
    "}"; 

static const char* textFragment =

    "#version 430\n"
    "layout(location = 0) in vec2 texturePos;\n" 
    "layout(location = 0) out vec4 out Color;\n" 
    "uniform sampler2D fontTexture;\n" 
    "uniform vec4 charPosition;\n" 
    "void main() {\n" 
        "ivec2 texSize = textureSize(fontTexture, 0);\n" 
        "vec2 texPos = texturePos * ((charPosition.zw - charPosition.xy) / texSize.x) + charPosition.xy / texSize.y;\n"  
        "float col = texture(fontTexture, texPos).r;\n"  
        "vec3 textColor = vec3(texPos.x, texturePos.y, texPos.y);\n"  
        "outColor = vec4(textColor, col);\n"  
    "}";
}

