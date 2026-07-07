#ifdef PRINT_LOGS 
    #define MA_DEBUG_OUTPUT
#endif

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_RESOURCE_MANAGER //disable lib threads
#include "../../vendors/miniaudio.h"

#include <stdio.h>
#include <thread>
#include <atomic>
#include <cstring>
#include "../../../build/sdk/include/audio.h"
#include "../../../build/sdk/include/manager.h" 
#include "../../../build/sdk/include/utils.h" 

#define MA_NO_ENCODING
#define MA_NO_NODE_GRAPH
#define MA_NO_GENERATION
#define MAX_VOICES 8

using namespace System;

struct SoundBuffer {
    float* pData = nullptr;
    ma_uint32 totalFrames = 0;
    ma_uint32 channels = 0;
};

struct AudioVoice {
    SoundBuffer* pBuffer = nullptr;
    std::atomic<bool> isActive{false};
    std::atomic<bool> shouldLoop{false};
    ma_uint32 currentFrame = 0;
};

static ma_uint32 _sampleRate;
static ma_device _sound_device;
static AudioVoice _voices[MAX_VOICES];
static std::vector<SoundBuffer> _soundBuffers;
static std::map<std::string, int> _loadedSounds; 


//------------------------------------------


static int GetSoundByID(const std::string& key) {
    const auto it = _loadedSounds.find(key);
    return it != _loadedSounds.end() ? it->second : -1;
}


//------------------------------------------


static void ReadFrames(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{//return;
    float* pOutFloat = (float*)pOutput;

    ma_uint32 channels = pDevice->playback.channels,
              totalSamples = frameCount * channels;

    //zero output buffer = no silence
    for (ma_uint32 i = 0; i < totalSamples; ++i) 
        pOutFloat[i] = 0.0f;

    for (int i = 0; i < MAX_VOICES; ++i)
    {
        if (!_voices[i].isActive.load(std::memory_order_relaxed))
            continue;

        SoundBuffer* sound = _voices[i].pBuffer;

        if (!sound || !sound->pData) {
            _voices[i].isActive.store(false, std::memory_order_relaxed);
            continue;
        }

        ma_uint32 voiceFrame = _voices[i].currentFrame;

        if (sound->channels != channels)
            continue;

        for (ma_uint32 f = 0; f < frameCount; ++f) 
        {
            if (voiceFrame >= sound->totalFrames) {
                if (_voices[i].shouldLoop.load(std::memory_order_relaxed))
                    voiceFrame = 0;
                else {
                    _voices[i].isActive.store(false, std::memory_order_relaxed);
                    break;
                }
            }

            for (ma_uint32 c = 0; c < channels; ++c) {
                ma_uint32 outIndex = (f * channels) + c;
                ma_uint32 sampleIndex = (voiceFrame * sound->channels) + c;
                pOutFloat[outIndex] += sound->pData[sampleIndex];
            }

            voiceFrame++;
        }

        _voices[i].currentFrame = voiceFrame;
    }

    (void)pInput;
}


//--------------------------


void Audio::Init()
{
    for (int i = 0; i < MAX_VOICES; ++i) {
        _voices[i].pBuffer = nullptr;
        _voices[i].currentFrame = 0;
        _voices[i].shouldLoop.store(false);
        _voices[i].isActive.store(false);
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);

    deviceConfig.playback.format   = ma_format_f32;    
    deviceConfig.playback.channels = 2;     
    deviceConfig.sampleRate        = 44100; //0 /* hardware sample rate */
    deviceConfig.dataCallback      = ReadFrames;

    if (ma_device_init(NULL, &deviceConfig, &_sound_device) != MA_SUCCESS) {
        LOG("Audio: failed to open playback device.");
        return;
    }

    _sampleRate = deviceConfig.sampleRate;

    if (ma_device_start(&_sound_device) != MA_SUCCESS) {
        LOG("Audio: failed to start playback device.");
        ma_device_uninit(&_sound_device);
        return;
    }

    LOG("Audio: sound device initialized.");
}


//--------------------------


void Audio::ShutDown()
{
    ma_device_uninit(&_sound_device);

    _loadedSounds.clear();
    _soundBuffers.clear();

    LOG("Audio: audio shut down.");
}

//--------------------------


void Audio::Load(const std::string& key)
{
    std::string filetype = "none";
    const auto filepath = System::Resources::Manager::GetFilePath(key);      

    ma_result result;
    ma_decoder decoder;
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 2, _sampleRate);
    
    if (filepath) {
        filetype = "filepath";
        result = ma_decoder_init_file((*filepath).c_str(), NULL, &decoder);
    }

    else {
        filetype = "binary";
        const auto data = System::Resources::Manager::GetResource(key);

        if (data)
            result = ma_decoder_init_memory(data->array_buffer, data->byte_length, NULL, &decoder);
    }

    if (result != MA_SUCCESS) {
        LOG("Audio: failed to init audio: " + filetype);
        return;
    }

    std::vector<float> loadedSamples;

    //ma_data_source_seek_to_pcm_frame(decoder, _voices[i].currentFrame);

    SoundBuffer buffer;

    constexpr ma_uint64 CHUNK_SIZE = 4096;
    ma_uint64 totalFramesRead = 0;

    float tempBuffer[CHUNK_SIZE * 2];

    while (true) 
    {
        ma_uint64 framesReadThisLoop = 0;            
        ma_result result = ma_decoder_read_pcm_frames(&decoder, tempBuffer, CHUNK_SIZE, &framesReadThisLoop);

        if (framesReadThisLoop == 0)
            break;

        //accum / mix the scratch samples into output buff
        size_t samplesRead = (size_t)framesReadThisLoop * 2;

        loadedSamples.insert(loadedSamples.end(), tempBuffer, tempBuffer + samplesRead);
        totalFramesRead += framesReadThisLoop;

        if (result != MA_SUCCESS)
            break;
    }
    
    ma_decoder_uninit(&decoder);
    
    if (totalFramesRead == 0 || loadedSamples.empty()) 
        return;
    
    size_t memSize = loadedSamples.size() * sizeof(float);
    buffer.pData = (float*)std::malloc(memSize);
    
    if (buffer.pData == nullptr) 
        return;
    
    std::memcpy(buffer.pData, loadedSamples.data(), memSize);

    buffer.totalFrames = (ma_uint32)totalFramesRead;
    buffer.channels = 2;
        
    _soundBuffers.emplace_back(buffer);
    _loadedSounds.insert({ key, _loadedSounds.size() });
}


//-------------------------------------------


void Audio::Play(const std::string& key, bool loop, float volume) 
{    
    int soundID = GetSoundByID(key);

    if (soundID == -1) {
        LOG("Audio: error - cannot play sound, ID not found.");
        return;
    }

    SetVolume(volume);

    for (int i = 0; i < MAX_VOICES; ++i)
        if (!_voices[i].isActive.load(std::memory_order_relaxed)) {
            _voices[i].pBuffer = &_soundBuffers[soundID];
            _voices[i].currentFrame = 0;
            _voices[i].shouldLoop.store(loop, std::memory_order_relaxed);
            _voices[i].isActive.store(true, std::memory_order_relaxed);

            return;
        }
}


//----------------------------------------


void Audio::Stop(const std::string& key) 
{
    int soundID = GetSoundByID(key);

    if (soundID == -1) {
        LOG("Audio: error - cannot stop sound, ID not found.");
        return;
    }

    SoundBuffer* targetBuffer = &_soundBuffers[soundID];

    for (int i = 0; i < MAX_VOICES; ++i)
        if (!_voices[i].isActive.load(std::memory_order_relaxed) && _voices[i].pBuffer == targetBuffer) 
            _voices[i].isActive.store(false, std::memory_order_relaxed);
    
    LOG("Audio: sound " + key + " stopped.");
}


//----------------------------------------


void Audio::StopAll() 
{
    for (int i = 0; i < MAX_VOICES; ++i)
        _voices[i].isActive.store(false, std::memory_order_relaxed);
    
    LOG("Audio: audio stopped.");
}


//----------------------------------------


void Audio::SetVolume(float volume) {
    if (ma_device_get_state(&_sound_device) == ma_device_state_started)
        ma_device_set_master_volume(&_sound_device, volume);
}




