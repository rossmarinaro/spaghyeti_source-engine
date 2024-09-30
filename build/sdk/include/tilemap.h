#pragma once

#include "./entity.h"


class MapManager {

    public:

        std::string currentMap;
        std::vector<std::vector<std::shared_ptr<Sprite>>> layers;

        static bool CreateLayer (
            const char* texture_key, 
            const char* data_key,
            uint32_t mapWidth, 
            uint32_t mapHeight, 
            uint32_t tileWidth, 
            uint32_t tileHeight,
            uint32_t depth
        );
        
        static void RemoveLayer(const std::string& key);
        static void ClearMap();

}; 




