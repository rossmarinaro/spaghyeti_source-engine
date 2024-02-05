#pragma once

#include "../entities/entity.h"

class MapManager {

    public:

        static inline std::vector<std::shared_ptr<Sprite>> tilesprites;

        static void CreateLayer (
            const char* data_key, 
            const char* texture_key, 
            uint32_t mapWidth, 
            uint32_t mapHeight, 
            uint32_t tileWidth, 
            uint32_t tileHeight,
            uint32_t depth
        );
        
        static void ClearMap();

}; 


