import { useEffect, useRef, useState } from "react";
import { useAtomValue } from "jotai";
import nipplejs from "nipplejs";
import styles from "./MobileInput.module.css";
import { GasPedalIcon } from "./GasPedalIcon";
import { IconRocket, IconBomb, IconBolt } from "@tabler/icons-react";
import { gameApiAtom } from "../atoms";
import { WeaponType, type GameInstanceAPI } from "../interop/gameInterop";

const simulateKeyboardEvent = (eventType: "keydown" | "keyup", canvasElement: HTMLCanvasElement, key: string) => {
    const event = new KeyboardEvent(eventType, {
        key,
        code: key,
        bubbles: true,
        cancelable: true,
    });
    canvasElement.dispatchEvent(event);
};

const useGasPedal = ({
    canvasElement,
    enabled,
    gasPedalZone,
    setIsGasPedalPressed,
}: {
    canvasElement: HTMLCanvasElement | null;
    enabled: boolean;
    gasPedalZone: HTMLDivElement | null;
    setIsGasPedalPressed: (pressed: boolean) => void;
}) => {
    useEffect(() => {
        if (!enabled || !canvasElement || !gasPedalZone) return;

        const handleTouchStart = (e: TouchEvent) => {
            e.preventDefault();
            setIsGasPedalPressed(true);
            simulateKeyboardEvent("keydown", canvasElement, "ShiftLeft");
        };

        const handleTouchEnd = (e: TouchEvent) => {
            e.preventDefault();
            setIsGasPedalPressed(false);
            simulateKeyboardEvent("keyup", canvasElement, "ShiftLeft");
        };
        gasPedalZone.addEventListener("touchstart", handleTouchStart, { passive: false });
        gasPedalZone.addEventListener("touchend", handleTouchEnd, { passive: false });

        return () => {
            gasPedalZone.removeEventListener("touchstart", handleTouchStart);
            gasPedalZone.removeEventListener("touchend", handleTouchEnd);
        };
    }, [enabled, canvasElement, gasPedalZone, setIsGasPedalPressed]);
};

const useWeaponButtons = ({
    canvasElement,
    enabled,
    fireButton1Zone,
    fireButton2Zone,
    fireButton3Zone,
    gameApi,
}: {
    canvasElement: HTMLCanvasElement | null;
    enabled: boolean;
    fireButton1Zone: HTMLDivElement | null;
    fireButton2Zone: HTMLDivElement | null;
    fireButton3Zone: HTMLDivElement | null;
    gameApi: GameInstanceAPI | undefined;
}) => {
    useEffect(() => {
        if (!enabled || !canvasElement) return;

        const createHandleTouchStart = (weaponType: number) => (e: TouchEvent) => {
            e.preventDefault();
            const buttonZone = e.currentTarget as HTMLDivElement;
            buttonZone.classList.add(styles.pressed);

            // Set the weapon type before firing
            if (gameApi?.setCurrentWeapon) {
                gameApi.setCurrentWeapon(weaponType);
            }

            simulateKeyboardEvent("keydown", canvasElement, "ControlLeft");
            setTimeout(() => {
                buttonZone.classList.remove(styles.pressed);
                simulateKeyboardEvent("keyup", canvasElement, "ControlLeft");
            }, 150);
        };

        const cleanup: (() => void)[] = [];

        if (fireButton1Zone) {
            const handleButton1Touch = createHandleTouchStart(WeaponType.Missile);
            fireButton1Zone.addEventListener("touchstart", handleButton1Touch, { passive: false });
            cleanup.push(() => fireButton1Zone.removeEventListener("touchstart", handleButton1Touch));
        }

        if (fireButton2Zone) {
            const handleButton2Touch = createHandleTouchStart(WeaponType.Mine);
            fireButton2Zone.addEventListener("touchstart", handleButton2Touch, { passive: false });
            cleanup.push(() => fireButton2Zone.removeEventListener("touchstart", handleButton2Touch));
        }

        if (fireButton3Zone) {
            const handleButton3Touch = createHandleTouchStart(WeaponType.PowerUp);
            fireButton3Zone.addEventListener("touchstart", handleButton3Touch, { passive: false });
            cleanup.push(() => fireButton3Zone.removeEventListener("touchstart", handleButton3Touch));
        }

        return () => {
            cleanup.forEach((fn) => fn());
        };
    }, [enabled, canvasElement, fireButton1Zone, fireButton2Zone, fireButton3Zone, gameApi]);
};

const useVirtualJoysticks = ({
    canvasElement,
    enabled,
    joystickZone,
}: MobileInputProps & {
    joystickZone: HTMLDivElement | null;
}) => {
    useEffect(() => {
        if (!enabled || !canvasElement || !joystickZone) return;

        const currentMovementKeys = new Set<string>();

        const movementManager = nipplejs.create({
            zone: joystickZone,
            mode: "static",
            position: { left: "50%", top: "50%" },
            color: "blue",
            size: 120,
        });

        movementManager.on("move", (_, data) => {
            currentMovementKeys.forEach((key) => {
                simulateKeyboardEvent("keyup", canvasElement, key);
            });
            currentMovementKeys.clear();

            const angle = data.angle.degree;
            const force = data.force;

            if (force > 0.1) {
                const keys: string[] = [];

                // Left
                if (angle >= 120 && angle < 240) {
                    keys.push("ArrowLeft");
                }
                // Right
                if (angle >= 300 || angle < 60) {
                    keys.push("ArrowRight");
                }

                // Up/Down only with higher force
                if (force > 0.3) {
                    // Up
                    if (angle >= 60 && angle < 120) {
                        keys.push("ArrowUp");
                    }
                    // Down
                    if (angle >= 240 && angle < 300) {
                        keys.push("ArrowDown");
                    }
                }

                keys.forEach((key) => {
                    simulateKeyboardEvent("keydown", canvasElement, key);
                    currentMovementKeys.add(key);
                });
            }
        });

        movementManager.on("end", () => {
            currentMovementKeys.forEach((key) => {
                simulateKeyboardEvent("keyup", canvasElement, key);
            });
            currentMovementKeys.clear();
        });

        return () => {
            movementManager.destroy();
        };
    }, [enabled, canvasElement, joystickZone]);
};

interface MobileInputProps {
    canvasElement: HTMLCanvasElement | null;
    enabled: boolean;
}

export function MobileInput({ canvasElement, enabled }: MobileInputProps) {
    const joystickZoneRef = useRef<HTMLDivElement>(null);
    const gasPedalZoneRef = useRef<HTMLDivElement>(null);
    const fireButton1ZoneRef = useRef<HTMLDivElement>(null);
    const fireButton2ZoneRef = useRef<HTMLDivElement>(null);
    const fireButton3ZoneRef = useRef<HTMLDivElement>(null);
    const [isGasPedalPressed, setIsGasPedalPressed] = useState(false);
    const gameApi = useAtomValue(gameApiAtom);

    useVirtualJoysticks({
        canvasElement,
        enabled,
        joystickZone: joystickZoneRef.current,
    });

    useGasPedal({
        canvasElement,
        enabled,
        gasPedalZone: gasPedalZoneRef.current,
        setIsGasPedalPressed,
    });

    useWeaponButtons({
        canvasElement,
        enabled,
        fireButton1Zone: fireButton1ZoneRef.current,
        fireButton2Zone: fireButton2ZoneRef.current,
        fireButton3Zone: fireButton3ZoneRef.current,
        gameApi,
    });

    return (
        <>
            <div ref={joystickZoneRef} className={`${styles.joystickLeft} ${enabled && styles.enabled}`}></div>
            <div
                ref={gasPedalZoneRef}
                className={`${styles.gasPedal} ${enabled && styles.enabled} ${isGasPedalPressed ? styles.pressed : ""}`}
            >
                <GasPedalIcon size={64} className={styles.gasPedalIcon} />
            </div>
            <div className={`${styles.fireButtonsContainer} ${enabled && styles.enabled}`}>
                <div ref={fireButton1ZoneRef} className={styles.fireButton}>
                    <IconRocket size={32} className={styles.fireIcon} />
                </div>
                <div ref={fireButton2ZoneRef} className={styles.fireButton}>
                    <IconBomb size={32} className={styles.fireIcon} />
                </div>
                <div ref={fireButton3ZoneRef} className={styles.fireButton}>
                    <IconBolt size={32} className={styles.fireIcon} />
                </div>
            </div>
        </>
    );
}
