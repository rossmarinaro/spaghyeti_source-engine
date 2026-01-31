#include <bitset>
#include <sstream>

#ifndef __EMSCRIPTEN__
    #include <bits/stdc++.h>
#endif

#include "../../../build/sdk/include/tilemap.h"
#include "../../../build/sdk/include/app.h"


const bool MapManager::CreateLayer (
    const char* texture_key,
    const char* data_key,
    uint32_t mapWidth,
    uint32_t mapHeight,
    uint32_t tileWidth,
    uint32_t tileHeight,
    uint32_t depth,
    int index,
    float posX,
    float posY,
    float rotation,
    float scaleX,
    float scaleY,
    float scrollFactorX,
    float scrollFactorY,
    const std::string& shaderKey
)
{
    auto data = System::Resources::Manager::ParseMapData(data_key, index);

    if (!data.size()) {                                       
        LOG("Tilemap: layer data not found. Expected " + (std::string)data_key);
        return false;
    }

    std::vector<std::shared_ptr<Sprite>> layer; 

    layer.reserve(10000);

    std::stringstream ss; 
    std::string line; 

    //remove commas from array before parsing

    for (int i = 0; i < data.size(); i++) 
        ss << data[i]; 

    data.clear();

    while(getline(ss, line, ','))
       data.emplace_back(line);

    for (int y = 0; y < mapHeight; ++y)
        for (int x = 0; x < mapWidth; ++x) 
        {
            if ((data.begin() + (x + y * mapWidth)) == data.end()) {
                LOG("Tilemap: index overflow, truncating map.");
                return false;
            }

            //convert string to int at index

            int tileType = atoi(data[x + y * mapWidth].c_str());

            //skip if no tile

            if (tileType != -1) 
            {
                bool flipX = false, 
                     flipY = false,
                     diag = false;

                //number larger than total tiles indicates flip or rotation
                //convert to binary to extract bit flags

                if ((tileType < -1) || (tileType > mapWidth * mapHeight)) 
                { 
                    //flags: 1=flipX, 2=flipY, 3=diagonal

                    std::bitset<32> bin(tileType);

                    std::string flags = bin.to_string().substr(0, 3),
                                bin_reset = bin.to_string();

                    bin_reset[0] = '0'; 
                    bin_reset[1] = '0';
                    bin_reset[2] = '0';
                    
                    tileType = System::Utils::BinToDec(atoi(bin_reset.c_str()));       
                                
                    flipX = static_cast<std::string>(flags).substr(0, 1) == "1";
                    flipY = static_cast<std::string>(flags).substr(1, 1) == "1";
                    diag = static_cast<std::string>(flags).substr(2, 1) == "1";
                }

                //create tilesprite entity

                const auto tile = System::Game::CreateTileSprite(texture_key, (x * tileWidth) + posX, (y * tileHeight) + posY, tileType); 
            
                tile->SetName((std::string)data_key);
                tile->SetDepth(depth); 
                tile->SetFlip(flipX, flipY);  
                tile->SetRotation(rotation); 
                tile->SetScale(scaleX, scaleY); 
                tile->SetScrollFactor({ scrollFactorX, scrollFactorY });
                tile->SetCull(true);

                if (shaderKey.length())
                    tile->SetShader(shaderKey);

                if (diag)
                    tile->SetRotation(rotation + 90);  

                //add layer to stack

                layer.emplace_back(tile);
            }

        }

    System::Application::game->maps->layers.emplace_back(layer);

    LOG("Tilemap: Initialized layer with texture key: " + (std::string)texture_key + " data key: " + (std::string)data_key);

    return true;
}


//------------------------------------- remove layer


void MapManager::RemoveLayer(const std::string& key) 
{
    for (auto it = System::Application::game->maps->layers.begin(); it != System::Application::game->maps->layers.end(); ++it) {

        auto layer = *it;

        layer.erase(
            std::remove_if(layer.begin(), layer.end(), [&](auto t) { 
                return t->GetType() == Entity::TILE && t->name == key; 
            }), layer.end());
    }

    System::Game::GetScene()->entities.erase(
        std::remove_if(System::Game::GetScene()->entities.begin(), System::Game::GetScene()->entities.end(), [&](auto t) { 
            return t->GetType() == Entity::TILE && t->name == key; }), 
                System::Game::GetScene()->entities.end());

    LOG("Tilemap: layer " + key + " cleared.");
}


//------------------------------------- clear all layers


void MapManager::ClearMap() 
{
    System::Application::game->maps->layers.clear();

    System::Game::GetScene()->entities.erase(
        std::remove_if(System::Game::GetScene()->entities.begin(), System::Game::GetScene()->entities.end(), [](auto t) { 
            return t->GetType() == Entity::TILE; }), 
                System::Game::GetScene()->entities.end());

    LOG("Tilemap: layers cleared.");

}


