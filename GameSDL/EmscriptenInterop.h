#pragma once

#include "../Util//WorldCoordinates.h"
#include "GhostFileFormat.h"

namespace EmscriptenInterop {
    void OnLap(int newLap, MR_SimulationTime lapDuration, const GhostFile& ghostData);
}