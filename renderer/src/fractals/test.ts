import {
    EasyFractal,
    Parameter,
    ParameterNumeric,
    Uniform4f,
    UniformParameters,

} from "./easyFractal";

export class TestFractal extends EasyFractal {
    constructor() { super(require("../shaders/mandelbrot/achat.fs"), false); }

    easyCreate(): Promise<void> {
        return new Promise(res => {
            this.loadPreset("Achat");
            res();
        });
    };

    public getParameters(): {[name: string]: Parameter} {
        return {
            "radius" : new ParameterNumeric("Escape Radius", 0, 4, [ 4 ]),
            "contrast" : new ParameterNumeric("Contrast", 0, 6, [ 3, 0 ], 2),
            "smoothing" : new ParameterNumeric("Smoothing", -4, 4, [ 1, 0 ], 3),
            "iterations" :
                new ParameterNumeric("Iterations", 0, 30, [ 1000 ], 3, true),
        };
    }

    public getUniforms():
        UniformParameters[]{return [ new Uniform4f("c", "radius", "contrast",
                                                   "smoothing", "iterations") ]}

    destroy() {
        this.prog.destroy();
        super.destroy();
    }

    public getName() { return "Test" }

    public static presets = {
        "Achat" : {
            "zoom" : Math.pow(2, -5.6),
            "x": -1.431124,
            "y": 0,
            "radius": 4,
            "iterations": 1000,
        },
    }
}