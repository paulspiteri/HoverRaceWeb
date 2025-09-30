import { useState, useRef } from "react";

type GameScreenMode = "mini" | "maximized" | "fullscreen" | "hidden";

interface UsePinchGestureReturn {
    handleTouchStart: (e: React.TouchEvent) => void;
    handleTouchMove: (e: React.TouchEvent) => void;
    handleTouchEnd: () => void;
}

export function useGameScreenModePinchGesture(
    gameScreenMode: GameScreenMode,
    setGameScreenMode: (mode: GameScreenMode) => void,
    pinchInThreshold = 0.7,  // 30% reduction triggers mini mode
    pinchOutThreshold = 1.3   // 30% increase triggers maximized mode
): UsePinchGestureReturn {
    const [initialPinchDistance, setInitialPinchDistance] = useState<number | undefined>(undefined);
    const tapStartRef = useRef<{ x: number; y: number; time: number } | undefined>(undefined);

    const getDistance = (touch1: React.Touch, touch2: React.Touch) => {
        const dx = touch1.clientX - touch2.clientX;
        const dy = touch1.clientY - touch2.clientY;
        return Math.sqrt(dx * dx + dy * dy);
    };

    const handleTouchStart = (e: React.TouchEvent) => {
        if (e.touches.length === 2 && (gameScreenMode === "maximized" || gameScreenMode === "mini")) {
            const distance = getDistance(e.touches[0], e.touches[1]);
            setInitialPinchDistance(distance);
            tapStartRef.current = undefined;
        } else if (e.touches.length === 1) {
            // Track single tap for click detection
            tapStartRef.current = {
                x: e.touches[0].clientX,
                y: e.touches[0].clientY,
                time: Date.now()
            };
        }
    };

    const handleTouchMove = (e: React.TouchEvent) => {
        if (e.touches.length === 2 && initialPinchDistance !== undefined) {
            const currentDistance = getDistance(e.touches[0], e.touches[1]);

            // Pinch in: maximized -> mini
            if (gameScreenMode === "maximized" && currentDistance < initialPinchDistance * pinchInThreshold) {
                setGameScreenMode("mini");
                setInitialPinchDistance(undefined);
            }

            // Pinch out: mini -> maximized
            if (gameScreenMode === "mini" && currentDistance > initialPinchDistance * pinchOutThreshold) {
                setGameScreenMode("maximized");
                setInitialPinchDistance(undefined);
            }
        }
    };

    const handleTouchEnd = () => {
        // Handle single tap
        if (tapStartRef.current) {
            const tapDuration = Date.now() - tapStartRef.current.time;
            if (tapDuration < 300) { // Quick tap
                // Toggle screen mode
                if (gameScreenMode === "fullscreen") {
                    document.exitFullscreen();
                    setGameScreenMode("mini");
                } else if (gameScreenMode === "mini") {
                    setGameScreenMode("maximized");
                }
            }
            tapStartRef.current = undefined;
        }
        setInitialPinchDistance(undefined);
    };

    return { handleTouchStart, handleTouchMove, handleTouchEnd };
}