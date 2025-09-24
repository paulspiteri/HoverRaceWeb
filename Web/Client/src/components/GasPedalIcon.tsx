interface GasPedalIconProps {
    size?: number;
    className?: string;
}

export const GasPedalIcon = ({ size = 24, className }: GasPedalIconProps) => (
    <svg
        width={size}
        height={size}
        viewBox="0 0 24 24"
        fill="none"
        stroke="currentColor"
        strokeWidth="2"
        className={className}
    >
        <path d="M8 20V8a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v12" />
        <path d="M6 16h12v4H6z" />
        <path d="M10 12h4" />
    </svg>
);