// @ts-ignore
import * as mandelbrot_smooth from "../shaders/mandelbrot/smooth.fs"
import {
    EasyFractal,
    Parameter,
    ParameterNumeric,
    Uniform4f,
    UniformParameters,

} from "./easyFractal";

export class Mandelbrot extends EasyFractal {
    constructor() { super(mandelbrot_smooth, true); }

    easyCreate(): Promise<void> {
        return new Promise(res => {

            this.loadPreset("Smooth");
            res();
        });
    };

    public getParameters(): {[name: string]: Parameter} {
        return {
            "radius" : new ParameterNumeric("Escape Radius", 0, 4, [ 4 ]),
            "smoothing" : new ParameterNumeric("Smoothing", -4, 4, [ 1, 0 ], 3),
            "iterations" :
                new ParameterNumeric("Iterations", 0, 30, [ 1000 ], 3, true),

            "contrast" : new ParameterNumeric("Contrast", 0, 6, [ 3, 0 ], 2),
            "phase" : new ParameterNumeric("Phase", 0, 2 * Math.PI, [ 0 ]),
            "shift" : new ParameterNumeric("Shift", 0, 2/3 * Math.PI, [ 1 ]),
            "iGamma" :
                new ParameterNumeric("Inverse Gamma", 0, 1, [ 0.7 ]),
        };
    }

    public getUniforms(): UniformParameters[]{return [
        new Uniform4f("c", "radius", "radius", "smoothing", "iterations"),
        new Uniform4f("c2", "contrast", "phase", "shift", "iGamma")
    ]}

    destroy() {
        this.prog.destroy();
        super.destroy();
    }

    public getName() { return "Mandelbrot" }

    public static presets = {
        "Smooth" : {
            "zoom" : 3.3,
            "x": -0.6,
            "y": 0,
            "radius": 4,
            "iterations": 1000,
            "superSamples": 2,
        },
        "Tortoise": {
            "zoom": Math.pow(2, -35.3),
            "x": -1.76890224278382635,
            "y": -0.00249310263813761,
            "iterations": 2000,
        },
        "Inf": { // TODO: Use quad precision! Zoom to center! This is a fascinating series!
            "zoom": Math.pow(2, -32),
            "x": -0.743643887037158704752191506114774,
            "y": 0.131825904205311970493132056385139,
            "iterations": 5000,
            "contrast": 0.23,
        },

        "Corals": {
            "zoom": Math.pow(2, -22),
            "x": -0.69586585925114464,
            "y": 0.44646844758065846,
            "iterations": 800,
        },
        "Black": {
            "zoom": Math.pow(2, -22.6),
            "x": -0.6958659321672108,
            "y": 0.4464684050481072,
            "iterations": 6000,
        },

        "Zebra": {
            "zoom": 0.00000000041556109,
            "x": -1.47926642477697179,
            "y": 0.01049892980285544,
            "iterations": 3000,
        },
        "Sputnik": {
            "zoom": 0.00000003537777001,
            "x": -1.94158036006772883,
            "y": 0.00084079117678854,
            "iterations": 2000,
        },
        "Sputnik-Star": {
            "zoom": Math.pow(2, -22.8),
            "x": -1.94158036006772883,
            "y": 0.00084079117678854,
            "iterations": 2000,
            "contrast": 36,
            "smoothing": 0,
        },
        "Sputnik-Flower": {
            "zoom": Math.pow(2, -22.8),
            "x": -1.94158036006772883,
            "y": 0.00084079117678854,
            "iterations": 2000,
            "contrast": 4.2,
            "smoothing": -10.2,
        },

        "Sea Shells": {
            "zoom": 0.00000030811596324,
            "x": 0.25248304937365174,
            "y": -0.00018443951415280,
            "smoothing": 3,
            "contrast": 3.96,
            "iterations": 980,
            "shift": 0.7,
        },
        "Kraken": {
            "zoom": 0.00000000262600591,
            "x": -0.52337883341641245,
            "y": 0.68058363785114295,
            "iterations": 10000,
        },

        "Zebra-Snail": {
            "zoom": 0.00000001253239359,
            "x": -0.53655782061638835,
            "y": 0.66622670863609945,
            "iterations": 12000,
        },

        "Domination": {
            "zoom": Math.pow(2., -27.295),
            "x": 0.2524801648402278,
            "y": -0.0001847804869756,
            "iterations": 7000,
            // TODO: Finde eine Instanz, wo die untere Spirale einen
            // geringeren Radius hat!
        },

        "Contraposition": {
            "zoom": 0.0003249702384,
            "x": 0.26000060263903690,
            "y": -0.00160370450573859
        },

        "Drops": {
            "zoom": Math.pow(2, -14.646),
            "x": 0.2514953857859145,
            "y": -0.0000936494732090,
            "iterations": 245,
        },

        "Balls": {
            "zoom": 0.00225727670266614,
            "x": -0.72229992560474776,
            "y": 0.19226090615766106,
            "radius": 0.95,
            "iterations": 1000,
            "smoothing": 2.0,
        },
    }
}