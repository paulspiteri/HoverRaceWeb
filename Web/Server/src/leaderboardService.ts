import type sqlite3 from "sqlite3";
import type { LeaderboardEntry, SubmitLapTimeRequest, SubmitLapTimeResult } from "./leaderboardTypes";

export class LeaderboardService {
    private db: sqlite3.Database;

    constructor(db: sqlite3.Database) {
        this.db = db;
    }

    async submitLapTime(request: SubmitLapTimeRequest): Promise<SubmitLapTimeResult> {
        return new Promise((resolve, reject) => {
            // Convert base64 ghost replay to buffer
            const ghostReplayBuffer = Buffer.from(request.ghostReplay, 'base64');

            this.db.run(
                `
                INSERT INTO leaderboard (player_name, track_name, lap_time_ms, is_mobile, vehicle_type, ghost_replay)
                VALUES (?, ?, ?, ?, ?, ?)
            `,
                [request.playerName, request.trackName, request.lapTimeMs, request.isMobile ? 1 : 0, request.vehicleType, ghostReplayBuffer],
                (err: Error | null) => {
                    if (err) {
                        console.error("💥 Error submitting lap time:", err);
                        reject(err);
                        return;
                    }

                    resolve({ success: true });
                },
            );
        });
    }

    // Get the best (fastest) lap time for a specific track, mobile, and vehicle configuration
    async getBestLapTime(trackName: string, isMobile: boolean, vehicleType: number): Promise<number | null> {
        return new Promise((resolve, reject) => {
            this.db.get(
                `
                SELECT lap_time_ms
                FROM leaderboard
                WHERE track_name = ? AND is_mobile = ? AND vehicle_type = ?
                ORDER BY lap_time_ms ASC
                LIMIT 1
            `,
                [trackName, isMobile ? 1 : 0, vehicleType],
                (err, row: any) => {
                    if (err) {
                        console.error("💥 Error getting best lap time:", err);
                        reject(err);
                        return;
                    }

                    resolve(row ? row.lap_time_ms : null);
                },
            );
        });
    }

    // Get top N lap times for a specific track and mobile configuration, optionally filtered by vehicle type
    async getTopLapTimes(trackName: string, isMobile: boolean | undefined, limit: number = 10, vehicleType?: number): Promise<LeaderboardEntry[]> {
        return new Promise((resolve, reject) => {
            let query = `
                SELECT id, player_name, track_name, lap_time_ms, is_mobile, vehicle_type, created_at
                FROM leaderboard
                WHERE track_name = ?
            `;
            const params: any[] = [trackName];

            // Add mobile filter if specified (undefined means fetch all platforms)
            if (isMobile !== undefined) {
                query += ` AND is_mobile = ?`;
                params.push(isMobile ? 1 : 0);
            }

            // Add vehicle type filter if specified
            if (vehicleType !== undefined) {
                query += ` AND vehicle_type = ?`;
                params.push(vehicleType);
            }

            query += `
                ORDER BY lap_time_ms ASC
                LIMIT ?
            `;
            params.push(limit);

            this.db.all(query, params,
                (err, rows: any[]) => {
                    if (err) {
                        console.error("💥 Error getting top lap times:", err);
                        reject(err);
                        return;
                    }

                    const entries = rows.map((row) => ({
                        id: row.id,
                        playerName: row.player_name,
                        trackName: row.track_name,
                        lapTimeMs: row.lap_time_ms,
                        isMobile: row.is_mobile === 1,
                        vehicleType: row.vehicle_type,
                        createdAt: new Date(row.created_at),
                    }));

                    resolve(entries);
                },
            );
        });
    }

    // Get ghost replay by leaderboard entry ID
    async getGhostReplay(id: number): Promise<Buffer | null> {
        return new Promise((resolve, reject) => {
            this.db.get(
                `
                SELECT ghost_replay
                FROM leaderboard
                WHERE id = ?
            `,
                [id],
                (err, row: any) => {
                    if (err) {
                        console.error("💥 Error getting ghost replay:", err);
                        reject(err);
                        return;
                    }

                    if (!row) {
                        resolve(null);
                        return;
                    }

                    if (!row.ghost_replay) {
                        console.error("💥 Ghost replay missing for entry (database inconsistency)");
                        reject(new Error("Ghost replay missing for entry"));
                        return;
                    }

                    resolve(row.ghost_replay);
                },
            );
        });
    }
}
