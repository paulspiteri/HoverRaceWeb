#include "SoundServer.h"
#include <vector>
#include <SDL3/SDL.h>

#define MR_MAX_SOUND_COPY 6

typedef struct WAVEFORMATEX
{
    WORD    wFormatTag;        /* format type */
    WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
    DWORD   nSamplesPerSec;    /* sample rate */
    DWORD   nAvgBytesPerSec;   /* for buffer estimation */
    WORD    nBlockAlign;       /* block size of data */
    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
    WORD    cbSize;            /* The count in bytes of the size of
                                    extra information (after cbSize) */

} WAVEFORMATEX;

class MR_SoundBuffer
{
    protected:
        uint32_t mSoundDataLen;
        const char* mSoundData;
        int mNbCopy;
        SDL_AudioStream* mSoundBuffer[MR_MAX_SOUND_COPY];

   public:
       MR_SoundBuffer(const char* pData, int pNbCopy)
       {
            if(pNbCopy > MR_MAX_SOUND_COPY)
            {
                ASSERT(FALSE);
                pNbCopy = MR_MAX_SOUND_COPY;
            }
            mNbCopy = pNbCopy;

            mSoundDataLen = *(uint32_t*)pData;
            WAVEFORMATEX* lWaveFormat = (WAVEFORMATEX*)(pData + sizeof(mSoundDataLen));
            mSoundData = const_cast<char*>(pData + sizeof(uint32_t) + sizeof(WAVEFORMATEX));
            if (lWaveFormat->wBitsPerSample != 8)
            {
                ASSERT(false);
                throw new std::runtime_error("Unsupported audio format: only 8-bit PCM is supported.");
            }
            SDL_AudioSpec sdlAudioSpec;
            sdlAudioSpec.format = SDL_AUDIO_U8;
            sdlAudioSpec.channels = lWaveFormat->nChannels;
            sdlAudioSpec.freq = lWaveFormat->nSamplesPerSec;

            for(int lCounter = 0; lCounter < mNbCopy; lCounter++)
            {
                auto audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &sdlAudioSpec, nullptr, nullptr);
                if (!audioStream) {
                    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
                }
                mSoundBuffer[lCounter] = audioStream;
            }
      }

         ~MR_SoundBuffer()
         {
            for(int lCounter = 0; lCounter < mNbCopy; lCounter++)
            {
                if (mSoundBuffer[lCounter]) {
                    SDL_DestroyAudioStream(mSoundBuffer[lCounter]);
                    mSoundBuffer[lCounter] = nullptr;
                }
            }
          }
};

class MR_ShortSound : public MR_SoundBuffer
{   
    private:
        int mCurrentCopy;

    public:
        MR_ShortSound(const char* pData, int pNbCopy) 
            : MR_SoundBuffer(pData, pNbCopy)
        {
            mCurrentCopy = 0;
        }

        void Play()
        {
            if (mSoundBuffer[mCurrentCopy]) {
                SDL_FlushAudioStream(mSoundBuffer[mCurrentCopy]);
                SDL_PutAudioStreamData(mSoundBuffer[mCurrentCopy], mSoundData, mSoundDataLen);
                SDL_ResumeAudioStreamDevice(mSoundBuffer[mCurrentCopy]);
            }

            mCurrentCopy++;
            if(mCurrentCopy >= mNbCopy )
            {
               mCurrentCopy = 0;
            }
        }
};

class MR_ContinuousSound : public MR_SoundBuffer
{
    public:
        MR_ContinuousSound(const char* pData, int pNbCopy) 
        : MR_SoundBuffer(pData, pNbCopy)
        {
        }
};

namespace MR_SoundServer
{
    std::vector <MR_SoundBuffer*> mSoundBufferList;
    MR_SoundBuffer* mSoundBuffer = nullptr;

   BOOL Init()
   {
    if (!SDL_Init(SDL_INIT_AUDIO)) 
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "SDL_Init failed: %s", SDL_GetError());
        return FALSE;
    }
      return TRUE;
   }

   void Close()
   {
      // Do nothing
   }

   MR_ShortSound* CreateShortSound(const char* pData, int pNbCopy)
   {
        auto soundBuffer = new MR_ShortSound(pData, pNbCopy);
        mSoundBufferList.push_back(soundBuffer);
        return soundBuffer;
   }

   void DeleteShortSound(MR_ShortSound* pSound)
   {
        delete pSound;
   }

   void Play(MR_ShortSound* pSound, int pDB, double pSpeed, int pPan)
   {
        pSound->Play();
   }

   int GetNbCopy(MR_ShortSound* pSound)
   {
      return 0; // TODO 
   }

   MR_ContinuousSound* CreateContinuousSound(const char* pData, int pNbCopy)
   {
        return new MR_ContinuousSound(pData, pNbCopy);
   }

   void DeleteContinuousSound(MR_ContinuousSound* pSound)
   {
        delete pSound;
   }

   void Play(MR_ContinuousSound* pSound, int pCopy, int pDB, double pSpeed, int pPan)
   {
        // pSound->Play();
   }

   int GetNbCopy(MR_ContinuousSound* pSound)
   {
        return 0; // TODO
   }

   void ApplyContinuousPlay()
   {
   }
}

