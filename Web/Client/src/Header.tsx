import { Group} from "@mantine/core";
import { ConnectionStatus } from "./ConnectionStatus";
import styles from "./Header.module.css";

export const Header: React.FC = () => {
    return (
        <Group className={styles.header}>
            <ConnectionStatus className={styles.connectionStatus} />
        </Group>
    );
};