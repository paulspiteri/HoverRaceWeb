import { useState, useEffect, useRef } from "react";

interface CanvasSize {
    width: number;
    height: number;
}

interface UseCanvasResizeReturn {
    canvasSize: CanvasSize;
    isResizing: boolean;
    handleResizeStart: (e: React.MouseEvent) => void;
}

export function useCanvasResize(
    initialWidth = 350,
    initialHeight = 262,
    minWidth = 175,
    minHeight = 131,
    maxWidthPercent = 0.95,
    maxHeightPercent = 0.95
): UseCanvasResizeReturn {
    const getInitialSize = (): CanvasSize => {
        const isMobile = window.innerWidth < 768 || !window.matchMedia('(pointer: fine)').matches;
        if (isMobile) {
            return {
                width: initialWidth / 2,
                height: initialHeight / 2
            };
        }
        return {
            width: initialWidth,
            height: initialHeight
        };
    };

    const [canvasSize, setCanvasSize] = useState<CanvasSize>(getInitialSize());
    const [isResizing, setIsResizing] = useState(false);
    const resizeStartRef = useRef({ x: 0, y: 0, width: 0, height: 0 });

    const handleResizeStart = (e: React.MouseEvent) => {
        e.preventDefault();
        e.stopPropagation();
        setIsResizing(true);
        resizeStartRef.current = {
            x: e.clientX,
            y: e.clientY,
            width: canvasSize.width,
            height: canvasSize.height,
        };
    };

    useEffect(() => {
        if (!isResizing) return;

        const handleMouseMove = (e: MouseEvent) => {
            const deltaX = resizeStartRef.current.x - e.clientX;
            const deltaY = resizeStartRef.current.y - e.clientY;

            const maxWidth = window.innerWidth * maxWidthPercent;
            const maxHeight = window.innerHeight * maxHeightPercent;

            const newWidth = Math.max(minWidth, Math.min(maxWidth, resizeStartRef.current.width + deltaX));
            const newHeight = Math.max(minHeight, Math.min(maxHeight, resizeStartRef.current.height + deltaY));

            setCanvasSize({ width: newWidth, height: newHeight });
        };

        const handleMouseUp = () => {
            setIsResizing(false);
        };

        document.addEventListener('mousemove', handleMouseMove);
        document.addEventListener('mouseup', handleMouseUp);

        return () => {
            document.removeEventListener('mousemove', handleMouseMove);
            document.removeEventListener('mouseup', handleMouseUp);
        };
    }, [isResizing, minWidth, minHeight, maxWidthPercent, maxHeightPercent]);

    return { canvasSize, isResizing, handleResizeStart };
}