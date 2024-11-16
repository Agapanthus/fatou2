import {
    EasyFractal,
    Parameter,
    ParameterNumeric,
    Uniform2fd,
    Uniform4f,
    UniformParameters
} from "./easyFractal";

export class Antifractal extends EasyFractal {
    constructor() {
        super(require("../shaders/antifractal/default.fs"), true, true);
    }

    easyCreate(): Promise<void> {
        return new Promise(res => {
            this.loadPreset("Start");
            res();
        });
    };

    public getParameters(): {[name: string]: Parameter} {
        return {
            "f1" : new ParameterNumeric("F1", -4, 4, [ 1 ], 2, true),
            "f2" : new ParameterNumeric("F2", 0, 10, [ 2 ], 2, true),
            "iterations" :
                new ParameterNumeric("Iterations", 0, 7, [ 1 ], 1, true),
            "contrast" : new ParameterNumeric("Contrast", 0, 1, [ 1 ], 10),
        };
    }

    public getUniforms(): UniformParameters[]{return [
        new Uniform4f("c", "contrast", "iterations", "f1", "f2"),
        // new Uniform4f("c2", "contrast", "iterations", "f1","f1")
    ]}

    destroy() {
        this.prog.destroy();
        super.destroy();
    }

    public getName() { return "Mandelbrot" }

    public static presets = {
        "Start" : {
            "zoom" : Math.pow(2, 4.6),
            "x": 0,
            "y": 0,
            "superSamples": 1,
        },
    }
}

/*
export class Antifractal1a extends Fractal {
    constructor() {
        super();
        this.create = (ready: () => void, gl: WebGLRenderingContext) => {
            this.prog = new Program(vertex_2default, fs_antifractal_float, gl,
                                    [ "zoom", "pos", "mods", "divs" ],
                                    [ "aPos", "aTexCoords" ], true);
            this.prog.set2f("mods", 65536, 65536);
            this.prog.set2f("divs", 100.0, 100.0);
            this.prog.setRect("aPos", 0.0, 0.0, 1.0, 1.0);
            this.prog.setRect("aTexCoords", 0.0, 0.0, 1.0, 1.0);

            ready();
        };
        this.update = () => {};
        this.defaults = () => {
            return { "zoom": 8000 }
        };
    }
}

export class Antifractal1b extends Fractal {
    constructor() {
        super();
        this.create = (ready: () => void, gl: WebGLRenderingContext) => {
            this.prog = new Program(vertex_2default, fs_antifractal_float, gl,
                                    [ "zoom", "pos", "mods", "divs" ],
                                    [ "aPos", "aTexCoords" ], true);
            this.prog.set2f("mods", 65536 / 4, 65536 / 4);
            this.prog.set2f("divs", 50.0, 100.0);
            this.prog.setRect("aPos", 0.0, 0.0, 1.0, 1.0);
            this.prog.setRect("aTexCoords", 0.0, 0.0, 1.0, 1.0);

            ready();
        };
        this.update = () => {};
        this.defaults = () => {
            return { "zoom": 5000 }
        };
    }
}

export class Antifractal1c extends Fractal {
    constructor() {
        super();
        this.create = (ready: () => void, gl: WebGLRenderingContext) => {
            this.prog = new Program(vertex_2default, fs_antifractal_float, gl,
                                    [ "zoom", "pos", "mods", "divs" ],
                                    [ "aPos", "aTexCoords" ], true);
            this.prog.set2f("mods", 65536 / 4, 65536 / 4);
            this.prog.set2f("divs", 50.0, 100.0);
            this.prog.setRect("aPos", 0.0, 0.0, 1.0, 1.0);
            this.prog.setRect("aTexCoords", 0.0, 0.0, 1.0, 1.0);

            ready();
        };
        this.update = () => {};
        this.defaults = () => {
            return { "zoom": 5000 * 500, "superSamples": 6 }
        };
    }
}*/