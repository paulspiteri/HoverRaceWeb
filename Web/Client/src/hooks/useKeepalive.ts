import { useEffect } from "react";

export const useKeepalive = (baseUrl: string, connectionId: string | undefined) => {
    useEffect(() => {
        if (!connectionId) return;

        const keepaliveInterval = setInterval(async () => {
            try {
                await fetch(`${baseUrl}/keepalive`, {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ connectionId }),
                });
            } catch (error) {
                console.error("Keepalive failed:", error);
            }
        }, 15000);

        return () => clearInterval(keepaliveInterval);
    }, [connectionId, baseUrl]);
};
