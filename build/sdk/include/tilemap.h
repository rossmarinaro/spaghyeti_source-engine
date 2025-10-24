#pragma once

#include "./entity.h"


class MapManager {

    public:

        std::string currentMap;
        std::vector<std::vector<std::shared_ptr<Sprite>>> layers;

        //create layer from csv or json
        static const bool CreateLayer (
            const char* texture_key, 
            const char* data_key,
            uint32_t mapWidth, 
            uint32_t mapHeight, 
            uint32_t tileWidth, 
            uint32_t tileHeight,
            uint32_t depth,
            int index,
            float posX = 0.0f,
            float posY = 0.0f,
            float rotation = 0.0f,
            float scaleX = 1.0f,
            float scaleY = 1.0f
        );
        
        static void RemoveLayer(const std::string& key);
        static void ClearMap();

}; 




