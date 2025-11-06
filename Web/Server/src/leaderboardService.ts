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
                INSERT INTO leaderboard (player_name, track_name, lap_time_ms, is_mobile, ghost_replay)
                VALUES (?, ?, ?, ?, ?)
            `,
                [request.playerName, request.trackName, request.lapTimeMs, request.isMobile ? 1 : 0, ghostReplayBuffer],
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

    // Get top N lap times for a specific track and mobile configuration
    async getTopLapTimes(trackName: string, isMobile: boolean, limit: number = 10): Promise<LeaderboardEntry[]> {
        return new Promise((resolve, reject) => {
            this.db.all(
                `
                SELECT id, player_name, track_name, lap_time_ms, is_mobile, created_at
                FROM leaderboard
                WHERE track_name = ? AND is_mobile = ?
                ORDER BY lap_time_ms ASC
                LIMIT ?
            `,
                [trackName, isMobile ? 1 : 0, limit],
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
