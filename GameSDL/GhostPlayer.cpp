#include "GhostPlayer.h"
#include <fstream>
#include <iostream>
#include <cstring>

GhostPlayer::GhostPlayer()
{
}

GhostPlayer::~GhostPlayer()
{
}

bool GhostPlayer::LoadFromFile(const std::string pFilename)
{
	// Reset state
	mLoaded = false;
	mPlaybackStarted = false;
	mPlayerId = -1;
	mLapDuration = 0;
	mTrackName.clear();
	mFilename.clear();
	mFrames.clear();

	// Open file for binary reading
	std::ifstream inFile(pFilename, std::ios::binary);
	if (!inFile) {
		std::cout << "Ghost loading: Failed to open file: " << pFilename << std::endl;
		return false;
	}

	// Read header as a single block
	GhostFileHeader header;
	inFile.read(reinterpret_cast<char*>(&header), sizeof(GhostFileHeader));

	if (!inFile) {
		std::cout << "Ghost loading: Failed to read header from file: " << pFilename << std::endl;
		return false;
	}

	// Verify magic header
	if (std::strncmp(header.magic, "HOVERRACE-GHOST", 15) != 0) {
		std::cout << "Ghost loading: Invalid magic header in file: " << pFilename << std::endl;
		return false;
	}

	// Verify version
	if (header.version != 6) {
		std::cout << "Ghost loading: Unsupported version " << (int)header.version << " (expected 6) in file: " << pFilename << std::endl;
		return false;
	}

	// Extract data from header
	uint32_t frameCount = header.frameCount;
	mPlayerId = header.playerId;
	mLapDuration = header.lapDuration;
	mTrackName = std::string(header.trackName);

	// Reserve space for frames
	mFrames.reserve(frameCount);

	// Read all frames
	for (uint32_t i = 0; i < frameCount; i++) {
		GhostFrame frame;

		// Read entire frame at once
		inFile.read(reinterpret_cast<char*>(&frame), sizeof(GhostFrame));

		// Check if read was successful
		if (!inFile) {
			std::cout << "Ghost loading: Failed to read frame " << i << " of " << frameCount << std::endl;
			return false;
		}

		mFrames.push_back(frame);
	}

	// File will be automatically closed by destructor
	mLoaded = true;
	mCurrentFrameIndex = 0;
	mFilename = pFilename;

	std::cout << "Ghost loaded (" << frameCount << " frames)" << std::endl;

	return true;
}

bool GhostPlayer::LoadFromData(const GhostFile& pGhostFile)
{
	// Reset state
	mLoaded = false;
	mPlaybackStarted = false;
	mPlayerId = -1;
	mLapDuration = 0;
	mTrackName.clear();
	mFilename.clear();
	mFrames.clear();

	// Verify magic header
	if (std::strncmp(pGhostFile.header.magic, "HOVERRACE-GHOST", 15) != 0) {
		std::cout << "Ghost loading: Invalid magic header in data" << std::endl;
		return false;
	}

	// Verify version
	if (pGhostFile.header.version != 6) {
		std::cout << "Ghost loading: Unsupported version " << (int)pGhostFile.header.version << " (expected 6) in data" << std::endl;
		return false;
	}

	// Extract data from header
	mPlayerId = pGhostFile.header.playerId;
	mLapDuration = pGhostFile.header.lapDuration;
	mTrackName = std::string(pGhostFile.header.trackName);

	// Copy frames
	mFrames = pGhostFile.frames;

	mLoaded = true;
	mCurrentFrameIndex = 0;

	std::cout << "Ghost loaded (" << mFrames.size() << " frames)" << std::endl;

	return true;
}

void GhostPlayer::StartPlayback()
{
	mCurrentFrameIndex = 0;
	mPlaybackStarted = true;
}

GhostFrameResult GhostPlayer::GetNextFrame(MR_SimulationTime pSimulationLapTime)
{
	GhostFrameResult result;
	result.frame = nullptr;
	result.isCompleted = false;

	// Check if playback has been started and we have a loaded ghost
	if (!mPlaybackStarted || !mLoaded) {
		return result;
	}

	// Remember the starting index
	size_t startIndex = mCurrentFrameIndex;

	// Advance through frames until we find one that's in the future
	while (mCurrentFrameIndex < mFrames.size()) {
		const GhostFrame& frame = mFrames[mCurrentFrameIndex];

		// If this frame's time is in the future, stop
		if (frame.mSimTime > pSimulationLapTime) {
			break;
		}

		// This frame is at or before the current time, advance
		mCurrentFrameIndex++;
	}

	// If we advanced, return the most recent frame we passed
	if (mCurrentFrameIndex > startIndex) {
		result.frame = &mFrames[mCurrentFrameIndex - 1];
	}

	// Check if we've reached the end
	if (mCurrentFrameIndex >= mFrames.size()) {
		result.isCompleted = true;
	}

	return result;
}
