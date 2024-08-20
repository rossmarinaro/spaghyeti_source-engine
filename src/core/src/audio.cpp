#ifdef PRINT_LOGS 
    #define MA_DEBUG_OUTPUT
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "../../../build/sdk/include/vendors/miniaudio.h"

#include <stdio.h>
#include <thread>

#include "../../../build/sdk/include/audio.h"
#include "../../../build/sdk/include/time.h"
#include "../../../build/sdk/include/manager.h"

#if DEVELOPMENT == 1
    #include "../../../build/sdk/include/utils.h"
#endif

#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000


ma_device sound_device;
ma_device music_device;

ma_decoder music_decoder; 
ma_decoder sound_decoder;

//-----------------------------------------


void read_frames(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{

    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData; 

    if (pDecoder == NULL) 
    {
        #if DEVELOPMENT == 1
            LOG("Audio: There was a problem decoding audio.");
        #endif

        return;
    }
 
    ma_data_source_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}



//---------------------------------------------


void process_audio(const char* key, bool loop, float volume)
{

    ma_device_config deviceConfig; 

    const char* filepath = System::Resources::Manager::GetFilePath(key);      

    //file asset or raw char data

    ma_result result = strcmp(filepath, "not found") != 0 ? 
        ma_decoder_init_file(filepath, NULL, loop ? &music_decoder : &sound_decoder) :
        ma_decoder_init_memory(System::Resources::Manager::GetRawData(key), System::Resources::Manager::GetSizeOfRawAudio(key), NULL, loop ? &music_decoder : &sound_decoder);

    if (result != MA_SUCCESS) {

        #if DEVELOPMENT == 1
            LOG("Audio: Failed to init sound.");
        #endif

        return;
    }

    //loop audio

    if (loop) {
        System::Audio::musicPlaying = true;
        ma_data_source_set_looping(loop ? &music_decoder : &sound_decoder, MA_TRUE);    
    } 

    if (ma_device_get_state(loop ? &music_device : &sound_device) == ma_device_state_started) 
        return;

    //volume
        System::Audio::setVolume(volume);

    deviceConfig = ma_device_config_init(ma_device_type_playback);

    deviceConfig.playback.format   = loop ? music_decoder.outputFormat : sound_decoder.outputFormat;      
    deviceConfig.playback.channels = loop ? music_decoder.outputChannels : sound_decoder.outputChannels;     
    deviceConfig.sampleRate        = loop ? music_decoder.outputSampleRate : sound_decoder.outputSampleRate;     
    deviceConfig.dataCallback      = read_frames;
    deviceConfig.pUserData         = loop ? &music_decoder : &sound_decoder;


    if (ma_device_init(NULL, &deviceConfig, loop ? &music_device : &sound_device) != MA_SUCCESS) 
    {

        #if DEVELOPMENT == 1
            LOG("Audio: Failed to open playback device.");
        #endif
        
        ma_decoder_uninit(loop ? &music_decoder : &sound_decoder);
        
        return;
    }

    if (ma_device_start(loop ? &music_device : &sound_device) != MA_SUCCESS) 
    {

        #if DEVELOPMENT == 1
            LOG("Audio: Failed to start playback device.");
        #endif

        ma_device_uninit(loop ? &music_device : &sound_device);
        ma_decoder_uninit(loop ? &music_decoder : &sound_decoder);
        
        return;
    }

    #ifndef __EMSCRIPTEN__

        getchar();

        ma_device_uninit(loop ? &music_device : &sound_device);
        ma_decoder_uninit(loop ? &music_decoder : &sound_decoder);

    #endif

}


//-------------------------------------------


void System::Audio::play(const char* key, bool loop, float volume)
{ 
    
    #ifdef __EMSCRIPTEN__
        process_audio(key, loop, volume);
    #else   
       std::thread(&process_audio, key, loop, volume).detach(); 
    #endif
}



//----------------------------------------


void System::Audio::stop()
{
    
    if (ma_device_get_state(&music_device) == ma_device_state_started) 
        ma_data_source_set_looping(&music_decoder, MA_FALSE);

    if (ma_device_get_state(&sound_device) == ma_device_state_started) 
        ma_data_source_set_looping(&sound_decoder, MA_FALSE);

    #if DEVELOPMENT == 1
        LOG("Audio: audio stopped.");
    #endif

    musicPlaying = false;
}


//----------------------------------------


void System::Audio::setVolume(float volume) 
{

    if (ma_device_get_state(&music_device) == ma_device_state_started)
        ma_device_set_master_volume(&music_device, volume);

    if (ma_device_get_state(&sound_device) == ma_device_state_started)
        ma_device_set_master_volume(&sound_device, volume);
}