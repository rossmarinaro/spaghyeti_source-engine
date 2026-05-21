#include <bitset>
#include <sstream>

#ifndef __EMSCRIPTEN__
    #include <bits/stdc++.h>
#endif
#if USE_JSON == 1
	#include "../../vendors/nlohmann/json.hpp"
	using json = nlohmann::json;
#endif

#include "../../../build/sdk/include/game.h"
#include "../../../build/sdk/include/app.h"
#include "../../vendors/UUID.hpp"

using namespace System;


//---------------------------------------------- construct tilemap on the fly from a loaded map json file

Scene::Tilemap Game::CreateTilemapFromJSON(const std::string& key) 
{
    #if USE_JSON == 1

    //read / parse file data

    const auto pathPointer = Resources::Manager::GetFilePath(key); 

    if (!pathPointer) {
        LOG("Tilemap: Cannot create map - data filepath not found.");
        return {};
    }

    const std::string path = *pathPointer;
    
    std::ifstream in(path);
    json data = json::parse(in);

    if (!data.contains("layers") && !data.contains("width") && !data.contains("height") && !data.contains("tilewidth") && !data.contains("tileheight")) {
        LOG("Tilemap: Cannot create map - JSON must have following values [layers, width, height, tilewidth, tileheight].");
        return {};
    }

    unsigned int index = 0,
                 map_width = 0, 
                 map_height = 0,
                 tile_width = 0,
                 tile_height = 0;

    //containers to populate and return

    std::vector<Scene::TilemapLayer> layers;
    std::vector<std::shared_ptr<Physics::Body>> bodies;

    if (data.contains("width"))
        map_width = data["width"];

    if (data.contains("height"))
        map_height = data["height"];

    if (data.contains("tilewidth"))
        tile_width = data["tilewidth"];

    if (data.contains("tileheight"))
        tile_height = data["tileheight"];

    //iterate over layers key in json

    for (const auto& layer : data["layers"]) 
    {
        if (static_cast<std::string>(layer["type"]) == "tilelayer")
        {
            //assign corresponding texture key

            std::string textureKey = "";
            uint32_t columns = 0;

            if (data.contains("tilesets") && data["tilesets"].size()) 
            {   
                const auto applyTexture = [&] (int tileset) -> void {

                    std::string p = static_cast<std::string>(data["tilesets"][tileset]["image"]),
                                ext = Utils::GetFileExtension(p);
                    textureKey = static_cast<std::string>(data["tilesets"][tileset]["name"]) + ext;  
                    columns = data["tilesets"][tileset]["columns"];
                };

                applyTexture(index < data["tilesets"].size() ? index : 0);
            }

            const uint32_t ID = layer.contains("id") ? static_cast<uint32_t>(layer["id"]) : index,
                           depth = layer.contains("z") ? static_cast<uint32_t>(layer["z"]) : index;

            const float x = layer.contains("x") ? static_cast<float>(layer["x"]) : 0.0f,
                        y = layer.contains("y") ? static_cast<float>(layer["y"]) : 0.0f,
                        scrollFactorX = layer.contains("parallaxx") ? static_cast<float>(layer["parallaxx"]) : 1.0f,
                        scrollFactorY = layer.contains("parallaxy") ? static_cast<float>(layer["parallaxy"]) : 1.0f;

            //create the tile layer

            const auto tileLayer = CreateTileLayer(ID, textureKey.c_str(), key.c_str(), columns, map_width, map_height, tile_width, tile_height, depth, index, x, y, scrollFactorX, scrollFactorY);  
            
            layers.emplace_back(tileLayer);

            index++;
        }

        //bodies

        if (layer.contains("objects")) 
            for (const auto& body : layer["objects"]) 
            {
                const float x = body.contains("x") ? static_cast<float>(body["x"]) : 0.0f,
                            y = body.contains("y") ? static_cast<float>(body["y"]) : 0.0f,
                            width = body.contains("width") ? static_cast<float>(body["width"]) : 0.0f,
                            height = body.contains("height") ? static_cast<float>(body["height"]) : 0.0f;

                const auto pb = Physics::CreateBody(Physics::Body::Type::STATIC, x + (width / 2), y + (height / 2), width / 2, height / 2);

                bodies.emplace_back(pb);
            }
    } 

    return { key, path, map_width, map_height, tile_width, tile_height, layers, bodies };

    #else
        LOG("Tilemap: Cannot create tilemap from JSON - USE_JSON=0");
    #endif

    return {};
}


//----------------------------------------------


Scene::TilemapLayer Game::CreateTileLayer(
    int ID,
    const char* texture_key,
    const char* data_key,
    uint32_t columns,
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
{ 
    Scene::TilemapLayer layer;
    
    layer.ID = ID;
    layer.textureKey = texture_key;
    layer.dataKey = data_key;
    layer.dataPath = *Resources::Manager::GetFilePath(data_key);
    layer.key = UUID::generate_uuid();
    layer.depth = depth;
    layer.shader = shaderKey;
    layer.scrollFactorX = scrollFactorX;
    layer.scrollFactorY = scrollFactorY;
    layer.columns = columns;

    auto data = Resources::Manager::ParseMapData(data_key, index);

    if (!data.size()) {                                       
        LOG("Tilemap: layer data not found. Expected " + (std::string)data_key);
        return layer;
    }

    std::stringstream ss; 
    std::string line; 

    //remove commas from array before parsing

    for (int i = 0; i < data.size(); i++) 
        ss << data[i]; 

    data.clear();

    while(getline(ss, line, ','))
       data.emplace_back(line);

    //create tilesprites and add to entity queue;

    for (int y = 0; y < mapHeight; ++y)
        for (int x = 0; x < mapWidth; ++x) 
        {
            if ((data.begin() + (x + y * mapWidth)) == data.end()) {
                LOG("Tilemap: Error - data overflow.");
                return layer;
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

                const auto tile = CreateTileSprite(texture_key, (x * tileWidth) + posX, (y * tileHeight) + posY, tileType); 
       
                tile->SetName((std::string)data_key);
                tile->SetDepth(depth); 
                tile->SetFlip(flipX, flipY);   
                tile->SetScrollFactor({ scrollFactorX, scrollFactorY });
                tile->SetCull(true);
                tile->SetData(data_key, true);
                tile->SetData("layer id", layer.ID);
                tile->SetData("layer key", layer.key);

                if (shaderKey.length())
                    tile->SetShader(shaderKey);

                if (diag)
                    tile->SetRotation(90.0f);  
            }
        }
 
    LOG("Tilemap: Initialized layer: " + layer.key + " with texture key: " + (std::string)texture_key + " and data key: " + (std::string)data_key);

    return layer;

}

//------------------------------------- get map


std::vector<std::shared_ptr<Sprite>> Game::GetTileMapSprites(const std::string& key) 
{
    std::vector<std::shared_ptr<Sprite>> tilesprites;
    const auto entities = GetScene()->entities;

    for (auto it = entities.begin(); it != entities.end(); ++it) 
    {
        const auto entity = *it;

        if (entity->GetType() != Entity::TILE)
            continue;

        //if entity has the dataKey boolean data, add to vector
            
        if (entity->GetData<bool>(key)) {
           const auto tile = std::static_pointer_cast<Sprite>(entity);
           tilesprites.emplace_back(tile);
        }
    }

    return tilesprites;
}


//------------------------------------- get layer


std::vector<std::shared_ptr<Sprite>> Game::GetTileLayerSprites(const std::string& mapKey, int layerID) 
{
    std::vector<std::shared_ptr<Sprite>> tilesprites;
    
    for (const auto& tilesprite : GetTileMapSprites(mapKey)) 
        if (tilesprite->GetData<int>("layer id") == layerID) 
            tilesprites.emplace_back(tilesprite);
        
    return tilesprites;
}



//------------------------------------- remove map


void Game::RemoveTilemap(const std::string& key) 
{
    for (const auto& tilesprite : GetTileMapSprites(key)) 
        DestroyEntity(tilesprite);

    LOG("Tilemap: layer " + key + " removed.");
}


//------------------------------------- remove layer


void Game::RemoveTileLayer(const std::string& mapKey, int layerID) 
{
    for (const auto& tilesprite : GetTileMapSprites(mapKey)) 
        if (tilesprite->GetData<int>("layer id") == layerID) 
            DestroyEntity(tilesprite); 

    LOG("Tilemap: layer " + std::to_string(layerID) + " removed from map: " +  mapKey + ".");
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






