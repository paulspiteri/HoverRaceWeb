var canvasElement = document.getElementById("canvas");
var outputElement = document.getElementById("output");
if (outputElement) outputElement.value = ""; // clear browser cache

// As a default initial behavior, pop up an alert when webgl context is lost. To make your
// application robust, you may want to override this behavior before shipping!
// See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
canvasElement.addEventListener(
    "webglcontextlost",
    (e) => {
        alert("WebGL context lost. You will need to reload the page.");
        e.preventDefault();
    },
    false,
);

var Module = (async () => {
    Module = await HoverRace({
        noInitialRun: true,
        print(...args) {
            console.log(...args);
        },
        canvas: canvasElement,
        totalDependencies: 0,
        monitorRunDependencies(left) {
            this.totalDependencies = Math.max(this.totalDependencies, left);
            // Module.setStatus(
            //     left
            //         ? "Preparing... (" + (this.totalDependencies - left) + "/" + this.totalDependencies + ")"
            //         : "All downloads complete.",
            // );
        },
    });
})();
window.onerror = () => {
    console.error("[post-exception status] ");
};

function updateCanvasSize(evt) {
    const container = document.getElementById("fullscreen-container");
    const rect = container.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    const w = rect.width * dpr;
    const h = rect.height * dpr;
    Module.ccall("ChangeWindowSize", null, ["number", "number"], [w, h]);

    if (evt?.type !== "resize") {
        window.dispatchEvent(new Event("resize"));
    }

    const isIOS = /iPad|iPhone/.test(navigator.userAgent);
    if (isIOS) {
        canvasElement.style.top = "85px";
        document.getElementById("trackselection").style.top = "85px";
        window.scrollTo(0, 85);
    }
}

function initVirtualJoysticks() {
    const joystickZone = document.getElementById("joystick-zone");
    const movementManager = nipplejs.create({
        zone: joystickZone,
        mode: "static",
        position: { left: "50%", top: "50%" },
        color: "blue",
        size: 120,
    });
    joystickZone.addEventListener("touchstart", (e) => e.preventDefault(), { passive: false });
    joystickZone.addEventListener("touchmove", (e) => e.preventDefault(), { passive: false });

    const weaponJoystickZone = document.getElementById("weapon-joystick-zone");
    const weaponManager = nipplejs.create({
        zone: weaponJoystickZone,
        mode: "static",
        position: { left: "50%", top: "50%" },
        color: "red",
        size: 120,
    });
    weaponJoystickZone.addEventListener("touchstart", (e) => e.preventDefault(), { passive: false });
    weaponJoystickZone.addEventListener("touchmove", (e) => e.preventDefault(), { passive: false });

    let currentMovementKeys = new Set();
    movementManager.on("move", function (evt, data) {
        currentMovementKeys.forEach((key) => {
            simulateKeyUp(key);
        });
        currentMovementKeys.clear();

        const angle = data.angle.degree;
        const force = data.force;

        if (force > 0.1) {
            let keys = [];
            if (angle >= 120 && angle < 240) {
                keys.push("ArrowLeft");
            }
            if (angle >= 300 || angle < 60) {
                keys.push("ArrowRight");
            }
            if (force > 0.3) {
                if (angle >= 60 && angle < 120) {
                    keys.push("ArrowUp");
                }

                if (angle >= 240 && angle < 300) {
                    keys.push("ArrowDown");
                }
            }

            keys.forEach((key) => {
                simulateKeyDown(key);
                currentMovementKeys.add(key);
            });
        }
    });

    movementManager.on("end", function () {
        currentMovementKeys.forEach((key) => {
            simulateKeyUp(key);
        });
        currentMovementKeys.clear();
    });

    let currentWeaponKeys = new Set();
    weaponManager.on("move", function (evt, data) {
        currentWeaponKeys.forEach((key) => {
            simulateKeyUp(key);
        });
        currentWeaponKeys.clear();

        const angle = data.angle.degree;
        const force = data.force;

        if (force > 0.9) {
            let keys = [];

            // UP = Tab (switch weapon)
            if (angle >= 45 && angle < 135) {
                keys.push("Tab");
            }

            // DOWN = Ctrl (fire weapon)
            if (angle >= 225 && angle < 315) {
                keys.push("ControlLeft");
            }

            keys.forEach((key) => {
                simulateKeyDown(key);
                currentWeaponKeys.add(key);
            });
        }
    });

    weaponManager.on("start", function () {
        simulateKeyDown("ShiftLeft");
    });

    weaponManager.on("end", function () {
        simulateKeyUp("ShiftLeft");

        currentWeaponKeys.forEach((key) => {
            simulateKeyUp(key);
        });
        currentWeaponKeys.clear();
    });
}

function simulateKeyDown(key) {
    const event = new KeyboardEvent("keydown", {
        key: key,
        code: key,
        bubbles: true,
        cancelable: true,
    });
    document.getElementById("canvas").dispatchEvent(event);
}

function simulateKeyUp(key) {
    const event = new KeyboardEvent("keyup", {
        key: key,
        code: key,
        bubbles: true,
        cancelable: true,
    });
    document.getElementById("canvas").dispatchEvent(event);
}

window.addEventListener("load", function () {
    initVirtualJoysticks();
});

window.addEventListener("resize", updateCanvasSize);
window.addEventListener("orientationchange", updateCanvasSize);
window.addEventListener("fullscreenchange", updateCanvasSize);

function receiveGameData(playerId, binaryData) {
    // Allocate memory in Emscripten heap
    const dataPtr = Module._malloc(binaryData.length);
    Module.HEAPU8.set(binaryData, dataPtr);

    // Call C++ function
    Module._ReceivePeerMessage(playerId, dataPtr, binaryData.length);

    // Free the allocated memory
    Module._free(dataPtr);
}

function setPlayerStatus(playerId, isConnected, minLatency, avgLatency) {
    Module._SetPeerStatus(playerId, isConnected, minLatency, avgLatency);
}
