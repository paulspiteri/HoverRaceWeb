import { useEffect } from "react";
import type { GameInstanceAPI } from "@/interop/gameInterop.ts";

export const useGameWindowSize = (gameInstance: GameInstanceAPI | undefined, canvas: HTMLCanvasElement | null) => {
    useEffect(() => {
        function updateCanvasSize(evt: Event | undefined) {
            const rect = canvas!.getBoundingClientRect();
            const dpr = window.devicePixelRatio || 1;
            const w = rect.width * dpr;
            const h = rect.height * dpr;
            gameInstance!.setWindowSize(w, h);

            if (evt?.type !== "resize") {
                window.dispatchEvent(new Event("resize"));
            }

            // old iphone 'fullscreen' fix
            //const isIOS = /iPad|iPhone/.test(navigator.userAgent);
            //if (isIOS) {
            //    canvas!.style.top = "85px";
            //document.getElementById("trackselection").style.top = "85px";
            //window.scrollTo(0, 85);
            //}
        }

        if (!gameInstance || !canvas) return;

        updateCanvasSize(undefined);

        window.addEventListener("resize", updateCanvasSize);
        window.addEventListener("orientationchange", updateCanvasSize);
        window.addEventListener("fullscreenchange", updateCanvasSize);
        return () => {
            window.removeEventListener("resize", updateCanvasSize);
            window.removeEventListener("orientationchange", updateCanvasSize);
            window.removeEventListener("fullscreenchange", updateCanvasSize);
        };
    }, [canvas, gameInstance]);
};
