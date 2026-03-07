#include "./events.h"
#include "../editor.h"

using namespace editor;

//------------------------------- encapsulate json data in array of temporary "instances"


void EventListener::StartSession(std::ifstream& JSON) 
{
    actionsInit = false;
    eventCount = 0;
    actionsCount = 0;

    sessionData << "{ \"instances\": [" << "\n";
    sessionData << JSON.rdbuf();
    sessionData << "]}" << "\n";
}


//------------------------------- add new session instance to json array


void EventListener::UpdateSession() 
{
    json currentSceneData;
    Serialize(currentSceneData, false);
    
    //check instance limit / embed new session instance
    
    json data = json::parse(sessionData);

    if (data.contains("instances"))
    {
        if (!actionsInit) //initial seed
            actionsCount++;

        actionsInit = true;
        data["instances"].push_back(currentSceneData);

        //remove first element if reached limit

        if (data["instances"].size() >= ACTIONS_LIMIT) 
            data["instances"].erase(data["instances"].front());
        else 
            eventCount++;     
    }

    //write new session instance data to stream

    std::stringstream().swap(sessionData);
    sessionData << data.dump(); 
}


//-------------------------------


void EventListener::ApplyState(bool increment) 
{
    //toggle undo/redo of actions
    
    if (increment) {
        if (actionsCount < ACTIONS_LIMIT && actionsCount < eventCount) 
            actionsCount++;
        else 
            return;
    }
    else {
        if (actionsCount > 0) 
            actionsCount--;
        else 
            return;
    }
    
    //set stream read position, load current session's instance index

    try {
        sessionData.seekg(0);

        json data = json::parse(sessionData);

        const bool arrayInBounds = data.contains("instances") && 
                                   data["instances"].is_array() && 
                                   actionsCount <= data["instances"].size() - 1;
        if (arrayInBounds) 
        {
            Editor::Get()->Reset(false);

            json currentSessionData = data["instances"][actionsCount]; 
            Deserialize(currentSessionData, true); 
        } 
    }

    catch(json::exception& err) {
        Editor::Log("error reading data: " + (std::string)err.what());
    }
}
