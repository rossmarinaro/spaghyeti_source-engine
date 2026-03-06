#include "./events.h"
#include "../editor.h"

using namespace editor;

//------------------------------- encapsulate json data in array of temporary "instances"


void EventListener::StartSession(std::ifstream& JSON) 
{
    actionsInit = false;
    eventCount = 0;
    actionsCount = 0;
 
    std::string line;

    std::ofstream out_file(Editor::sessionFilePath);

    out_file << "{ \"instances\": [" << "\n";
    out_file << JSON.rdbuf();
    out_file << "]}" << "\n";
    out_file.close();
}


//------------------------------- add new session instance to json array


void EventListener::UpdateSession() 
{
    json JSON;
    Serialize(JSON);

    std::ifstream session_src(Editor::sessionFilePath);
    
    if (session_src.good()) 
    {  
        //check instance limit / embed new session instance

        json parsed_data = json::parse(session_src);

        if (parsed_data.contains("instances"))
        {
            if (!actionsInit) //initial seed
                actionsCount++;

            actionsInit = true;
            parsed_data["instances"].push_back(JSON);

            //remove first element if reached limit

            if (parsed_data["instances"].size() >= ACTIONS_LIMIT) 
                parsed_data["instances"].erase(parsed_data["instances"].front());
            else 
                eventCount++;
        }

        session_src.close();

        //write new session instance data to stream

        std::ofstream session_out(Editor::sessionFilePath);
        session_out << parsed_data.dump();
        session_out.close();
    }

}


//-------------------------------


void EventListener::ApplyState(bool increment) 
{
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
    
    json JSON;
    std::ifstream session_src(Editor::sessionFilePath);

    //load current session's instance index

    Editor::Get()->Reset();
    Deserialize(session_src, actionsCount);

    session_src.close();
}
