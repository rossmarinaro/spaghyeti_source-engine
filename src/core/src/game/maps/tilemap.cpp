#include "../../../../../build/include/game.h"
#include "../../../../../build/include/tilemap.h"
#include "../../../../../build/include/app.h"



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

            if (tileType > -1) {
                auto tile = Game::CreateTileSprite(texture_key, x * tileWidth, y * tileHeight, tileType);
                tile->SetDepth(depth); 
            }

        }

    std::cout << "Tilemap: Initialized layer: " + (std::string)data_key + "\n";

}


//-------------------------------------


void MapManager::ClearMap() {

    Game::entities.erase(std::remove_if(Game::entities.begin(), Game::entities.end(), [](auto t) { return strcmp(t->type, "tile") == 0; }), Game::entities.end());

    std::cout << "Tilemap: layer cleared.\n";

}
