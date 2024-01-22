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
    uint32_t tileHeight
)
{

    const std::vector<std::string> &map = System::Resources::Manager::GetRawTilemapData(data_key);

    if (!map.size()) {

        std::cout << "Tilemap: layer data not found.\n";

        return;
    }

    for (int y = 0; y < mapHeight; ++y)
        for (int x = 0; x < mapWidth; ++x) 
        {

            //convert string to int at index

            int tileType = atoi(map[x + y * mapWidth].c_str());

            //skip if no tile

            if (tileType > -1) {
                auto tile = Game::CreateSprite(texture_key, x * tileWidth, y * tileHeight, tileType);
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
