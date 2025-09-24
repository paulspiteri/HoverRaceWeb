import { useEffect, useRef, useState } from "react";
import nipplejs from "nipplejs";
import styles from "./MobileInput.module.css";
import { GasPedalIcon } from "./GasPedalIcon";
import { IconRocket } from "@tabler/icons-react";

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

        const simulateKeyDown = (key: string) => {
            if (!canvasElement) return;
            const event = new KeyboardEvent("keydown", {
                key,
                code: key,
                bubbles: true,
                cancelable: true,
            });
            canvasElement.dispatchEvent(event);
        };

        const simulateKeyUp = (key: string) => {
            if (!canvasElement) return;
            const event = new KeyboardEvent("keyup", {
                key,
                code: key,
                bubbles: true,
                cancelable: true,
            });
            canvasElement.dispatchEvent(event);
        };

        const handleTouchStart = (e: TouchEvent) => {
            e.preventDefault();
            setIsGasPedalPressed(true);
            simulateKeyDown("ShiftLeft");
        };

        const handleTouchEnd = (e: TouchEvent) => {
            e.preventDefault();
            setIsGasPedalPressed(false);
            simulateKeyUp("ShiftLeft");
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
}: {
    canvasElement: HTMLCanvasElement | null;
    enabled: boolean;
    fireButton1Zone: HTMLDivElement | null;
    fireButton2Zone: HTMLDivElement | null;
    fireButton3Zone: HTMLDivElement | null;
}) => {
    useEffect(() => {
        if (!enabled || !canvasElement || !fireButton1Zone || !fireButton2Zone || !fireButton3Zone) return;

        const simulateKeyDown = (key: string) => {
            if (!canvasElement) return;
            const event = new KeyboardEvent("keydown", {
                key,
                code: key,
                bubbles: true,
                cancelable: true,
            });
            canvasElement.dispatchEvent(event);
        };

        const simulateKeyUp = (key: string) => {
            if (!canvasElement) return;
            const event = new KeyboardEvent("keyup", {
                key,
                code: key,
                bubbles: true,
                cancelable: true,
            });
            canvasElement.dispatchEvent(event);
        };

        const handleTouchStart = (e: TouchEvent) => {
            e.preventDefault();
            const buttonZone = e.currentTarget as HTMLDivElement;
            buttonZone.classList.add(styles.pressed);
            simulateKeyDown("ControlLeft");
            setTimeout(() => {
                buttonZone.classList.remove(styles.pressed);
                simulateKeyUp("ControlLeft"); // Immediate release for tap
            }, 150);
        };

        fireButton1Zone.addEventListener("touchstart", handleTouchStart, { passive: false });
        fireButton2Zone.addEventListener("touchstart", handleTouchStart, { passive: false });
        fireButton3Zone.addEventListener("touchstart", handleTouchStart, { passive: false });

        return () => {
            fireButton1Zone.removeEventListener("touchstart", handleTouchStart);
            fireButton2Zone.removeEventListener("touchstart", handleTouchStart);
            fireButton3Zone.removeEventListener("touchstart", handleTouchStart);
        };
    }, [enabled, canvasElement, fireButton1Zone, fireButton2Zone, fireButton3Zone]);
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

        const simulateKeyDown = (key: string) => {
            if (!canvasElement) return;
            const event = new KeyboardEvent("keydown", {
                key,
                code: key,
                bubbles: true,
                cancelable: true,
            });
            canvasElement.dispatchEvent(event);
        };

        const simulateKeyUp = (key: string) => {
            if (!canvasElement) return;
            const event = new KeyboardEvent("keyup", {
                key,
                code: key,
                bubbles: true,
                cancelable: true,
            });
            canvasElement.dispatchEvent(event);
        };

        const movementManager = nipplejs.create({
            zone: joystickZone,
            mode: "static",
            position: { left: "50%", top: "50%" },
            color: "blue",
            size: 120,
        });

        // joystickZone.addEventListener("touchstart", (e) => e.preventDefault(), { passive: false });
        // joystickZone.addEventListener("touchmove", (e) => e.preventDefault(), { passive: false });

        movementManager.on("move", (_, data) => {
            currentMovementKeys.forEach((key) => {
                simulateKeyUp(key);
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
                    simulateKeyDown(key);
                    currentMovementKeys.add(key);
                });
            }
        });

        movementManager.on("end", () => {
            currentMovementKeys.forEach((key) => {
                simulateKeyUp(key);
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
                <div ref={fireButton2ZoneRef} className={styles.fireButton}></div>
                <div ref={fireButton3ZoneRef} className={styles.fireButton}></div>
            </div>
        </>
    );
}
