import { Group, ActionIcon, Title } from "@mantine/core";
import { IconHome } from "@tabler/icons-react";
import { useNavigate } from "react-router-dom";
import { ConnectionStatus } from "./ConnectionStatus";
import styles from "./Header.module.css";

export const Header: React.FC = () => {
    const navigate = useNavigate();

    return (
        <Group className={styles.header}>
            <Group className={styles.headerContent}>
                <ActionIcon
                    variant="subtle"
                    onClick={() => navigate("/")}
                    size="lg"
                    title="Go to Home"
                    className={styles.homeButton}
                >
                    <IconHome className={styles.homeIcon} />
                </ActionIcon>
                <Title
                    order={2}
                    className={styles.title}
                    onClick={() => navigate("/")}
                >
                    HoverRace
                </Title>
            </Group>
            <ConnectionStatus className={styles.connectionStatus} />
        </Group>
    );
};