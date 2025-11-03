// GhostPlayer.h
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
#include "GhostFileFormat.h"

/**
 * Loads and manages ghost car recordings for playback.
 * Reads ghost files created by GhostRecorder.
 */
class GhostPlayer
{
public:
	GhostPlayer();
	~GhostPlayer();

	/**
	 * Load a ghost recording from disk.
	 * @param pFilename Path to the ghost file
	 * @return true if loaded successfully, false otherwise
	 */
	bool LoadFromFile(const std::string pFilename);

	/**
	 * Load a ghost recording from a GhostFile structure (in-memory).
	 * @param pGhostFile The ghost file data to load
	 * @return true if loaded successfully, false otherwise
	 */
	bool LoadFromData(const GhostFile& pGhostFile);

	/**
	 * Check if a ghost file is currently loaded.
	 * @return true if a file is loaded
	 */
	bool IsLoaded() const { return mLoaded; }

	/**
	 * Get the track name from the loaded ghost file.
	 * @return The track name, or empty string if not loaded
	 */
	const std::string& GetTrackName() const { return mTrackName; }

	/**
	 * Get the number of frames in the loaded ghost.
	 * @return Frame count, or 0 if not loaded
	 */
	size_t GetFrameCount() const { return mFrames.size(); }

	/**
	 * Get the player ID from the loaded ghost file.
	 * @return The player ID, or -1 if not loaded
	 */
	int GetPlayerId() const { return mPlayerId; }

	/**
	 * Get the lap duration from the loaded ghost file.
	 * @return The lap duration in milliseconds, or 0 if not loaded
	 */
	MR_SimulationTime GetLapDuration() const { return mLapDuration; }

	/**
	 * Start playback from the beginning.
	 * Call this when the lap starts.
	 * @param pLapStartTime The simulation time when the lap started
	 */
	void StartPlayback(MR_SimulationTime pLapStartTime);

	/**
	 * Get the next ghost frame for the given simulation time.
	 * @param pSimulationTime Current absolute simulation time
	 * @return Result containing the frame (or nullptr) and completion status
	 */
	GhostFrameResult GetNextFrame(MR_SimulationTime pSimulationTime);

private:
	bool mLoaded = false;
	bool mPlaybackStarted = false;
	int mPlayerId = -1;
	MR_SimulationTime mLapDuration = 0;
	std::string mTrackName;
	std::string mFilename;
	std::vector<GhostFrame> mFrames;
	size_t mCurrentFrameIndex = 0;
	MR_SimulationTime mLapStartTime = 0;
};
