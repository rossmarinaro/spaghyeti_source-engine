#pragma once

namespace System {

  //audio (sound, music)
  class Audio {

    public:

      static inline bool musicPlaying = false;

      static void play(const char* key, bool loop = false, float volume = 1);
      static void stop();
      static void setVolume(float volume);

    private:
    

  };

}

          