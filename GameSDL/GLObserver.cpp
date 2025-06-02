#include "Observer.h"
#include "imgui.h"
#define SOKOL_IMGUI_NO_SOKOL_APP

#include "util/sokol_imgui.h"

void MR_Observer::RenderGLView(const MR_MainCharacter* pViewingCharacter, MR_SimulationTime pTime)
{
    MR_Angle lOrientation = pViewingCharacter->mOrientation;

    int lDist = 3400;
    if (pTime < -3000)
    {
        int lFactor = (-pTime - 3000) * 2 / 3;
        int rotateMRDegrees = lFactor * MR_2PI / 11000; // some fraction of a game circle
        lOrientation = static_cast<MR_Int16>((lOrientation + rotateMRDegrees) % MR_2PI);

        lDist += lFactor;
    }

    MR_3DCoordinate lCameraPos;
    auto orientationRadians = MR_ANGLE_TO_RADIANS(lOrientation);
    lCameraPos.mX = pViewingCharacter->mPosition.mX - lDist * cos(orientationRadians);
    lCameraPos.mY = pViewingCharacter->mPosition.mY - lDist * sin(orientationRadians);
    lCameraPos.mZ = pViewingCharacter->mPosition.mZ + 1700;
    if (mLastGlCameraPos.has_value())
    {
        constexpr float XY_SMOOTHING = 0.75f;
        constexpr float Z_SMOOTHING = 0.667f;
        lCameraPos.mX = std::lerp(mLastGlCameraPos.value().mX, lCameraPos.mX, XY_SMOOTHING);
        lCameraPos.mY = std::lerp(mLastGlCameraPos.value().mY, lCameraPos.mY, XY_SMOOTHING);
        lCameraPos.mZ = std::lerp(mLastGlCameraPos.value().mZ, lCameraPos.mZ, Z_SMOOTHING);
    }
    mLastGlCameraPos = lCameraPos;
    mGLView.SetCameraPosition(lCameraPos, lOrientation);
    mGLView.SetSimulationTime(pTime);
}

void MR_Observer::RenderGLHUD(const MR_MainCharacter* pViewingCharacter, MR_SimulationTime pTime)
{
    RenderGLHUDBars(pViewingCharacter);
    RenderGLHUDWeapon(pViewingCharacter, pTime);
}

void MR_Observer::RenderGLHUDBars(const MR_MainCharacter* pViewingCharacter)
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

void MR_Observer::RenderGLHUDWeapon(const MR_MainCharacter* pViewingCharacter, MR_SimulationTime pTime)
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
        float lMissileScaling = 1 + (310 / screenWidth);
        auto sprite = lWeaponSprite->GetSprite();
        //   draw_list->AddImage(nullptr, )
    }
}
