#include <bitset>
#include <sstream>

#ifndef __EMSCRIPTEN__
    #include <bits/stdc++.h>
#endif
//#if USE_JSON == 1
	#include "../../vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
//#endif

#include "../../../build/sdk/include/game.h"
#include "../../../build/sdk/include/app.h"
#include "../../vendors/UUID.hpp"

using namespace System;


//---------------------------------------------- construct tilemap on the fly from a loaded map json file


void Game::CreateTilemapFromJSON(const std::string& key) 
{
    const auto pathPointer = Resources::Manager::GetFilePath(key); 

    if (!pathPointer) {
        LOG("error");
        return;
    }

    const std::string path = *pathPointer;
    
    std::ifstream in(path);
    json data = json::parse(in);

    unsigned int index = 0;

    //bodies

    if (data.contains("layers") && data["layers"].size()) 
        for (const auto& layer : data["layers"]) 
        {
            if (static_cast<std::string>(layer["type"]) == "tilelayer")
            {
                //assign corresponding texture key

                std::string textureKey = "";

                uint32_t tilewidth, tileHeight;

                if (data.contains("tilesets") && data["tilesets"].size()) 
                {   
                    // if (index /* <= */< data["tilesets"].size()) 
                    // {
                      
                    //     std::string p = static_cast<std::string>(data["tilesets"][index]["image"]),
                    //                 ext = Utils::GetFileExtension(p);
                    //     textureKey = static_cast<std::string>(data["tilesets"][index]["name"]) + ext; 
                    // }
                    // else {
                        std::string p = static_cast<std::string>(data["tilesets"][0]["image"]),
                                    ext = Utils::GetFileExtension(p);
                        textureKey = static_cast<std::string>(data["tilesets"][0]["name"]) + ext;  

                        tilewidth = data["tilesets"][0].contains("tilewidth") ? static_cast<uint32_t>(data["tilesets"][0]["tilewidth"]) : 0,
                        tileHeight = data["tilesets"][0].contains("tileheight") ? static_cast<uint32_t>(data["tilesets"][0]["tileheight"]) : 0;
                    //}
                }

                const uint32_t id = layer.contains("id") ? static_cast<int>(layer["id"]) : 0,
                               width = layer.contains("width") ? static_cast<uint32_t>(layer["width"]) : 0,
                               height = layer.contains("height") ? static_cast<uint32_t>(layer["height"]) : 0,
                               depth = layer.contains("z") ? static_cast<uint32_t>(layer["z"]) : index;

                const float x = layer.contains("x") ? static_cast<float>(layer["x"]) : 0.0f,
                            y = layer.contains("y") ? static_cast<float>(layer["y"]) : 0.0f,
                            scrollFactorX = layer.contains("parallaxx") ? static_cast<float>(layer["parallaxx"]) : 1.0f,
                            scrollFactorY = layer.contains("parallaxy") ? static_cast<float>(layer["parallaxy"]) : 1.0f;

                //create the tile layer

                CreateTileLayer(
                    id,
                    textureKey.c_str(),
                    key.c_str(),
                    width,
                    height,
                    tilewidth,
                    tileHeight,
                    depth,
                    index,
                    x,
                    y,
                    scrollFactorX,
                    scrollFactorY
                );   

                index++;
            }

            // if (layer.contains("objects")) 
            //     for (auto& body : layer["objects"]) 
            //         Physics::CreateBody(Physics::Body::Type::STATIC, 
            //             static_cast<float>(body["x"]), 
            //             static_cast<float>(body["x"]), 
            //             static_cast<float>(body["width"]), 
            //             static_cast<float>(body["height"])
            //         );

   
        } ;LOG(22);
}


//----------------------------------------------


void Game::CreateTileLayer(
    int layer,
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
    float scrollFactorX,
    float scrollFactorY,
    const std::string& shaderKey
)
{ LOG(index);
    auto data = Resources::Manager::ParseMapData(data_key, index);

    if (!data.size()) {                                       
        LOG("Tilemap: layer data not found. Expected " + (std::string)data_key);
        return;
    }
    
    //int layer;//const std::string ID = UUID::generate_uuid(); 

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
                LOG("Tilemap: Error - data overflow.");
                return;
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
                    
                    tileType = Utils::BinToDec(atoi(bin_reset.c_str()));       
                                
                    flipX = static_cast<std::string>(flags).substr(0, 1) == "1";
                    flipY = static_cast<std::string>(flags).substr(1, 1) == "1";
                    diag = static_cast<std::string>(flags).substr(2, 1) == "1";  
                }

                //create tilesprite entity
//LOG("x: "+std::to_string(x)+", y:"+std::to_string(y)+", tileW: "+std::to_string(tileWidth)+", tileH: "+std::to_string(tileHeight));
                const auto tile = CreateTileSprite(texture_key, (x * tileWidth) + posX, (y * tileHeight) + posY, tileType); 
       
                tile->SetName((std::string)data_key);
                tile->SetDepth(depth); 
                tile->SetFlip(flipX, flipY);   
                tile->SetScrollFactor({ scrollFactorX, scrollFactorY });
                tile->SetCull(true);
                //tile->SetData(data_key, true);
                tile->SetData("layer", layer);

                if (shaderKey.length())
                    tile->SetShader(shaderKey);

                if (diag)
                    tile->SetRotation(90.0f);  
            }
        }
 
    LOG("Tilemap: Initialized layer: " + std::to_string(layer) + " with texture key: " + (std::string)texture_key + " and data key: " + (std::string)data_key);

}

//------------------------------------- get map


std::vector<std::shared_ptr<Sprite>> Game::GetTileMap(const std::string& key) 
{
    std::vector<std::shared_ptr<Sprite>> tiles;
    const auto entities = Game::GetScene()->entities;

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        const auto entity = *it;
        if (entity->GetData<bool>(key) && entity->GetType() == Entity::TILE) {
           const auto tile = std::static_pointer_cast<Sprite>(entity);
           tiles.emplace_back(tile);
        }
    }

    return tiles;
}


//------------------------------------- get layer


std::vector<std::shared_ptr<Sprite>> Game::GetTileLayer(int layer) 
{
    std::vector<std::shared_ptr<Sprite>> tiles;
    const auto entities = Game::GetScene()->entities;

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        const auto entity = *it;
        if (entity->GetData<int>("layer") == layer && entity->GetType() == Entity::TILE) {
            const auto tile = std::static_pointer_cast<Sprite>(entity);
            tiles.emplace_back(tile);
        }
    }

    return tiles;
}

//------------------------------------- remove map


void Game::RemoveTilemap(const std::string& key) 
{
    //remove tiles in layer from entity render queue

    const auto entities = Game::GetScene()->entities;

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        const auto tile = *it;
        if (tile->GetData<bool>(key) && tile->GetType() == Entity::TILE) 
            Game::DestroyEntity(tile); 
    }

    LOG("Tilemap: layer " + key + " removed.");
}


//------------------------------------- remove layer


void Game::RemoveTileLayer(int layer) 
{
    //remove tiles in layer from entity render queue

    const auto entities = Game::GetScene()->entities;

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        const auto tile = *it;
        if (tile->GetData<int>("layer") == layer && tile->GetType() == Entity::TILE) 
            Game::DestroyEntity(tile); 
    }

    LOG("Tilemap: layer " + std::to_string(layer) + " removed.");
}

//----------------------------- tile


std::shared_ptr<Sprite> Game::CreateTileSprite(const std::string& key, float x, float y, int frame)
{
    const auto ts = std::make_shared<Sprite>(key, x, y, false, true);

    GetScene()->entities.emplace_back(ts);

    ts->ReadSpritesheetData();
    ts->SetFrame(frame);

    return ts;
}






