#include <bitset>
#include <sstream>

#ifndef __EMSCRIPTEN__
    #include <bits/stdc++.h>
#endif

#include "../../../build/sdk/include/game.h"
#include "../../../build/sdk/include/app.h"
#include "../../vendors/UUID.hpp"

const std::string System::Game::CreateTileLayer(
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
        return "";
    }
    
    const std::string ID = UUID::generate_uuid(); 

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
                return "";
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
                tile->SetData(ID, true);

                if (shaderKey.length())
                    tile->SetShader(shaderKey);

                if (diag)
                    tile->SetRotation(rotation + 90.0f);  
            }
        }

    LOG("Tilemap: Initialized layer: " + ID + " with texture key: " + (std::string)texture_key + " and data key: " + (std::string)data_key);

    return ID;
}


//------------------------------------- remove layer


void System::Game::RemoveTileLayer(const std::string& ID) 
{
    //remove tiles in layer from entity render queue

    const auto entities = System::Game::GetScene()->entities;

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        const auto tile = *it;
        if (tile->GetData<bool>(ID) && tile->GetType() == Entity::TILE) 
            System::Game::DestroyEntity(tile); 
    }

    LOG("Tilemap: layer " + ID + " cleared.");
}

//----------------------------- tile


std::shared_ptr<Sprite> System::Game::CreateTileSprite(const std::string& key, float x, float y, int frame)
{
    const auto ts = std::make_shared<Sprite>(key, x, y, false, true);

    GetScene()->entities.emplace_back(ts);

    ts->ReadSpritesheetData();
    ts->SetFrame(frame);

    return ts;
}






