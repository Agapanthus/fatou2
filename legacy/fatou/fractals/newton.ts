import * as f from "../global";
import { Window } from "../gui";
// @ts-ignore
import newton_smooth from "../shaders/newton/smooth.fs";
// @ts-ignore
import vertex_default from "../shaders/vertex/default.vs";
import { Texture } from "../texture";

import { Fractal } from "./abstractFractal";

/*
export class Newton extends Fractal {
    constructor() { super(); }

    public create(ready: () => void, gl: WebGLRenderingContext) {
        this.prog = new Program(vertex_default, newton_smooth, gl,
                                [
                                    "screenTexture", "c", "zoom", "pos",
                                    "coe" //, "coec", "iter"
                                ],
                                [ "aPos", "aTexCoords" ]);

        this.prog.set2f("c", Math.pow(2, -40), 3.0);
        this.prog.seti("screenTexture", 0);

        this.win = new Window(0, 0, 240, 600, "Newton Parameters", true, true);
        for (let i = 0; i < 45; i++) {
            this.win.addSlider("P" + i, this.coef[i], -5, 5, 3, (v) => {
                this.coefT[i] = v;
                this.invalidate();
            })
        }

        this.loadPreset(this.preset);

        this.tex = new Texture("/dist/hue.png", (succ: boolean) => {
            if (!succ)
                f.crash("Texture load failed");
            ready();
        }, gl);
    };
    
    public precision() {
        return 4;
    }

    public update() {
        this.tex.bind();
        this.prog.set2f("c", Math.pow(2, -40), 3.0);

        // TODO: Use elapsed time as dt!
        const dt = 0.1
        for (let i = 0; i < this.numParams; i++) {
            if (this.coef[i] === this.coefT[i])
                continue;
            if (Math.abs(this.coef[i] - this.coefT[i]) <= 0.01) {
                this.coef[i] = this.coefT[i]
            } else {
                this.coef[i] = this.coef[i] * (1 - dt) + this.coefT[i] * dt
            }
            this.dirty = true
        }
        this.prog.set1fv("coe", this.coef);
    };

    public defaults() { return Newton.presets[this.preset]; }

    public loadPreset(name: string) {
        this.dirty = true;
        this.preset = name;
        this.prog.use();

        for (let i = 0; i < this.numParams; i++)
            this.coef[i] = 0.0;

        for (let i = 0; i < this.numParams; i++) {
            if (i + "" in Newton.presets[this.preset].coefs)
                this.coef[i] = Newton.presets[this.preset].coefs[i + ""];

            this.win.setSlider("P" + i, this.coef[i]);
        }

        for (let i = 0; i < this.numParams; i++)
            this.coefT[i] = this.coef[i];

        this.prog.set1fv("coe", this.coef);
    }

    destroy() {
        this.win.destroy();
        this.prog.destroy();
        this.tex.destroy();
    }

    private tex: Texture;
    private numParams = 45;
    private coef = new Array<number>(this.numParams);
    private coefT = new Array<number>(this.numParams);
    private win: Window;
    public name = "Newton";
    private preset = "Default";

    public static presets = {
        "Default" : {"coefs" : {"0" : -2, "3": 1}, "zoom": 4},
        "Skaliks44": {
            "coefs": {"0": -1, "1": -1, "4": 20, "7": 100, "44": 20},
            "zoom": 1
        },
        "Micro": {
            "coefs": {"0": 0.681, "1": -1, "4": 11, "7": 44.74, "9": -97.336},
            "zoom": 0.2,
            "x": 0,
            "y": 0,
        },
        "Tethers": {
            "coefs": {"0": -2, "3": 1, "44": 0.0005},
            "zoom": 0.43,
            "x": -0.188,
            "y": -1.079,
        },
    };
}
*/
