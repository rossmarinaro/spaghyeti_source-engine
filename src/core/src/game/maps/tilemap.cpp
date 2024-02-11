#include "../game.h"
#include "./tilemap.h"
#include "../../app/app.h"



//------------------------------------- load tilemap layer


void MapManager::CreateLayer (

    const char* data_key,
    const char* texture_key,
    uint32_t mapWidth,
    uint32_t mapHeight,
    uint32_t tileWidth,
    uint32_t tileHeight,
    uint32_t depth
)
{

    const std::vector<std::string> &data = System::Resources::Manager::GetRawTilemapData(data_key);
    std::vector<std::string> map;

    std::stringstream ss; 
    std::string line; 


    if (!data.size()) {

        std::cout << "Tilemap: layer data not found.\n";

        return;
    }

    //remove commas from array before parsing

    for (int i = 0; i < data.size(); i++) 
        ss << data[i]; 

    while(getline(ss, line, ','))
        map.push_back(line);  

    for (int y = 0; y < mapHeight; ++y)
        for (int x = 0; x < mapWidth; ++x) 
        {

            //convert string to int at index

            int tileType = atoi(map[x + y * mapWidth].c_str());

            //skip if no tile

            if (tileType > -1) 
            {
                auto tile = Game::CreateSprite(texture_key, x * tileWidth, y * tileHeight, tileType);
                tile->m_depth = depth;
                tilesprites.push_back(tile);  
            }

        }

    std::cout << "Tilemap: Initialized layer: " + (std::string)data_key + "\n";

}


//-------------------------------------


void MapManager::ClearMap()
{

  for (auto &tile : tilesprites) {                

    Game::DestroySprite(tile);

    tile.reset();
    tile = nullptr;
  }

  tilesprites.clear();

  std::cout << "Tilemap: layer cleared.\n";

}
