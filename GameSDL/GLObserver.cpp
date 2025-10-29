#include "GLObserver.h"
#include "../Util/StrRes.h"

#include "imgui.h"
#define SOKOL_IMGUI_NO_SOKOL_APP
#include "util/sokol_imgui.h"

const char* gFirstLapStr = MR_LoadString( IDS_FLAP_STR );
const char* gChartFinish     = MR_LoadString( IDS_CHART_FINISH );
const char* gChart           = MR_LoadString( IDS_CHART );
const char* gHitChart        = MR_LoadString( IDS_HIT_CHART );
const char* gCountdownStr    = MR_LoadString( IDS_COUNTDOWN );
const char* gFinishStr       = MR_LoadString( IDS_FINISH );
const char* gFinishStrSingle = MR_LoadString( IDS_FINISH_SINGLE );
const char* gBestLapStr      = MR_LoadString( IDS_BEST_LAP );
const char* gHeaderStr       = MR_LoadString( IDS_HEADER );
const char* gLastLapStr      = MR_LoadString( IDS_LAST_LAP );
const char* gCurLapStr       = MR_LoadString( IDS_CUR_LAP );

GLObserver::GLObserver()
{
    MR_ObjectFromFactoryId lMissileLevelId = { 1, 1100 };
    mMissileLevel = (MR_SpriteHandle*)MR_DllObjectFactory::CreateObject( lMissileLevelId );

    MR_ObjectFromFactoryId lMineDispId = { 1, 1102 };
    mMineDisp = (MR_SpriteHandle*)MR_DllObjectFactory::CreateObject( lMineDispId );

    MR_ObjectFromFactoryId lPowerUpDispId = { 1, 1103 };
    mPowerUpDisp = (MR_SpriteHandle*)MR_DllObjectFactory::CreateObject( lPowerUpDispId );
}

void GLObserver::RenderGLDisplay( GLRenderer* glRenderer, const MR_MainCharacter* pViewingCharacter, MR_SimulationTime pTime )
{
    mGLView.Setup(glRenderer, glRenderer->state.swapchain.width, glRenderer->state.swapchain.height);
    mGLView.SetMapSize(mMapSize);

    if( pViewingCharacter->mRoom != -1 )
    {
        RenderGLView(pViewingCharacter, pTime);
    }
}

void GLObserver::RenderGLView(const MR_MainCharacter* pViewingCharacter, MR_SimulationTime pTime)
{
    MR_Angle lOrientation = pViewingCharacter->mOrientation;
    MR_Angle lLastOrientation = pViewingCharacter->mLastOrientation;

    int lDist = 3400;
    if (pTime < -3000)
    {
        int lFactor = (-pTime - 3000) * 2 / 3;
        int rotateMRDegrees = lFactor * MR_2PI / 11000; // some fraction of a game circle
        lOrientation = static_cast<MR_Int16>((lOrientation + rotateMRDegrees) % MR_2PI);
        lLastOrientation = static_cast<MR_Int16>((lLastOrientation + rotateMRDegrees) % MR_2PI);

        lDist += lFactor;
    }

    MR_3DCoordinate lCameraPos;
    auto orientationRadians = MR_ANGLE_TO_RADIANS(lOrientation);
    lCameraPos.mX = pViewingCharacter->mPosition.mX - lDist * cos(orientationRadians);
    lCameraPos.mY = pViewingCharacter->mPosition.mY - lDist * sin(orientationRadians);
    lCameraPos.mZ = pViewingCharacter->mPosition.mZ + 1700;

    auto lastOrientationRadians = MR_ANGLE_TO_RADIANS(lLastOrientation);
    MR_3DCoordinate lLastCameraPos;
    lLastCameraPos.mX = pViewingCharacter->mLastPosition.mX - lDist * cos(lastOrientationRadians);
    lLastCameraPos.mY = pViewingCharacter->mLastPosition.mY - lDist * sin(lastOrientationRadians);
    lLastCameraPos.mZ = pViewingCharacter->mLastPosition.mZ + 1700;

    constexpr float XY_SMOOTHING = 0.75f;
    constexpr float Z_SMOOTHING = 0.667f;
    lCameraPos.mX = std::lerp(lLastCameraPos.mX, lCameraPos.mX, XY_SMOOTHING);
    lCameraPos.mY = std::lerp(lLastCameraPos.mY, lCameraPos.mY, XY_SMOOTHING);
    lCameraPos.mZ = std::lerp(lLastCameraPos.mZ, lCameraPos.mZ, Z_SMOOTHING);

    mGLView.SetCameraPosition(lCameraPos, lOrientation);
    mGLView.SetSimulationTime(pTime);
}

void GLObserver::RenderGLHUD(const GLRenderer* glRenderer, const MR_ClientSession* currentSession)
{
    RenderGLHUDBars(currentSession->GetMainCharacter());
    RenderGLHUDWeapon(glRenderer, currentSession->GetMainCharacter(), currentSession->GetSimulationTime());
    RenderGLHUDLapTimes(currentSession);
}

void GLObserver::RenderGLHUDBars(const MR_MainCharacter* pViewingCharacter)
{
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    float screenWidth = io.DisplaySize.x;
    float screenHeight = io.DisplaySize.y;

    float lSpeedMeterLen = screenWidth / 2;
    float lFuelMeterLen = screenWidth / 4;
    float lMeterHeight = screenHeight / 32;
    float lXMargin = screenWidth / 32;
    float lYMargin = screenHeight / 32;;

    double lAbsSpeedLen = pViewingCharacter->GetAbsoluteSpeed() * lSpeedMeterLen;
    double lDirSpeedLen = pViewingCharacter->GetDirectionalSpeed() * lSpeedMeterLen;
    double lFuelLevel = pViewingCharacter->GetFuelLevel();
    double lFuelLen = lFuelLevel * lFuelMeterLen;


    // absolute speed
    ImVec2 bar_start = ImVec2(lXMargin, lYMargin);
    ImVec2 bar_end = ImVec2(lSpeedMeterLen, lYMargin + lMeterHeight);
    draw_list->AddRectFilled(bar_start, bar_end, IM_COL32(193, 202, 231, 0xFF));
    draw_list->AddRectFilled(bar_start, ImVec2(bar_start.x + lAbsSpeedLen, bar_end.y),IM_COL32(84, 115, 207, 0xFF));

    // directional speed
    ImVec2 dir_bar_start = ImVec2(lXMargin, lYMargin + lMeterHeight);
    ImVec2 dir_bar_end = ImVec2(lSpeedMeterLen, lYMargin + lMeterHeight + lMeterHeight);
    draw_list->AddRectFilled(dir_bar_start, dir_bar_end, IM_COL32(193, 202, 231, 0xFF));
    draw_list->AddRectFilled(dir_bar_start, ImVec2(dir_bar_start.x + std::abs(lDirSpeedLen), dir_bar_end.y),
                             lDirSpeedLen > 0 ? IM_COL32(84, 115, 207, 0xFF) : IM_COL32(204, 21, 77, 0xFF));

    // fuel
    ImVec2 fuel_bar_start = ImVec2(screenWidth - lXMargin - lFuelMeterLen, lYMargin);
    ImVec2 fuel_bar_end = ImVec2(screenWidth - lXMargin, lYMargin + lMeterHeight + lMeterHeight);
    draw_list->AddRectFilled(fuel_bar_start, fuel_bar_end, IM_COL32(193, 202, 231, 0xFF));
    draw_list->AddRectFilled(fuel_bar_start, ImVec2(fuel_bar_start.x + lFuelLen, dir_bar_end.y),
                             IM_COL32(84, 115, 207, 0xFF));
}

void GLObserver::RenderGLHUDWeapon(const GLRenderer* glRenderer, const MR_MainCharacter* pViewingCharacter,
                                    MR_SimulationTime pTime)
{
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    float screenWidth = io.DisplaySize.x;
    float screenHeight = io.DisplaySize.y;

    MR_SpriteHandle* lWeaponSprite = nullptr;
    int lWeaponSpriteIndex = 0;

    if (pViewingCharacter->GetCurrentWeapon() == MR_MainCharacter::eMissile)
    {
        lWeaponSprite = mMissileLevel;
        lWeaponSpriteIndex = pViewingCharacter->GetMissileRefillLevel(mMissileLevel->GetSprite()->GetNbItem());
    }
    else if (pViewingCharacter->GetCurrentWeapon() == MR_MainCharacter::eMine)
    {
        lWeaponSprite = mMineDisp;
        lWeaponSpriteIndex = pViewingCharacter->GetMineCount();

        if (lWeaponSpriteIndex > 0)
        {
            lWeaponSpriteIndex = ((lWeaponSpriteIndex - 1) * 2) + 1;
            if ((pTime >> 9) & 1)
            {
                lWeaponSpriteIndex++;
            }
        }
    }
    else if (pViewingCharacter->GetCurrentWeapon() == MR_MainCharacter::ePowerUp)
    {
        lWeaponSprite = mPowerUpDisp;
        lWeaponSpriteIndex = pViewingCharacter->GetPowerUpFraction(4);
        if (lWeaponSpriteIndex == 0)
        {
            lWeaponSpriteIndex = pViewingCharacter->GetPowerUpCount();
        }
        else
        {
            lWeaponSpriteIndex = 9 - lWeaponSpriteIndex;
        }
    }

    if (lWeaponSprite != nullptr)
    {
        ImTextureID texture_id = simgui_imtextureid(glRenderer->state.sprites_image);

        auto sprite = lWeaponSprite->GetSprite();
        auto resSprite = static_cast<const MR_ResSprite*>(sprite);
        auto atlasIndex = glRenderer->GetSpriteAtlasIndex(resSprite->GetResourceId());
        auto atlas_uv = glRenderer->state.sprite_atlas_coords[atlasIndex];
        float atlasSpriteHeight = (atlas_uv.w - atlas_uv.y) / sprite->GetNbItem();

        ImVec2 size(sprite->GetItemWidth(), sprite->GetItemHeight());
        ImVec2 pos(screenWidth - sprite->GetItemWidth(), screenHeight / 16);

        draw_list->AddImage(
            texture_id,
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            ImVec2(atlas_uv.x, atlas_uv.y + (atlasSpriteHeight * lWeaponSpriteIndex)),
            ImVec2(atlas_uv.z, atlas_uv.y + (atlasSpriteHeight * (lWeaponSpriteIndex + 1)))
        );
    }
}

void DrawTextWithEffect(ImVec2 pos, const char* text) {
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    
    float shadowOffset = 2.0f * io.FontGlobalScale;

    // Shadow (dark offset)
    draw_list->AddText(
        ImVec2(pos.x - shadowOffset, pos.y + shadowOffset),
        IM_COL32(35, 35, 35, 192),
        text);

    // Highlight (light offset)
    draw_list->AddText(
        ImVec2(pos.x + shadowOffset, pos.y - shadowOffset),
        IM_COL32(218, 218, 218, 128),
        text);

    // Main text
    draw_list->AddText(pos, IM_COL32(221, 22, 83, 255), text);
}

void GLObserver::RenderGLHUDLapTimes(const MR_ClientSession* pSession)
{
    auto pViewingCharacter = pSession->GetMainCharacter();
    auto pTime = pSession->GetSimulationTime();

    // Display timers
    char lMainLineBuffer[80];
    char lLapLineBuffer[80];
    lLapLineBuffer[0] = 0;

    if (pTime < 0)
    {
        pTime = -pTime;
        sprintf(lMainLineBuffer, gCountdownStr, (pTime % 60000) / 1000, (pTime % 1000) / 10,
                pViewingCharacter->GetTotalLap());
    }
    else if (pViewingCharacter->GetTotalLap() <= pViewingCharacter->GetLap())
    {
        MR_SimulationTime lTotalTime = pViewingCharacter->GetTotalTime();
        MR_SimulationTime lBestLap = pViewingCharacter->GetBestLapDuration();

        // Race is finish
        if (pSession->GetNbPlayers() > 1)
        {
            sprintf(lMainLineBuffer, gFinishStr, lTotalTime / 60000, (lTotalTime % 60000) / 1000,
                    (lTotalTime % 1000) / 10, pSession->GetRank(pViewingCharacter), pSession->GetNbPlayers());
        }
        else
        {
            sprintf(lMainLineBuffer, gFinishStrSingle, lTotalTime / 60000, (lTotalTime % 60000) / 1000,
                    (lTotalTime % 1000) / 10);
        }
        sprintf(lLapLineBuffer, gBestLapStr, lBestLap / 60000, (lBestLap % 60000) / 1000, (lBestLap % 1000) / 10);
    }
    else if (pViewingCharacter->GetLap() == 0)
    {
        // First lap
        sprintf(lMainLineBuffer, gHeaderStr, pTime / 60000, (pTime % 60000) / 1000, (pTime % 1000) / 10, 1,
                pViewingCharacter->GetTotalLap());
        // sprintf( lLapLineBuffer, "Current lap %d.%02d.%02d", pTime/60000, (pTime%60000)/1000, (pTime%1000)/10 );
    }
    else if (pViewingCharacter->GetLastLapCompletion() > (pTime - 8000))
    {
        // Lap terminated less than 8 sec ago
        MR_SimulationTime lBestLap = pViewingCharacter->GetBestLapDuration();
        MR_SimulationTime lLastLap = pViewingCharacter->GetLastLapDuration();

        // More than one lap completed
        sprintf(lMainLineBuffer, gHeaderStr, pTime / 60000, (pTime % 60000) / 1000, (pTime % 1000) / 10,
                pViewingCharacter->GetLap() + 1, pViewingCharacter->GetTotalLap());
        sprintf(lLapLineBuffer, gLastLapStr,
                lLastLap / 60000, (lLastLap % 60000) / 1000, (lLastLap % 1000) / 10,
                lBestLap / 60000, (lBestLap % 60000) / 1000, (lBestLap % 1000) / 10);
    }
    else
    {
        MR_SimulationTime lBestLap = pViewingCharacter->GetBestLapDuration();
        MR_SimulationTime lCurrentLap = pTime - pViewingCharacter->GetLastLapCompletion();

        // More than one lap completed
        sprintf(lMainLineBuffer, gHeaderStr, pTime / 60000, (pTime % 60000) / 1000, (pTime % 1000) / 10,
                pViewingCharacter->GetLap() + 1, pViewingCharacter->GetTotalLap());
        sprintf(lLapLineBuffer, gCurLapStr,
                lCurrentLap / 60000, (lCurrentLap % 60000) / 1000, (lCurrentLap % 1000) / 10,
                lBestLap / 60000, (lBestLap % 60000) / 1000, (lBestLap % 1000) / 10);
    }

    ImGuiIO& io = ImGui::GetIO();
    float screenWidth = io.DisplaySize.x;
    float screenHeight = io.DisplaySize.y;
    ImVec2 mainTextPos(screenWidth / 2, screenHeight / 16);
    ImVec2 mainTextSize = ImGui::CalcTextSize(lMainLineBuffer);
    mainTextPos.x -= mainTextSize.x / 2; // Center horizontally

    DrawTextWithEffect(mainTextPos, lMainLineBuffer);

    if (strlen(lLapLineBuffer) > 0)
    {
        ImVec2 lapTextPos(screenWidth / 2, screenHeight - 1);
        ImVec2 lapTextSize = ImGui::CalcTextSize(lLapLineBuffer);
        lapTextPos.x -= lapTextSize.x / 2;
        lapTextPos.y -= lapTextSize.y;
        DrawTextWithEffect(lapTextPos, lLapLineBuffer);
    }
}

void GLObserver::PlaySounds( const MR_Level* pLevel, MR_MainCharacter* pViewingCharacter )
{
    // Play the sound of all moving elemnts arround

    int lCurrentRoom   = pViewingCharacter->mRoom;
    int lNeighborCount = pLevel->GetRoomVertexCount( lCurrentRoom );

    for( int lCounter = -1; lCounter < lNeighborCount; lCounter++ )
    {
        int      lRoomId;

        if( lCounter == -1 )
        {
            lRoomId = lCurrentRoom;
        }
        else
        {
            lRoomId = pLevel->GetNeighbor( lCurrentRoom, lCounter );
        }

        if( lRoomId != -1 )
        {
            MR_FreeElementHandle lHandle = pLevel->GetFirstFreeElement( lRoomId );

            while( lHandle != NULL )
            {
                MR_FreeElement* lElement = MR_Level::GetFreeElement( lHandle );

                if( lElement != pViewingCharacter )
                {
                    double lXDist = pViewingCharacter->mPosition.mX-lElement->mPosition.mX;
                    double lYDist = pViewingCharacter->mPosition.mY-lElement->mPosition.mY;

                    int lDB = -std::sqrt( lXDist*lXDist+lYDist*lYDist )/15.0;


                    lElement->PlayExternalSounds( lDB, 0 );
                }

                lHandle = MR_Level::GetNextFreeElement( lHandle );
            }
        }
    }

    pViewingCharacter->PlayInternalSounds();
}
