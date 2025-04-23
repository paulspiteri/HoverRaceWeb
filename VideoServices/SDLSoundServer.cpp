#include "SoundServer.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <list>
#include <cmath>

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

class MR_SoundBuffer;

struct StreamHungryCallbackData
{
    MR_SoundBuffer* soundBuffer;
    int copyIndex;
};

class MR_SoundBuffer
{
    private:
        StreamHungryCallbackData callbackData;

    protected:
        uint32_t mSoundDataLen;
        const char* mSoundData;
        int mNbCopy;
        SDL_AudioStream* mSoundBuffer[MR_MAX_SOUND_COPY];

        virtual void OnStreamHungry(SDL_AudioStream* stream, int copyIndex, int bytesNeeded)
        {
        }

   public:
        MR_SoundBuffer(const char* pData, int pNbCopy, bool isHungry)
        {
            if(pNbCopy > MR_MAX_SOUND_COPY)
            {
                ASSERT(FALSE);
                pNbCopy = MR_MAX_SOUND_COPY;
            }
            mNbCopy = pNbCopy;

            mSoundDataLen = (*(uint32_t*)pData) - 4;    /* I cannot explain this -4 but otherwise there is some corrupt popping up at the end of the continuous sound */
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
                callbackData.soundBuffer = this;
                callbackData.copyIndex = lCounter;
                mSoundBuffer[lCounter] = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &sdlAudioSpec, isHungry ? StreamHungryCallback : nullptr, &callbackData);
                if (!mSoundBuffer[lCounter]) {
                    SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
                }
            }
        }

        static void SDLCALL StreamHungryCallback(void* userdata, SDL_AudioStream* stream, int bytesNeeded, int bytesQueued)
        {
            StreamHungryCallbackData* cbData = static_cast<StreamHungryCallbackData*>(userdata);
            cbData->soundBuffer->OnStreamHungry(stream, cbData->copyIndex, bytesNeeded);
        }

        ~MR_SoundBuffer()
        {
            for(int lCounter = 0; lCounter < mNbCopy; lCounter++)
            {
                if (mSoundBuffer[lCounter]) 
                {
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
            : MR_SoundBuffer(pData, pNbCopy, false)
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
    private:
        BOOL   mOn[MR_MAX_SOUND_COPY];
        int    mMaxDB[MR_MAX_SOUND_COPY];
        double mMaxSpeed[MR_MAX_SOUND_COPY];
        int    mLooping[MR_MAX_SOUND_COPY];

        void ResetCumStat()
        {
            for( int lCounter = 0; lCounter < mNbCopy; lCounter++ )
            {
               mOn[ lCounter ] = FALSE;
               mMaxSpeed[ lCounter ] = 0;
               mMaxDB[ lCounter ] = -10000;
            }
        }

        void SetParams( int pCopy, int pDB, double pSpeed, int pPan )
        {
            if( pCopy >= mNbCopy )
            {
                pCopy = mNbCopy-1;
            }

            if( mSoundBuffer[ pCopy ] != NULL )
            {
                // set speed
                SDL_SetAudioStreamFrequencyRatio(mSoundBuffer[pCopy], pSpeed);

                // set volume
                // convert decibel value used by old DirectSound to linear value expected by SDL
                float attenuation_db = pDB / 100.0f;
                float gain = powf(10.0f, attenuation_db / 20.0f);
                gain = (gain < 0.0f) ? 0.0f : (gain > 1.0f) ? 1.0f : gain;
                SDL_SetAudioStreamGain(mSoundBuffer[pCopy], gain);
            }
        }


    protected:
        void OnStreamHungry(SDL_AudioStream* stream, int copyIndex, int bytesNeeded) override
        {
            // loop the sound
            const char* pos = this->mSoundData + this->mLooping[copyIndex];
            auto sizeput = std::min(this->mSoundDataLen - this->mLooping[copyIndex], static_cast<uint32_t>(bytesNeeded));
            SDL_PutAudioStreamData(stream, pos, sizeput);
            this->mLooping[copyIndex] += sizeput;
            if (this->mLooping[copyIndex] >= this->mSoundDataLen) {
                this->mLooping[copyIndex] = 0;
            }
        }

    public:
        MR_ContinuousSound(const char* pData, int pNbCopy) 
        : MR_SoundBuffer(pData, pNbCopy, true)
        {
            ResetCumStat();
            for( int lCounter = 0; lCounter < mNbCopy; lCounter++ )
            {
               mLooping[ lCounter ] = 0;
            }
        }

        void CumPlay(int pCopy, int pDB, double pSpeed)
        {
            if( pCopy >= mNbCopy )
            {
                pCopy = mNbCopy-1;
            }

            mOn[ pCopy ]       = TRUE;
            mMaxDB[ pCopy ]    = std::max( mMaxDB   [ pCopy ], pDB );
            mMaxSpeed[ pCopy ] = std::max( mMaxSpeed[ pCopy ], pSpeed );
        }

        inline static std::list<MR_ContinuousSound*> mContinuousSounds;

        void ApplyCumCommand()
        {
            for( int lCounter = 0; lCounter < mNbCopy; lCounter++ )
            {
                if( mOn[ lCounter ] )
                {
                    SetParams( lCounter, mMaxDB[ lCounter ], mMaxSpeed[ lCounter ], 0 );
                    Restart(lCounter);
                }
                else
                {
                    Pause(lCounter);
                }
            }
            ResetCumStat();
        }

        void Restart( int pCopy )
        {
            if( pCopy >= mNbCopy )
            {
                pCopy = mNbCopy-1;
            }
            if (mSoundBuffer[pCopy]) {
                SDL_ResumeAudioStreamDevice(mSoundBuffer[pCopy]);
            }
        }

        void Pause(int pCopy)
        {
            if( pCopy >= mNbCopy )
            {
               pCopy = mNbCopy-1;
            }
         
            SDL_PauseAudioStreamDevice(mSoundBuffer[pCopy]);
        }
};

namespace MR_SoundServer
{
    BOOL Init()
    {
        if (!SDL_Init(SDL_INIT_AUDIO)) 
        {
            SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "SDL_Init failed: %s", SDL_GetError());
            return FALSE;
        }
        return TRUE;
    }

    MR_ShortSound* CreateShortSound(const char* pData, int pNbCopy)
    {
        return new MR_ShortSound(pData, pNbCopy);
    }

    void DeleteShortSound(MR_ShortSound* pSound)
    {
        delete pSound;
    }

    void Play(MR_ShortSound* pSound, int pDB, double pSpeed, int pPan)
    {
        if (pSound != NULL)
        {
            pSound->Play();
        }
    }

    MR_ContinuousSound* CreateContinuousSound(const char* pData, int pNbCopy)
    {
        auto sound =  new MR_ContinuousSound(pData, pNbCopy);
        MR_ContinuousSound::mContinuousSounds.push_back(sound);
        return sound;
    }

    void DeleteContinuousSound(MR_ContinuousSound* pSound)
    {
        MR_ContinuousSound::mContinuousSounds.remove(pSound);
        delete pSound;
    }

    void Play(MR_ContinuousSound* pSound, int pCopy, int pDB, double pSpeed, int pPan)
    {
        if (pSound != NULL)
        {
            pSound->CumPlay( pCopy, pDB, pSpeed );
        }
    }

    void ApplyContinuousPlay()
    {
        for (auto it = MR_ContinuousSound::mContinuousSounds.begin(); it != MR_ContinuousSound::mContinuousSounds.end(); ++it)
        {
            MR_ContinuousSound* mCurrent = *it;
            mCurrent->ApplyCumCommand();
        }
    }
}
