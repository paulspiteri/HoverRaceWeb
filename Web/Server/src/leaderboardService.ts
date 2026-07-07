import type { DatabaseSync } from "node:sqlite";
import type { LeaderboardEntry, SubmitLapTimeRequest, SubmitLapTimeResult, VehicleType } from "./leaderboardTypes";

export class LeaderboardService {
    private db: DatabaseSync;

    constructor(db: DatabaseSync) {
        this.db = db;
    }

    submitLapTime(request: SubmitLapTimeRequest): SubmitLapTimeResult {
        // Convert base64 ghost replay to buffer
        const ghostReplayBuffer = Buffer.from(request.ghostReplay, "base64");

        this.db
            .prepare(
                `
                INSERT INTO leaderboard (player_name, track_name, lap_time_ms, is_mobile, vehicle_type, ghost_replay)
                VALUES (?, ?, ?, ?, ?, ?)
            `,
            )
            .run(
                request.playerName ?? null,
                request.trackName,
                request.lapTimeMs,
                request.isMobile ? 1 : 0,
                request.vehicleType,
                ghostReplayBuffer,
            );

        return { success: true };
    }

    // Get the best (fastest) lap time for a specific track, mobile, and vehicle configuration
    getBestLapTime(trackName: string, isMobile: boolean, vehicleType: number): number | null {
        const row = this.db
            .prepare(
                `
                SELECT lap_time_ms
                FROM leaderboard
                WHERE track_name = ? AND is_mobile = ? AND vehicle_type = ?
                ORDER BY lap_time_ms ASC
                LIMIT 1
            `,
            )
            .get(trackName, isMobile ? 1 : 0, vehicleType) as { lap_time_ms: number } | undefined;

        return row ? row.lap_time_ms : null;
    }

    // Get top N lap times for a specific track and mobile configuration, optionally filtered by vehicle type
    // Returns only one lap time per player (their best/fastest time)
    getTopLapTimes(trackName: string, isMobile: boolean | undefined, limit: number = 10, vehicleType?: number): LeaderboardEntry[] {
        // Build WHERE clause conditions
        let whereConditions = `track_name = ?`;
        const params: (string | number)[] = [trackName];

        // Add mobile filter if specified (undefined means fetch all platforms)
        if (isMobile !== undefined) {
            whereConditions += ` AND is_mobile = ?`;
            params.push(isMobile ? 1 : 0);
        }

        // Add vehicle type filter if specified
        if (vehicleType !== undefined) {
            whereConditions += ` AND vehicle_type = ?`;
            params.push(vehicleType);
        }

        // Use CTE with window function to get only the best time per player
        // Anonymous players are treated separately using their unique id
        const query = `
            WITH RankedTimes AS (
                SELECT
                    id, player_name, track_name, lap_time_ms, is_mobile, vehicle_type, created_at,
                    ROW_NUMBER() OVER (
                        PARTITION BY COALESCE(player_name, 'anonymous_' || id)
                        ORDER BY lap_time_ms ASC
                    ) as rank
                FROM leaderboard
                WHERE ${whereConditions}
            )
            SELECT id, player_name, track_name, lap_time_ms, is_mobile, vehicle_type, created_at
            FROM RankedTimes
            WHERE rank = 1
            ORDER BY lap_time_ms ASC
            LIMIT ?
        `;
        params.push(limit);

        const rows = this.db.prepare(query).all(...params) as Array<{
            id: number;
            player_name: string | null;
            track_name: string;
            lap_time_ms: number;
            is_mobile: number;
            vehicle_type: number;
            created_at: string;
        }>;

        return rows.map((row) => ({
            id: row.id,
            playerName: row.player_name,
            trackName: row.track_name,
            lapTimeMs: row.lap_time_ms,
            isMobile: row.is_mobile === 1,
            vehicleType: row.vehicle_type as VehicleType,
            createdAt: new Date(row.created_at),
        }));
    }

    // Get ghost replay by leaderboard entry ID
    getGhostReplay(id: number): Buffer | null {
        const row = this.db
            .prepare(
                `
                SELECT ghost_replay
                FROM leaderboard
                WHERE id = ?
            `,
            )
            .get(id) as { ghost_replay: Uint8Array } | undefined;

        if (!row) {
            return null;
        }

        if (!row.ghost_replay) {
            console.error("💥 Ghost replay missing for entry (database inconsistency)");
            throw new Error("Ghost replay missing for entry");
        }

        // node:sqlite returns BLOBs as Uint8Array; wrap so callers get Buffer helpers.
        return Buffer.from(row.ghost_replay);
    }
}
