import type express from "express";
import type { LeaderboardService } from "./leaderboardService";
import type { SubmitLapTimeRequest } from "./leaderboardTypes";

export function registerLeaderboardRoutes(app: express.Application, leaderboardService: LeaderboardService | null) {
    if (!leaderboardService) {
        return; // Skip registration if service not available
    }
    // REST endpoint to submit a lap time to the leaderboard
    app.post("/api/leaderboard/submit", async (req, res) => {
        console.log("🏁 POST /api/leaderboard/submit - Submit lap time request");
        try {
            const { playerName, trackName, lapTimeMs, isMobile, vehicleType, ghostReplay }: SubmitLapTimeRequest = req.body;
            console.log(`🎯 Submitting lap time: ${playerName} - ${trackName} - ${lapTimeMs}ms - mobile: ${isMobile} - vehicle: ${vehicleType}`);

            if (!playerName || !trackName || lapTimeMs === undefined || isMobile === undefined || vehicleType === undefined || !ghostReplay) {
                console.log("❌ Missing required fields");
                return res.status(400).json({ error: "Missing required fields: playerName, trackName, lapTimeMs, isMobile, vehicleType, ghostReplay" });
            }

            if (typeof lapTimeMs !== "number" || lapTimeMs <= 0) {
                console.log("❌ Invalid lap time");
                return res.status(400).json({ error: "lapTimeMs must be a positive number" });
            }

            if (typeof vehicleType !== "number" || vehicleType < 0 || vehicleType > 2) {
                console.log("❌ Invalid vehicle type");
                return res.status(400).json({ error: "vehicleType must be 0 (ELECTRO), 1 (HITECH), or 2 (BITURBO)" });
            }

            // Check if this lap time is better than the current best for this track/mobile/vehicle combination
            const currentBestTime = await leaderboardService.getBestLapTime(trackName, isMobile, vehicleType);
            if (currentBestTime !== null && lapTimeMs >= currentBestTime) {
                console.log(`⚠️ Lap time ${lapTimeMs}ms is not faster than current best ${currentBestTime}ms`);
                return res.status(400).json({
                    error: "Lap time is not faster than the current best",
                    currentBest: currentBestTime,
                    submitted: lapTimeMs
                });
            }

            const result = await leaderboardService.submitLapTime({ playerName, trackName, lapTimeMs, isMobile, vehicleType, ghostReplay });

            console.log(`✅ Lap time submitted successfully (beat best time of ${currentBestTime}ms)`);
            res.status(201).json(result);
        } catch (error) {
            console.log("💥 Error submitting lap time:", error);
            res.status(500).json({ error: "Failed to submit lap time" });
        }
    });

    // REST endpoint to get leaderboard for a track
    app.get("/api/leaderboard/:trackName", async (req, res) => {
        console.log("🏆 GET /api/leaderboard/:trackName - Get leaderboard request");
        try {
            const { trackName } = req.params;
            const isMobile = req.query.isMobile === "true";
            const limit = req.query.limit ? parseInt(req.query.limit as string, 10) : 10;
            const vehicleType = req.query.vehicleType !== undefined ? parseInt(req.query.vehicleType as string, 10) : undefined;
            console.log(`🎯 Getting leaderboard for ${trackName} - mobile: ${isMobile} - limit: ${limit} - vehicleType: ${vehicleType}`);

            if (!trackName) {
                console.log("❌ Missing track name");
                return res.status(400).json({ error: "Missing track name" });
            }

            if (isNaN(limit) || limit <= 0 || limit > 100) {
                console.log("❌ Invalid limit");
                return res.status(400).json({ error: "Limit must be between 1 and 100" });
            }

            if (vehicleType !== undefined && (isNaN(vehicleType) || vehicleType < 0 || vehicleType > 2)) {
                console.log("❌ Invalid vehicle type");
                return res.status(400).json({ error: "vehicleType must be 0 (ELECTRO), 1 (HITECH), or 2 (BITURBO)" });
            }

            const entries = await leaderboardService.getTopLapTimes(trackName, isMobile, limit, vehicleType);

            console.log(`✅ Retrieved ${entries.length} leaderboard entries`);
            res.status(200).json({ entries });
        } catch (error) {
            console.log("💥 Error getting leaderboard:", error);
            res.status(500).json({ error: "Failed to get leaderboard" });
        }
    });

    // REST endpoint to get ghost replay by leaderboard entry ID
    app.get("/api/leaderboard/ghost/:id", async (req, res) => {
        console.log("👻 GET /api/leaderboard/ghost/:id - Get ghost replay request");
        try {
            const id = parseInt(req.params.id, 10);
            console.log(`🎯 Getting ghost replay for leaderboard entry ${id}`);

            if (isNaN(id)) {
                console.log("❌ Invalid ID");
                return res.status(400).json({ error: "Invalid ID" });
            }

            const ghostReplay = await leaderboardService.getGhostReplay(id);

            if (!ghostReplay) {
                console.log(`⚠️ No ghost replay found for entry ${id}`);
                return res.status(404).json({ error: "Ghost replay not found" });
            }

            // Convert buffer to base64 for JSON response
            const base64GhostReplay = ghostReplay.toString('base64');

            console.log(`✅ Retrieved ghost replay for entry ${id} (${ghostReplay.length} bytes)`);
            res.status(200).json({ ghostReplay: base64GhostReplay });
        } catch (error) {
            console.log("💥 Error getting ghost replay:", error);
            res.status(500).json({ error: "Failed to get ghost replay" });
        }
    });
}
