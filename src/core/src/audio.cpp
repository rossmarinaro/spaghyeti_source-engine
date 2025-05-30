#ifdef PRINT_LOGS 
    #define MA_DEBUG_OUTPUT
#endif

#define MINIAUDIO_IMPLEMENTATION
#include "../../vendors/miniaudio.h"

#include <stdio.h>
#include <thread>

#include "../../../build/sdk/include/audio.h"
#include "../../../build/sdk/include/manager.h" 
#include "../../../build/sdk/include/utils.h" 


#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000
#define MA_NO_ENCODING
#define MA_NO_NODE_GRAPH
#define MA_NO_GENERATION

static ma_device _sound_device;
static ma_device _music_device;

static ma_decoder _music_decoder; 
static ma_decoder _sound_decoder;


static void _ReadFrames(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{

    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData; 

    if (pDecoder == NULL) 
    {
        LOG("Audio: there was a problem decoding audio.");

        return;
    }
 
    ma_data_source_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}


//---------------------------------------------


static void _ProcessAudio(const char* key, bool loop, float volume)
{

    std::string filetype = "none";
    const auto filepath = System::Resources::Manager::GetFilePath(key);      

    //file asset or raw char data

    ma_result result;
    
    if (filepath)  {
        filetype = "filepath";
        result = ma_decoder_init_file((*filepath).c_str(), NULL, loop ? &_music_decoder : &_sound_decoder);
    }

    else {
        filetype = "binary";
        const auto data = System::Resources::Manager::GetResource(key);
        if (data)
            result = ma_decoder_init_memory(data->array_buffer, data->byte_length, NULL, loop ? &_music_decoder : &_sound_decoder);
    }

    if (result != MA_SUCCESS) {
        LOG("Audio: failed to init audio: " + filetype);
        return;
    }

    //loop audio

    if (loop) {
        System::Audio::musicPlaying = true;
        ma_data_source_set_looping(loop ? &_music_decoder : &_sound_decoder, MA_TRUE);    
    } 

    if (ma_device_get_state(loop ? &_music_device : &_sound_device) == ma_device_state_started) 
        return;

    //volume

    System::Audio::setVolume(volume);

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);

    deviceConfig.playback.format   = loop ? _music_decoder.outputFormat : _sound_decoder.outputFormat;      
    deviceConfig.playback.channels = loop ? _music_decoder.outputChannels : _sound_decoder.outputChannels;     
    deviceConfig.sampleRate        = loop ? _music_decoder.outputSampleRate : _sound_decoder.outputSampleRate;     
    deviceConfig.dataCallback      = _ReadFrames;
    deviceConfig.pUserData         = loop ? &_music_decoder : &_sound_decoder;

    if (ma_device_init(NULL, &deviceConfig, loop ? &_music_device : &_sound_device) != MA_SUCCESS) {

        LOG("Audio: failed to open playback device.");
        ma_decoder_uninit(loop ? &_music_decoder : &_sound_decoder);
        return;
    }

    if (ma_device_start(loop ? &_music_device : &_sound_device) != MA_SUCCESS) {
        LOG("Audio: failed to start playback device.");
        ma_device_uninit(loop ? &_music_device : &_sound_device);
        ma_decoder_uninit(loop ? &_music_decoder : &_sound_decoder);
        return;
    }

    #ifndef __EMSCRIPTEN__

        getchar();

        ma_device_uninit(loop ? &_music_device : &_sound_device);
        ma_decoder_uninit(loop ? &_music_decoder : &_sound_decoder);

    #endif

}


//-------------------------------------------


void System::Audio::play(const char* key, bool loop, float volume) {    
    #ifdef __EMSCRIPTEN__
        _ProcessAudio(key, loop, volume);
    #else   
       std::thread(&_ProcessAudio, key, loop, volume).detach(); 
    #endif
}



//----------------------------------------


void System::Audio::stop() {
    if (ma_device_get_state(&_music_device) == ma_device_state_started) 
        ma_data_source_set_looping(&_music_decoder, MA_FALSE);

    if (ma_device_get_state(&_sound_device) == ma_device_state_started) 
        ma_data_source_set_looping(&_sound_decoder, MA_FALSE);

    LOG("Audio: audio stopped.");

    musicPlaying = false;
}


//----------------------------------------


void System::Audio::setVolume(float volume) {
    if (ma_device_get_state(&_music_device) == ma_device_state_started)
        ma_device_set_master_volume(&_music_device, volume);

    if (ma_device_get_state(&_sound_device) == ma_device_state_started)
        ma_device_set_master_volume(&_sound_device, volume);
}