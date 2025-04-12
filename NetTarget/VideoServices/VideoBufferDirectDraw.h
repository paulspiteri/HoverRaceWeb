#include <ddraw.h>

class VideoBufferDirectDraw 
{
    public:
        HWND                 mWindow;
        LPDIRECTDRAW         mDirectDraw;
        LPDIRECTDRAWSURFACE  mFrontBuffer;
        LPDIRECTDRAWSURFACE  mBackBuffer;
        LPDIRECTDRAWCLIPPER  mClipper;    // To use in windows only
                                        // remove if too slow

        VideoBufferDirectDraw(HWND pWindow);
        ~VideoBufferDirectDraw(); 

        void DeleteInternalSurfaces();
};
