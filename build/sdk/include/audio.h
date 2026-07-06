#pragma once

namespace System {
  class Audio {
        public:
            static void Init();
            static void Load(const std::string& key);
            static void Play(const std::string& key, bool loop = false, float volume = 1);
            static void Stop(const std::string& key);
            static void StopAll();
            static void SetVolume(float volume);
            static void ShutDown();
    };
}

          