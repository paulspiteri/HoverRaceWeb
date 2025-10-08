import { ConnectionStatus } from "./ConnectionStatus";
import { ElectroIntro } from "./ElectroIntro.tsx";
import styles from "./Header.module.css";

export const Header: React.FC = () => {
    return (
        <div className={styles.header}>
            <ConnectionStatus className={styles.connectionStatus} />
            <div className={styles.title}>HoverRace Web</div>
            <ElectroIntro  />
        </div>
    );
};