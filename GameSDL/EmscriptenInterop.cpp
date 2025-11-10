#include "EmscriptenInterop.h"
#include "GhostFileFormat.h"
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace EmscriptenInterop {

void OnLap(int newLap, MR_SimulationTime lapDuration, int vehicleType, const GhostFile& ghostData) {
#ifdef __EMSCRIPTEN__
    // Serialize ghost data to a contiguous buffer
    std::vector<unsigned char> binaryData;

    // Add header
    const unsigned char* headerPtr = reinterpret_cast<const unsigned char*>(&ghostData.header);
    binaryData.insert(binaryData.end(), headerPtr, headerPtr + sizeof(GhostFileHeader));

    // Add frames
    for (const auto& frame : ghostData.frames) {
        const unsigned char* framePtr = reinterpret_cast<const unsigned char*>(&frame);
        binaryData.insert(binaryData.end(), framePtr, framePtr + sizeof(GhostFrame));
    }

    const unsigned char* dataPtr = binaryData.data();
    const size_t dataSize = binaryData.size();

    // Call JavaScript with lap info, vehicle type, and ghost data
    EM_ASM({
        if (typeof onLapComplete === 'function') {
            // Create a new Uint8Array and copy the data
            var dataArray = new Uint8Array($4);
            for (var i = 0; i < $4; i++) {
                dataArray[i] = HEAPU8[$3 + i];
            }
            onLapComplete($0, $1, $2, dataArray);
        }
    }, newLap, lapDuration, vehicleType, dataPtr, dataSize);
#endif
}

} // namespace EmscriptenInterop
