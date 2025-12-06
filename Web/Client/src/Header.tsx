import { ConnectionStatus } from "./ConnectionStatus";
import { ElectroIntro } from "./ElectroIntro.tsx";
import { ActionIcon, Group } from "@mantine/core";
import { IconTrophy, IconHome, IconInfoCircle } from "@tabler/icons-react";
import { useNavigate } from "react-router-dom";
import styles from "./Header.module.css";

export const Header: React.FC = () => {
    const navigate = useNavigate();

    return (
        <div className={styles.header}>
            <div className={styles.leftSide}>
                <Group gap="xs" className={styles.actionButtons}>
                    <ActionIcon
                        onClick={() => navigate('/')}
                        variant="subtle"
                        size="lg"
                        title="Home"
                    >
                        <IconHome size={24} />
                    </ActionIcon>
                    <ActionIcon
                        onClick={() => navigate('/records/ClassicH')}
                        variant="subtle"
                        size="lg"
                        title="View Records"
                    >
                        <IconTrophy size={24} />
                    </ActionIcon>
                    <ActionIcon
                        onClick={() => navigate('/about')}
                        variant="subtle"
                        size="lg"
                        title="About"
                    >
                        <IconInfoCircle size={24} />
                    </ActionIcon>
                </Group>
            </div>
            <ConnectionStatus className={styles.connectionStatus} />
            <div className={styles.title}>HoverRace Web</div>
            <ElectroIntro  />
        </div>
    );
};