import { useEffect, useState } from "react";
import * as THREE from "three";
import { OBJLoader } from "three/addons/loaders/OBJLoader.js";

// Shared loaded model template
let loadedModelTemplate: THREE.Group | null = null;
const loadingCallbacks: Array<(model: THREE.Group) => void> = [];

export function useElectroCarModel() {
    const [model, setModel] = useState<THREE.Group | null>(null);

    useEffect(() => {
        // If we already have a loaded template, clone it immediately
        if (loadedModelTemplate) {
            setModel(loadedModelTemplate.clone());
            return;
        }

        // Add our callback to the queue
        const callback = (template: THREE.Group) => {
            setModel(template.clone());
        };
        loadingCallbacks.push(callback);

        // If already loading, we're done (callback will fire when loading completes)
        if (loadingCallbacks.length > 1) {
            return;
        }

        // We're the first, start loading
        const textureLoader = new THREE.TextureLoader();
        const objLoader = new OBJLoader();

        const textureMap = {
            material_20: "/electro_car/texture_20.png",
            material_21: "/electro_car/texture_21.png",
            material_22: "/electro_car/texture_22.png",
            material_23: "/electro_car/texture_23.png",
            material_24: "/electro_car/texture_24.png",
            material_25: "/electro_car/texture_25.png",
            material_26: "/electro_car/texture_26.png",
        };

        objLoader.load("/electro_car/electro_car_seq0.obj", (object) => {
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
                        const newMaterials = child.material.map((mat) => {
                            if (mat.name && textureMap[mat.name as keyof typeof textureMap]) {
                                const texturePath = textureMap[mat.name as keyof typeof textureMap];

                                const texture = textureLoader.load(
                                    texturePath,
                                    () => console.log(`✅ Texture loaded: ${texturePath}`),
                                    undefined,
                                    (err) => console.error(`❌ Failed to load texture: ${texturePath}`, err),
                                );

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

            // Store as template
            loadedModelTemplate = object;

            // Notify all waiting callbacks
            loadingCallbacks.forEach(cb => cb(object));
            loadingCallbacks.length = 0;
        });
    }, []);

    return model;
}
