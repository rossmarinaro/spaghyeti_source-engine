#include "../../../../../build/include/tilemap.h"
#include "../../../../../build/include/app.h"

using app = System::Application;

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
    std::vector<std::shared_ptr<Sprite>> layer; 

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
                auto tile = app::game->CreateTileSprite(texture_key, x * tileWidth, y * tileHeight, tileType); 
                
                tile->SetDepth(depth); 
                tile->ID = (std::string)data_key;

                //add layer to stack

                layer.push_back(tile);
            }

        }

    app::game->maps->layers.push_back(layer);

    std::cout << "Tilemap: Initialized layer: " + (std::string)data_key + "\n";

}


//------------------------------------- remove layer


void MapManager::RemoveLayer(const std::string& key) 
{

    for (auto it = app::game->maps->layers.begin(); it != app::game->maps->layers.end(); ++it) {

        auto layer = *it;

        layer.erase(
            std::remove_if(layer.begin(), layer.end(), [&](auto t) { 
                return strcmp(t->type, "tile") == 0 && t->ID == key; 
            }), layer.end());
    }

    app::game->entities.erase(
        std::remove_if(app::game->entities.begin(), app::game->entities.end(), [&](auto t) { 
            return strcmp(t->type, "tile") == 0 && t->ID == key; }), 
                app::game->entities.end());

    std::cout << "Tilemap: layer " + key + " cleared.\n";

}


//------------------------------------- clear all layers


void MapManager::ClearMap() 
{

    app::game->maps->layers.clear();

    app::game->entities.erase(
        std::remove_if(app::game->entities.begin(), app::game->entities.end(), [](auto t) { 
            return strcmp(t->type, "tile") == 0; }), 
                app::game->entities.end());

    std::cout << "Tilemap: layers cleared.\n";

}
