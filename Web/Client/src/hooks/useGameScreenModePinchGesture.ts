import { useState } from "react";

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

    const getDistance = (touch1: React.Touch, touch2: React.Touch) => {
        const dx = touch1.clientX - touch2.clientX;
        const dy = touch1.clientY - touch2.clientY;
        return Math.sqrt(dx * dx + dy * dy);
    };

    const handleTouchStart = (e: React.TouchEvent) => {
        if (e.touches.length === 2 && (gameScreenMode === "maximized" || gameScreenMode === "mini")) {
            const distance = getDistance(e.touches[0], e.touches[1]);
            setInitialPinchDistance(distance);
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
        setInitialPinchDistance(undefined);
    };

    return { handleTouchStart, handleTouchMove, handleTouchEnd };
}