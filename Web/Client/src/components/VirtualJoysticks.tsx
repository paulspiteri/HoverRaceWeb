import { useEffect, useRef } from "react";
import nipplejs from "nipplejs";
import styles from "./VirtualJoysticks.module.css";

interface VirtualJoysticksProps {
    canvasElement: HTMLCanvasElement | null;
    enabled: boolean;
}

const useVirtualJoysticks = ({
    canvasElement,
    enabled,
    joystickZone,
    weaponJoystickZone,
}: VirtualJoysticksProps & { joystickZone: HTMLDivElement | null; weaponJoystickZone: HTMLDivElement | null }) => {
    useEffect(() => {
        if (!enabled || !canvasElement || !joystickZone || !weaponJoystickZone) return;

        const currentMovementKeys = new Set<string>();
        const currentWeaponKeys = new Set<string>();

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

        // Movement joystick (left side)
        const movementManager = nipplejs.create({
            zone: joystickZone,
            mode: "static",
            position: { left: "50%", top: "50%" },
            color: "blue",
            size: 120,
        });

        joystickZone.addEventListener("touchstart", (e) => e.preventDefault(), { passive: false });
        joystickZone.addEventListener("touchmove", (e) => e.preventDefault(), { passive: false });

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

        // Weapon joystick (right side)
        const weaponManager = nipplejs.create({
            zone: weaponJoystickZone,
            mode: "static",
            position: { left: "50%", top: "50%" },
            color: "red",
            size: 120,
        });

        weaponJoystickZone.addEventListener("touchstart", (e) => e.preventDefault(), { passive: false });
        weaponJoystickZone.addEventListener("touchmove", (e) => e.preventDefault(), { passive: false });

        weaponManager.on("move", (_, data) => {
            currentWeaponKeys.forEach((key) => {
                simulateKeyUp(key);
            });
            currentWeaponKeys.clear();

            const angle = data.angle.degree;
            const force = data.force;

            if (force > 0.9) {
                const keys: string[] = [];

                // UP = Tab (switch weapon)
                if (angle >= 45 && angle < 135) {
                    keys.push("Tab");
                }

                // DOWN = Ctrl (fire weapon)
                if (angle >= 225 && angle < 315) {
                    keys.push("ControlLeft");
                }

                keys.forEach((key) => {
                    simulateKeyDown(key);
                    currentWeaponKeys.add(key);
                });
            }
        });

        weaponManager.on("start", () => {
            simulateKeyDown("ShiftLeft");
        });

        weaponManager.on("end", () => {
            simulateKeyUp("ShiftLeft");

            currentWeaponKeys.forEach((key) => {
                simulateKeyUp(key);
            });
            currentWeaponKeys.clear();
        });

        return () => {
            movementManager.destroy();
            weaponManager.destroy();
        };
    }, [enabled, canvasElement, joystickZone, weaponJoystickZone]);

    return {
        isEnabled: enabled && !!canvasElement,
    };
};

export function VirtualJoysticks({ canvasElement, enabled }: VirtualJoysticksProps) {
    const joystickZoneRef = useRef<HTMLDivElement>(null);
    const weaponJoystickZoneRef = useRef<HTMLDivElement>(null);

    useVirtualJoysticks({
        canvasElement,
        enabled,
        joystickZone: joystickZoneRef.current,
        weaponJoystickZone: weaponJoystickZoneRef.current,
    });

    return (
        <>
            <div ref={joystickZoneRef} className={`${styles.joystickLeft} ${enabled && styles.enabled}`}></div>
            <div ref={weaponJoystickZoneRef} className={`${styles.joystickRight} ${enabled && styles.enabled}`}>
                <div className={styles.labelTop}>↑ Select</div>
                <div className={styles.labelBottom}>↓ Fire</div>
            </div>
        </>
    );
}
