import { useEffect, useRef } from "react";
import { Box } from "@mantine/core";
import * as THREE from "three";
import { OBJLoader } from "three/addons/loaders/OBJLoader.js";

interface ElectroCarProps {
    width?: number;
    height?: number;
    className?: string;
}

export function ElectroCar({ width = 90, height = 50, className }: ElectroCarProps) {
    const mountRef = useRef<HTMLDivElement>(null);
    const sceneRef = useRef<THREE.Scene>(undefined);
    const rendererRef = useRef<THREE.WebGLRenderer>(undefined);
    const animationIdRef = useRef<number>(undefined);

    useEffect(() => {
        if (!mountRef.current) return;

        // Prevent double initialization in Strict Mode
        if (mountRef.current.children.length > 0) {
            return;
        }

        const scene = new THREE.Scene();
        sceneRef.current = scene;

        const camera = new THREE.PerspectiveCamera(35, width / height, 0.1, 1000);
        camera.position.set(0, 2, 4);
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
        rendererRef.current = renderer;

        mountRef.current.appendChild(renderer.domElement);

        const ambientLight = new THREE.AmbientLight(0x404040, 0.6);
        scene.add(ambientLight);

        const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
        directionalLight.position.set(5, 5, 5);
        directionalLight.castShadow = true;
        scene.add(directionalLight);

        const textureLoader = new THREE.TextureLoader();
        const objLoader = new OBJLoader();

        // First, let's try loading just the OBJ without materials to see if it renders
        objLoader.load("/electro_car/electro_car_seq0.obj", (object) => {
            object.scale.setScalar(1.0);
            object.position.set(0, -0.3, 0);
            object.rotation.set(-0.1, Math.PI * 0.75, 0);

            // Apply basic materials with textures manually
            const textureMap = {
                material_20: "/electro_car/texture_20.png",
                material_21: "/electro_car/texture_21.png",
                material_22: "/electro_car/texture_22.png",
                material_23: "/electro_car/texture_23.png",
                material_24: "/electro_car/texture_24.png",
                material_25: "/electro_car/texture_25.png",
                material_26: "/electro_car/texture_26.png",
            };

            let meshCount = 0;
            object.traverse((child) => {
                if (child instanceof THREE.Mesh) {
                    meshCount++;
                    child.castShadow = true;
                    child.receiveShadow = true;

                    console.log(`Mesh ${meshCount}:`, {
                        name: child.name,
                        material: child.material,
                        materialName: child.material?.name,
                        materialType: child.material?.type,
                        geometry: child.geometry,
                    });

                    if (Array.isArray(child.material)) {
                        console.log(
                            `Mesh has ${child.material.length} materials:`,
                            child.material.map((m) => m.name),
                        );

                        // Apply textures to each material in the array
                        const newMaterials = child.material.map((mat) => {
                            if (mat.name && textureMap[mat.name as keyof typeof textureMap]) {
                                const texturePath = textureMap[mat.name as keyof typeof textureMap];
                                console.log(`Loading texture: ${texturePath} for material: ${mat.name}`);

                                const texture = textureLoader.load(
                                    texturePath,
                                    () => console.log(`✅ Texture loaded: ${texturePath}`),
                                    undefined,
                                    (err) => console.error(`❌ Failed to load texture: ${texturePath}`, err),
                                );

                                // Enhance color vibrancy
                                texture.colorSpace = THREE.SRGBColorSpace;
                                texture.minFilter = THREE.LinearFilter;
                                texture.magFilter = THREE.LinearFilter;

                                return new THREE.MeshBasicMaterial({
                                    map: texture,
                                    side: THREE.DoubleSide,
                                });
                            }
                        });

                        child.material = newMaterials;
                    }
                }
            });

            scene.add(object);

            // Always animate, but use a ref for disabled state
            const animate = () => {
                object.rotation.y += 0.015;
                renderer.render(scene, camera);
                animationIdRef.current = requestAnimationFrame(animate);
            };
            animate();
        });

        const mountRefForDispose = mountRef.current;
        return () => {
            if (animationIdRef.current) {
                cancelAnimationFrame(animationIdRef.current);
                animationIdRef.current = undefined;
            }

            // Clear the mount point completely
            if (mountRefForDispose) {
                while (mountRefForDispose.firstChild) {
                    mountRefForDispose.removeChild(mountRefForDispose.firstChild);
                }
            }

            if (renderer) {
                renderer.dispose();
            }
            if (scene) {
                scene.clear();
            }

            sceneRef.current = undefined;
            rendererRef.current = undefined;
        };
    }, [width, height]);

    return <Box ref={mountRef} w={width} h={height} className={className} />;
}
