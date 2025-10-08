import { useEffect, useRef } from "react";
import { Box } from "@mantine/core";
import * as THREE from "three";
import { useElectroCarModel } from "./useElectroCarModel";

interface DrivingCarProps {
    width?: number;
    height?: number;
    className?: string;
}

export function ElectroIntro({ width = 400, height = 75, className }: DrivingCarProps) {
    const mountRef = useRef<HTMLDivElement>(null);
    const animationIdRef = useRef<number>(undefined);
    const model = useElectroCarModel();

    useEffect(() => {
        if (!mountRef.current) return;
        if (mountRef.current.children.length > 0) return;
        if (!model) return;

        const scene = new THREE.Scene();
        const camera = new THREE.PerspectiveCamera(35, width / height, 0.1, 1000);
        camera.position.set(0, 0, 0);
        camera.lookAt(0, 0, 0);

        const renderer = new THREE.WebGLRenderer({
            antialias: true,
            alpha: true,
            powerPreference: "high-performance",
        });
        renderer.setSize(width, height);
        renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        renderer.setClearColor(0x000000, 0);
        renderer.outputColorSpace = THREE.SRGBColorSpace;
        renderer.shadowMap.enabled = true;
        renderer.shadowMap.type = THREE.PCFSoftShadowMap;

        mountRef.current.appendChild(renderer.domElement);

        model.scale.setScalar(0.8);
        model.position.set(0, 0, 0);
        model.rotation.set(0, 0, 0);
        scene.add(model);

        // Swooping camera animation (from GLObserver.cpp)
        const startTime = Date.now();
        const countdownStart = -11000; // Start at -11 seconds
        let lOrientation = 0;

        const animate = () => {
            const elapsed = Date.now() - startTime;
            const pTime = countdownStart + elapsed; // Count up from -11000 to 0

            let lDist = 3;

            if (pTime < 0) {
                const lFactor = (-pTime) * 2 / 3;
                const MR_2PI = Math.PI * 2;
                const rotateDegrees = lFactor * MR_2PI / 11000;
                lOrientation = rotateDegrees;
                lDist += lFactor / 1000;
            }

            // Calculate camera position (C++ uses X,Y for ground plane, Z for height)
            // THREE.js uses X,Z for ground plane, Y for height
            const cameraX = model.position.x - lDist * Math.cos(lOrientation);
            const cameraZ = model.position.z - lDist * Math.sin(lOrientation);
            const cameraY = 1.7;

            camera.position.set(cameraX, cameraY, cameraZ);
            camera.lookAt(model.position);

            renderer.render(scene, camera);
            animationIdRef.current = requestAnimationFrame(animate);
        };
        animate();

        const mountRefForDispose = mountRef.current;
        return () => {
            if (animationIdRef.current) {
                cancelAnimationFrame(animationIdRef.current);
                animationIdRef.current = undefined;
            }

            if (mountRefForDispose) {
                while (mountRefForDispose.firstChild) {
                    mountRefForDispose.removeChild(mountRefForDispose.firstChild);
                }
            }

            renderer.dispose();
            scene.clear();
        };
    }, [width, height, model]);

    return <Box ref={mountRef} w={width} h={height} className={className} />;
}
