// GhostRecorder.h
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
 * Records ghost car data for lap 1 by capturing network state packets.
 * The recorded data can be saved to disk for future playback.
 */
class GhostRecorder
{
public:
	GhostRecorder(int playerId);

	/**
	 * Start recording lap 1 data.
	 * Should be called when the player crosses the finish line to start lap 1.
	 * @param pLapStartTime The simulation time when lap 1 started
	 */
	void StartRecording(MR_SimulationTime pLapStartTime);

	/**
	 * Record a single frame of car state data with timing.
	 * @param pState The character state to record (18 bytes)
	 * @param pCurrentTime The current simulation time
	 */
	void RecordFrame(const MR_MainCharacterState& pState, MR_SimulationTime pCurrentTime);

	/**
	 * Stop recording but keep data in memory.
	 * Should be called when lap 1 completes.
	 * @param pLapDuration The duration of the lap in milliseconds
	 */
	void StopRecording(MR_SimulationTime pLapDuration);

	/**
	 * Get the recorded data as a GhostFile structure.
	 * @param pTrackName The name of the track being raced
	 * @return GhostFile containing header and frames, or empty file if no data recorded
	 */
	GhostFile GetGhostFile(const std::string& pTrackName) const;

	/**
	 * Save the recorded data to disk.
	 * @param pTrackName The name of the track being raced
	 * @return true if save was successful, false otherwise
	 */
	bool Save(const std::string& pTrackName);

	/**
	 * Check if currently recording.
	 * @return true if recording is active
	 */
	bool IsRecording() const { return mIsRecording; }

private:
	bool mIsRecording;
	MR_SimulationTime mLapStartTime;  // Simulation time when lap 1 started
	MR_SimulationTime mLastRecordTime;  // Last time a frame was recorded
	MR_SimulationTime mLapDuration;  // Duration of the completed lap

	// Store frames with timing information
	std::vector<GhostFrame> mRecordedFrames;
	int mPlayerId;

	/**
	 * Generate the filename for the ghost recording.
	 * @param pTrackName The name of the track
	 * @return The full path to the ghost file
	 */
	std::string GenerateFilename(const std::string& pTrackName) const;
};
