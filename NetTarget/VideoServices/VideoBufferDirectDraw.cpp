#include "VideoBufferDirectDraw.h"

VideoBufferDirectDraw::VideoBufferDirectDraw(HWND pWindow) : mWindow(pWindow), mDirectDraw(nullptr), mFrontBuffer(nullptr), mBackBuffer(nullptr), mClipper(nullptr) 
{
}

VideoBufferDirectDraw::~VideoBufferDirectDraw()
{
    if (mDirectDraw) 
    {
        mDirectDraw->Release();
        mDirectDraw = nullptr;
    }
}

void VideoBufferDirectDraw::DeleteInternalSurfaces()
{
    if( mDirectDraw!= nullptr )
    {
       if( mBackBuffer != nullptr )
       {
          mBackBuffer->Release();
          mBackBuffer = nullptr;
       }
 
       if( mFrontBuffer != nullptr )
       {
          mFrontBuffer->Release();
          mFrontBuffer = nullptr;
       }
 
       if( mClipper != nullptr )
       {
          mClipper->Release(); 
          mClipper = nullptr;
       }
    }
}
