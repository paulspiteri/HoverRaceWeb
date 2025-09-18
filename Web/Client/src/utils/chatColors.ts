export const getPlayerColor = (senderId: string, currentConnectionId?: string) => {
    // If it's the current user, use blue
    if (senderId === currentConnectionId) {
        return { color: "blue.3", text: "white" };
    }

    // Generate a consistent color based on the senderId
    const colorPairs = [
        { color: "green.3", text: "white" },
        { color: "red.3", text: "white" },
        { color: "yellow.3", text: "dark" },
        { color: "purple.3", text: "white" },
        { color: "orange.3", text: "white" },
        { color: "pink.3", text: "white" },
        { color: "indigo.3", text: "white" },
        { color: "teal.3", text: "white" },
    ];

    // Use a simple hash to get consistent color for same senderId
    let hash = 0;
    for (let i = 0; i < senderId.length; i++) {
        hash = ((hash << 5) - hash + senderId.charCodeAt(i)) & 0xffffffff;
    }
    return colorPairs[Math.abs(hash) % colorPairs.length];
};

export const formatTimestamp = (timestamp: Date) => {
    return new Date(timestamp).toLocaleTimeString([], {
        hour: "2-digit",
        minute: "2-digit",
    });
};