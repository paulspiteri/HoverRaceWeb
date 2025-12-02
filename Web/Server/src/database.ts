import sqlite3 from "sqlite3";

export function initializeDatabase(dbPath: string): sqlite3.Database {
    console.log(`📊 Initializing database at: ${dbPath}`);

    const db = new sqlite3.Database(dbPath, (err) => {
        if (err) {
            console.error("❌ Error opening database:", err);
            process.exit(1);
        }
        console.log("✅ Database connection established");
    });

    db.serialize(() => {
        db.run("PRAGMA foreign_keys = ON");

        db.run(
            `
            CREATE TABLE IF NOT EXISTS leaderboard (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                player_name TEXT,
                track_name TEXT NOT NULL,
                lap_time_ms INTEGER NOT NULL,
                is_mobile BOOLEAN NOT NULL,
                vehicle_type INTEGER NOT NULL,
                ghost_replay BLOB NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                UNIQUE(track_name, lap_time_ms, vehicle_type, is_mobile, player_name)
            )
        `,
            (err) => {
                if (err) {
                    console.error("❌ Error creating leaderboard table:", err);
                    process.exit(1);
                }
            },
        );

        db.run("CREATE INDEX IF NOT EXISTS idx_track_mobile ON leaderboard(track_name, is_mobile)");
        db.run("CREATE INDEX IF NOT EXISTS idx_lap_time ON leaderboard(lap_time_ms)", (err) => {
            if (err) {
                console.error("❌ Error creating indexes:", err);
                process.exit(1);
            } else {
                console.log("✅ Database schema initialized successfully");
            }
        });
    });

    return db;
}
