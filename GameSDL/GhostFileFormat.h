// GhostFileFormat.h
//
// Copyright (c) 2025 HoverRace contributors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <vector>
#include <string>
#include "../Model/MazeElement.h"
#include "../Util/WorldCoordinates.h"
#include "../MainCharacter/MainCharacterState.h"

/**
 * File header for ghost replay files.
 * This struct represents the binary format at the start of .ghost files.
 */
struct GhostFileHeader
{
	char magic[16];           // "HOVERRACE-GHOST" (null-terminated)
	uint8_t version;          // File format version (currently 6)
	uint32_t frameCount;      // Number of frames in the file
	uint64_t timestamp;       // Unix timestamp when recorded
	int playerId;             // ID of the player who recorded this
	MR_SimulationTime lapDuration;  // Total lap duration in milliseconds
	char trackName[51];       // Track name (null-terminated, max 50 chars + null)
};

/**
 * Represents a single recorded frame with timing information.
 */
struct GhostFrame
{
	MR_SimulationTime mSimTime;  // Simulation time relative to lap start (in ms)
	MR_MainCharacterState mState;  // Car state data (18 bytes)
};

/**
 * Complete ghost replay file structure.
 *
 * Binary file layout:
 * 1. GhostFileHeader (fixed size)
 * 2. Array of GhostFrame structs (count from header.frameCount)
 */
struct GhostFile
{
	GhostFileHeader header;
	std::vector<GhostFrame> frames;  // Array of recorded frames

	// Helper to get total file size in bytes
	size_t GetFileSize() const
	{
		return sizeof(GhostFileHeader) +           // Header (fixed size)
		       (frames.size() * sizeof(GhostFrame)); // All frames
	}
};

/**
 * Result of getting the next ghost frame for playback.
 */
struct GhostFrameResult
{
	const GhostFrame* frame;      // The frame data, or nullptr if no frame available
	bool isCompleted;              // True when playback has reached the end
};
