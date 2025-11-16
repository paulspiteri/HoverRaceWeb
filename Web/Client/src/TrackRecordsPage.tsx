import * as React from "react";
import { useParams, useSearchParams, useNavigate } from "react-router-dom";
import { Container, Table, Text, Stack, Paper, Badge, Group, SegmentedControl, Button } from "@mantine/core";
import { useLeaderboard } from "@/hooks/useLeaderboard.ts";
import { VehicleType } from "@/types.ts";
import styles from "./TrackRecordsPage.module.css";

const tracks = [
    { name: "ClassicH", displayName: "ClassicH" },
    { name: "Steeplechase", displayName: "Steeplechase" },
    { name: "The Alley2", displayName: "The Alley2" },
    { name: "The River", displayName: "The River" },
];

const vehicleNames: Record<VehicleType, string> = {
    [VehicleType.ELECTRO]: "Electro",
    [VehicleType.HITECH]: "HiTech",
    [VehicleType.BITURBO]: "BiTurbo",
};

const vehicleColors: Record<VehicleType, string> = {
    [VehicleType.ELECTRO]: "blue",
    [VehicleType.HITECH]: "green",
    [VehicleType.BITURBO]: "red",
};

function formatLapTime(lapTimeMs: number): string {
    const totalSeconds = lapTimeMs / 1000;
    const minutes = Math.floor(totalSeconds / 60);
    const seconds = (totalSeconds % 60).toFixed(3);

    if (minutes > 0) {
        return `${minutes}:${seconds.padStart(6, '0')}`;
    }
    return `${seconds}s`;
}

function formatDate(date: Date): string {
    return new Date(date).toLocaleDateString('en-US', {
        year: 'numeric',
        month: 'short',
        day: 'numeric',
    });
}

export const TrackRecordsPage: React.FC = () => {
    const { trackname } = useParams<{ trackname: string }>();
    const [searchParams] = useSearchParams();
    const navigate = useNavigate();

    // Parse URL parameters for filters
    const vehicleParam = searchParams.get('vehicle');
    const platformParam = searchParams.get('platform');

    // Convert vehicle param to VehicleType number or undefined
    let vehicleFilter: number | undefined;
    if (vehicleParam === 'electro') {
        vehicleFilter = VehicleType.ELECTRO;
    } else if (vehicleParam === 'hitech') {
        vehicleFilter = VehicleType.HITECH;
    } else if (vehicleParam === 'biturbo') {
        vehicleFilter = VehicleType.BITURBO;
    }

    // Convert platform param to boolean or undefined
    // true = mobile only, false = desktop only, undefined = all platforms
    let isMobile: boolean | undefined;
    if (platformParam === 'mobile') {
        isMobile = true;
    } else if (platformParam === 'desktop') {
        isMobile = false;
    }

    const { data: records, isLoading, error } = useLeaderboard(trackname, 10, vehicleFilter, isMobile);

    const handleVehicleChange = (value: string) => {
        const params = new URLSearchParams(searchParams);
        if (value === 'all') {
            params.delete('vehicle');
        } else {
            params.set('vehicle', value);
        }
        navigate(`/records/${trackname}?${params.toString()}`, { replace: true });
    };

    const handlePlatformChange = (value: string) => {
        const params = new URLSearchParams(searchParams);
        if (value === 'all') {
            params.delete('platform');
        } else {
            params.set('platform', value);
        }
        navigate(`/records/${trackname}?${params.toString()}`, { replace: true });
    };

    if (isLoading) {
        return (
            <Container size="lg" className={styles.container}>
                <Text>Loading records...</Text>
            </Container>
        );
    }

    if (error) {
        return (
            <Container size="lg" className={styles.container}>
                <Text c="red">Error loading records: {error.message}</Text>
            </Container>
        );
    }

    const hasRecords = records && records.length > 0;

    const handleTrackChange = (trackName: string) => {
        const params = new URLSearchParams(searchParams);
        navigate(`/records/${trackName}?${params.toString()}`);
    };

    return (
        <Container size="lg" className={styles.container}>
            <Stack gap="lg">
                <Group gap="xs" justify="center">
                    {tracks.map((track) => (
                        <Button
                            key={track.name}
                            onClick={() => handleTrackChange(track.name)}
                            variant={trackname === track.name ? "filled" : "light"}
                            size="sm"
                        >
                            {track.displayName}
                        </Button>
                    ))}
                </Group>

                <Group gap="lg" align="flex-start" justify="center">
                    <Stack gap="xs">
                        <Text size="sm" fw={500}>Vehicle</Text>
                        <SegmentedControl
                            value={vehicleParam || 'all'}
                            onChange={handleVehicleChange}
                            data={[
                                { label: 'All', value: 'all' },
                                { label: 'Electro', value: 'electro' },
                                { label: 'HiTech', value: 'hitech' },
                                { label: 'BiTurbo', value: 'biturbo' },
                            ]}
                        />
                    </Stack>

                    <Stack gap="xs">
                        <Text size="sm" fw={500}>Platform</Text>
                        <SegmentedControl
                            value={platformParam || 'all'}
                            onChange={handlePlatformChange}
                            data={[
                                { label: 'All', value: 'all' },
                                { label: 'Desktop', value: 'desktop' },
                                { label: 'Mobile', value: 'mobile' },
                            ]}
                        />
                    </Stack>
                </Group>

                {!hasRecords ? (
                    <Paper shadow="sm" p="md" withBorder>
                        <Text c="dimmed" ta="center">
                            No records found
                        </Text>
                    </Paper>
                ) : (
                    <Paper shadow="sm" p="md" withBorder>
                        <Table>
                            <Table.Thead>
                                <Table.Tr>
                                    <Table.Th style={{ width: '60px' }}>Rank</Table.Th>
                                    <Table.Th>Player</Table.Th>
                                    <Table.Th>Vehicle</Table.Th>
                                    <Table.Th>Lap Time</Table.Th>
                                    <Table.Th>Platform</Table.Th>
                                    <Table.Th>Date</Table.Th>
                                </Table.Tr>
                            </Table.Thead>
                            <Table.Tbody>
                                {records.map((record, index) => (
                                    <Table.Tr key={record.id}>
                                        <Table.Td>
                                            <Text fw={700} size="lg" c={index === 0 ? 'yellow' : index === 1 ? 'gray' : index === 2 ? 'orange' : undefined}>
                                                {index + 1}
                                            </Text>
                                        </Table.Td>
                                        <Table.Td>
                                            <Text fw={index < 3 ? 600 : 400}>{record.playerName}</Text>
                                        </Table.Td>
                                        <Table.Td>
                                            <Badge color={vehicleColors[record.vehicleType]} variant="light">
                                                {vehicleNames[record.vehicleType]}
                                            </Badge>
                                        </Table.Td>
                                        <Table.Td>
                                            <Text fw={index === 0 ? 700 : 500} c={index === 0 ? 'yellow' : undefined}>
                                                {formatLapTime(record.lapTimeMs)}
                                            </Text>
                                        </Table.Td>
                                        <Table.Td>
                                            <Badge color={record.isMobile ? 'cyan' : 'grape'} variant="outline">
                                                {record.isMobile ? 'Mobile' : 'Desktop'}
                                            </Badge>
                                        </Table.Td>
                                        <Table.Td>
                                            <Text size="sm" c="dimmed">{formatDate(record.createdAt)}</Text>
                                        </Table.Td>
                                    </Table.Tr>
                                ))}
                            </Table.Tbody>
                        </Table>
                    </Paper>
                )}
            </Stack>
        </Container>
    );
};
