import { describe, it, expect, beforeEach, afterEach } from 'vitest';
import sqlite3 from 'sqlite3';
import { LeaderboardService } from './leaderboardService.js';
import { initializeDatabase } from './database.js';

describe('LeaderboardService - getTopLapTimes', () => {
    let db: sqlite3.Database;
    let service: LeaderboardService;

    beforeEach(async () => {
        // Use the actual initializeDatabase function
        db = initializeDatabase(':memory:');
        service = new LeaderboardService(db);

        // Wait for schema initialization to complete
        await new Promise<void>((resolve) => {
            setTimeout(resolve, 50);
        });
    });

    afterEach(async () => {
        // Close database connection
        await new Promise<void>((resolve) => {
            db.close(() => resolve());
        });
    });

    async function insertLapTime(
        playerName: string | null,
        trackName: string,
        lapTimeMs: number,
        isMobile: boolean,
        vehicleType: number
    ): Promise<void> {
        // Use the actual service method
        await service.submitLapTime({
            playerName: playerName ?? undefined,
            trackName,
            lapTimeMs,
            isMobile,
            vehicleType: vehicleType as 0 | 1 | 2,
            ghostReplay: Buffer.from('test').toString('base64'),
        });
    }

    it('returns only one result per player (their best time)', async () => {
        // Insert multiple times for the same player
        await insertLapTime('Alice', 'track1', 50000, false, 1);
        await insertLapTime('Alice', 'track1', 45000, false, 1); // Better time
        await insertLapTime('Alice', 'track1', 55000, false, 1);
        await insertLapTime('Bob', 'track1', 48000, false, 1);

        const results = await service.getTopLapTimes('track1', false, 10);

        // Should return only 2 results (one per player)
        expect(results).toHaveLength(2);

        // Alice should have her best time (45000)
        const alice = results.find((r) => r.playerName === 'Alice');
        expect(alice?.lapTimeMs).toBe(45000);

        // Bob should have his only time
        const bob = results.find((r) => r.playerName === 'Bob');
        expect(bob?.lapTimeMs).toBe(48000);
    });

    it('treats anonymous players separately (does not group them)', async () => {
        // Insert multiple anonymous times
        await insertLapTime(null, 'track1', 40000, false, 1);
        await insertLapTime(null, 'track1', 41000, false, 1);
        await insertLapTime(null, 'track1', 42000, false, 1);

        const results = await service.getTopLapTimes('track1', false, 10);

        // All 3 anonymous entries should be returned separately
        expect(results).toHaveLength(3);
        expect(results.every((r) => r.playerName === null)).toBe(true);
    });

    it('orders results by lap time (fastest first)', async () => {
        await insertLapTime('Alice', 'track1', 50000, false, 1);
        await insertLapTime('Bob', 'track1', 45000, false, 1);
        await insertLapTime('Charlie', 'track1', 55000, false, 1);

        const results = await service.getTopLapTimes('track1', false, 10);

        expect(results).toHaveLength(3);
        expect(results[0].playerName).toBe('Bob'); // 45000
        expect(results[1].playerName).toBe('Alice'); // 50000
        expect(results[2].playerName).toBe('Charlie'); // 55000
    });

    it('respects mobile filter when true', async () => {
        await insertLapTime('Alice', 'track1', 45000, true, 1); // mobile
        await insertLapTime('Bob', 'track1', 40000, false, 1); // desktop

        const results = await service.getTopLapTimes('track1', true, 10);

        expect(results).toHaveLength(1);
        expect(results[0].playerName).toBe('Alice');
        expect(results[0].isMobile).toBe(true);
    });

    it('respects mobile filter when false', async () => {
        await insertLapTime('Alice', 'track1', 45000, true, 1); // mobile
        await insertLapTime('Bob', 'track1', 40000, false, 1); // desktop

        const results = await service.getTopLapTimes('track1', false, 10);

        expect(results).toHaveLength(1);
        expect(results[0].playerName).toBe('Bob');
        expect(results[0].isMobile).toBe(false);
    });

    it('returns all platforms when mobile filter is undefined', async () => {
        await insertLapTime('Alice', 'track1', 45000, true, 1); // mobile
        await insertLapTime('Bob', 'track1', 40000, false, 1); // desktop

        const results = await service.getTopLapTimes('track1', undefined, 10);

        expect(results).toHaveLength(2);
    });

    it('respects vehicle type filter', async () => {
        await insertLapTime('Alice', 'track1', 45000, false, 1);
        await insertLapTime('Bob', 'track1', 40000, false, 2);

        const results = await service.getTopLapTimes('track1', false, 10, 2);

        expect(results).toHaveLength(1);
        expect(results[0].playerName).toBe('Bob');
        expect(results[0].vehicleType).toBe(2);
    });

    it('respects limit parameter', async () => {
        await insertLapTime('Alice', 'track1', 45000, false, 1);
        await insertLapTime('Bob', 'track1', 46000, false, 1);
        await insertLapTime('Charlie', 'track1', 47000, false, 1);
        await insertLapTime('Dave', 'track1', 48000, false, 1);
        await insertLapTime('Eve', 'track1', 49000, false, 1);

        const results = await service.getTopLapTimes('track1', false, 3);

        expect(results).toHaveLength(3);
        expect(results[0].playerName).toBe('Alice');
        expect(results[1].playerName).toBe('Bob');
        expect(results[2].playerName).toBe('Charlie');
    });

    it('handles mix of named and anonymous players', async () => {
        await insertLapTime('Alice', 'track1', 45000, false, 1);
        await insertLapTime(null, 'track1', 40000, false, 1);
        await insertLapTime('Bob', 'track1', 50000, false, 1);
        await insertLapTime(null, 'track1', 42000, false, 1);

        const results = await service.getTopLapTimes('track1', false, 10);

        expect(results).toHaveLength(4);
        // Check mix includes both named and anonymous
        const namedPlayers = results.filter((r) => r.playerName !== null);
        const anonymousPlayers = results.filter((r) => r.playerName === null);
        expect(namedPlayers).toHaveLength(2);
        expect(anonymousPlayers).toHaveLength(2);
    });

    it('groups players correctly across different tracks', async () => {
        // Alice has times on both tracks
        await insertLapTime('Alice', 'track1', 45000, false, 1);
        await insertLapTime('Alice', 'track2', 50000, false, 1);

        const track1Results = await service.getTopLapTimes('track1', false, 10);
        const track2Results = await service.getTopLapTimes('track2', false, 10);

        expect(track1Results).toHaveLength(1);
        expect(track2Results).toHaveLength(1);
        expect(track1Results[0].lapTimeMs).toBe(45000);
        expect(track2Results[0].lapTimeMs).toBe(50000);
    });

    it('returns correct data structure', async () => {
        await insertLapTime('Alice', 'track1', 45000, true, 2);

        const results = await service.getTopLapTimes('track1', true, 10);

        expect(results[0]).toEqual({
            id: expect.any(Number),
            playerName: 'Alice',
            trackName: 'track1',
            lapTimeMs: 45000,
            isMobile: true,
            vehicleType: 2,
            createdAt: expect.any(Date),
        });
    });
});
