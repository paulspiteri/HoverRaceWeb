import { DatabaseSync } from "node:sqlite";

export function initializeDatabase(dbPath: string): DatabaseSync {
    console.log(`📊 Initializing database at: ${dbPath}`);

    let db: DatabaseSync;
    try {
        db = new DatabaseSync(dbPath);
    } catch (err) {
        console.error("❌ Error opening database:", err);
        process.exit(1);
    }
    console.log("✅ Database connection established");

    try {
        db.exec("PRAGMA foreign_keys = ON");

        db.exec(`
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
        `);

        db.exec("CREATE INDEX IF NOT EXISTS idx_track_mobile ON leaderboard(track_name, is_mobile)");
        db.exec("CREATE INDEX IF NOT EXISTS idx_lap_time ON leaderboard(lap_time_ms)");

        console.log("✅ Database schema initialized successfully");
    } catch (err) {
        console.error("❌ Error initializing database schema:", err);
        process.exit(1);
    }

    return db;
}
