import { Group, ActionIcon, Title } from "@mantine/core";
import { IconHome } from "@tabler/icons-react";
import { useNavigate } from "react-router-dom";
import { ConnectionStatus } from "./ConnectionStatus";

interface HeaderProps {
    isMobile?: boolean;
}

export const Header: React.FC<HeaderProps> = ({ isMobile = false }) => {
    const navigate = useNavigate();

    return (
        <Group justify={isMobile ? "space-between" : "center"} align="center">
            <Group gap={isMobile ? "xs" : "sm"} align="center">
                <ActionIcon
                    variant="subtle"
                    onClick={() => navigate("/")}
                    size={isMobile ? "lg" : "xl"}
                    title="Go to Home"
                >
                    <IconHome size={isMobile ? 20 : 24} />
                </ActionIcon>
                <Title
                    order={isMobile ? 2 : 1}
                    size={isMobile ? "h3" : "h1"}
                    style={{
                        background: "linear-gradient(45deg, #1976d2, #9c27b0)",
                        WebkitBackgroundClip: "text",
                        WebkitTextFillColor: "transparent",
                        cursor: "pointer",
                    }}
                    onClick={() => navigate("/")}
                >
                    HoverRace
                </Title>
            </Group>
            {isMobile && <ConnectionStatus />}
        </Group>
    );
};