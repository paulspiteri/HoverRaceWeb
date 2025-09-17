import { useEffect } from "react";
import type { GameInstanceAPI } from "@/interop/gameInterop.ts";

export const useGameWindowSize = (gameInstance: GameInstanceAPI | undefined, canvas: HTMLCanvasElement | null) => {
    useEffect(() => {
        function updateCanvasSize() {
            if (!canvas || !gameInstance) return;

            const rect = canvas.getBoundingClientRect();
            const dpr = window.devicePixelRatio || 1;
            const w = rect.width * dpr;
            const h = rect.height * dpr;
            gameInstance.setWindowSize(w, h);
        }

        if (!gameInstance || !canvas) return;

        let resizeObserver: ResizeObserver | null = null;
        if (typeof ResizeObserver !== "undefined") {
            resizeObserver = new ResizeObserver(() => updateCanvasSize());
            resizeObserver.observe(canvas);
        }

        window.addEventListener("resize", updateCanvasSize);
        window.addEventListener("orientationchange", updateCanvasSize);
        window.addEventListener("fullscreenchange", updateCanvasSize);

        return () => {
            resizeObserver?.disconnect();
            window.removeEventListener("resize", updateCanvasSize);
            window.removeEventListener("orientationchange", updateCanvasSize);
            window.removeEventListener("fullscreenchange", updateCanvasSize);
        };
    }, [canvas, gameInstance]);
};
