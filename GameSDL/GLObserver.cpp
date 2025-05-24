#include "Observer.h"

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
    lCameraPos.mX  = pViewingCharacter->mPosition.mX - lDist * cos(orientationRadians);
    lCameraPos.mY  = pViewingCharacter->mPosition.mY - lDist * sin(orientationRadians);
    lCameraPos.mZ  = pViewingCharacter->mPosition.mZ + 1700;
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
