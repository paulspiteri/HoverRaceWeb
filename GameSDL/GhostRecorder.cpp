#include "GhostRecorder.h"
#ifndef __EMSCRIPTEN__
#include <fstream>
#include <filesystem>
#endif
#include <ctime>
#include <cstring>
#include <sys/stat.h>
#include <iostream>

GhostRecorder::GhostRecorder(int playerId)
	: mIsRecording(false), mLapStartTime(0), mLastRecordTime(0), mLapDuration(0), mPlayerId(playerId)
{
	// Reserve space for ~1 minute lap at 16ms intervals (60000ms / 16ms = 3750 frames)
	mRecordedFrames.reserve(3750);
}

void GhostRecorder::StartRecording(MR_SimulationTime pLapStartTime)
{
	if (!mIsRecording) {
		mIsRecording = true;
		mLapStartTime = pLapStartTime;
		mLastRecordTime = pLapStartTime;
		mRecordedFrames.clear();
		std::cout << "Ghost recording: Started recording lap at sim time " << pLapStartTime << "ms" << std::endl;
	}
}

void GhostRecorder::RecordFrame(const MR_MainCharacterState& pState, MR_SimulationTime pCurrentTime)
{
	if (mIsRecording) {
		// Throttle to ~27 Hz (37ms between frames)
		MR_SimulationTime timeSinceLastRecord = pCurrentTime - mLastRecordTime;

		if (timeSinceLastRecord >= 16) {
			GhostFrame frame;
			frame.mSimTime = pCurrentTime - mLapStartTime;  // Calculate relative time

			// Copy the MR_MainCharacterState data
			std::memcpy(&frame.mState, &pState, sizeof(MR_MainCharacterState));

			mRecordedFrames.push_back(frame);
			mLastRecordTime = pCurrentTime;
		}
	}
}

void GhostRecorder::StopRecording(MR_SimulationTime pLapDuration)
{
	if (mIsRecording) {
		mIsRecording = false;
		mLapDuration = pLapDuration;
	}
}

GhostFile GhostRecorder::GetGhostFile(const std::string& pTrackName) const
{
	GhostFile ghostFile;

	// Build header
	ghostFile.header = {};
	std::strncpy(ghostFile.header.magic, "HOVERRACE-GHOST", sizeof(ghostFile.header.magic));
	ghostFile.header.version = 6;  // Version 6: Fixed-size header with embedded track name
	ghostFile.header.frameCount = static_cast<uint32_t>(mRecordedFrames.size());
	ghostFile.header.timestamp = static_cast<uint64_t>(std::time(nullptr));
	ghostFile.header.playerId = mPlayerId;
	ghostFile.header.lapDuration = mLapDuration;
	std::strncpy(ghostFile.header.trackName, pTrackName.c_str(), sizeof(ghostFile.header.trackName) - 1);
	ghostFile.header.trackName[sizeof(ghostFile.header.trackName) - 1] = '\0';  // Ensure null termination

	// Copy frames
	ghostFile.frames = mRecordedFrames;

	return ghostFile;
}

bool GhostRecorder::Save(const std::string& pTrackName)
{
#ifdef __EMSCRIPTEN__
	return false;
#endif
	if (mRecordedFrames.empty()) {
		return false;
	}

	// Get the ghost file data
	GhostFile ghostFile = GetGhostFile(pTrackName);

	// Generate filename
	std::string filename = GenerateFilename(pTrackName);

	// Open file for binary writing
	std::ofstream outFile(filename, std::ios::binary);
	if (!outFile) {
		return false;
	}

	// Write header as a single block
	outFile.write(reinterpret_cast<const char*>(&ghostFile.header), sizeof(GhostFileHeader));

	// Write all recorded frames
	for (const auto& frame : ghostFile.frames) {
		outFile.write(reinterpret_cast<const char*>(&frame), sizeof(GhostFrame));
	}

	std::cout << "Ghost recorded (" << ghostFile.frames.size() << " frames)" << std::endl;
	return true;
}

std::string GhostRecorder::GenerateFilename(const std::string& pTrackName) const
{
	// Use a simple ghosts directory in the current working directory
	std::string ghostDir = "ghosts";

	// Create ghosts directory if it doesn't exist
	std::filesystem::create_directories(ghostDir);

	// Sanitize track name (replace spaces and special chars with underscores)
	std::string sanitizedTrack = pTrackName;
	for (char& c : sanitizedTrack) {
		if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
			c = '_';
		}
	}

	std::string filename = ghostDir + "/" + sanitizedTrack + ".ghost";
	return filename;
}
