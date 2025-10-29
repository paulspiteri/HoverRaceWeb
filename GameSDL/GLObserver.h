#pragma once

#include "ClientSession.h"
#include "../MainCharacter/MainCharacter.h"
#include "../ObjFacTools/SpriteHandle.h"
#include "../VideoServices/GL/GLViewport.h"

class GLObserver
{
    MR_SpriteHandle* mMissileLevel;
    MR_SpriteHandle* mMineDisp;
    MR_SpriteHandle* mPowerUpDisp;

    glm::ivec4 mMapSize = {};
    GLViewport mGLView;
    void RenderGLView(const MR_MainCharacter* pViewingCharacter, MR_SimulationTime pTime);

public:
    GLObserver();
    void RenderGLDisplay(GLRenderer* glRenderer, const MR_MainCharacter* pViewingCharacter,
                             MR_SimulationTime pTime);

    void PlaySounds(const MR_Level* pLevel, MR_MainCharacter* pViewingCharacter);

    void RenderGLHUD(const GLRenderer* glRenderer, const MR_ClientSession* currentSession);
    void RenderGLHUDBars(const MR_MainCharacter* pViewingCharacter);
    void RenderGLHUDWeapon(const GLRenderer* glRenderer, const MR_MainCharacter* pViewingCharacter,
                           MR_SimulationTime pTime);
    void RenderGLHUDLapTimes(const MR_ClientSession* pSession);
    void SetMapSize(glm::ivec4 vec) { mMapSize = vec; }
    glm::ivec4 GetMapSize() const { return mMapSize; };
};
