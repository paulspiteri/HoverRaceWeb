/**
 * Detects if the current device is a mobile device based on pointer type
 * @returns true if mobile device, false if desktop
 */
export function isMobileDevice(): boolean {
    return window.matchMedia("(pointer: coarse)").matches;
}
