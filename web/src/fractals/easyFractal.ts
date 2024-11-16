import {expect} from "chai";
import {FractalProgram} from "../fractalProgram";
import {Rect} from "../global";
import {Window} from "../gui";
import {Fractal} from "./abstractFractal";

export class Parameter {
    constructor(
        public prettyName: string, // Name to show to the user
    ) {}
}

export class ParameterNumeric extends Parameter {
    constructor(
        prettyName: string,        // Name to show to the user
        public min: number,        // minimum value
        public max: number,        // maximum value
        public defaults: number[], // different default values - the first one
                                   // is used when not overridden
        public base: number = 1,   // when scaling exponentially, use this base!
        public integer: boolean = false, // allow only integer values
    ) {
        super(prettyName);
    }
}

export class ParameterOptions extends Parameter {
    constructor(
        prettyName: string, // Name to show to the user
        public options: {
            [option: string]: number
        }, // Options to choose from and their numeric values
        public fallback: string, // the default option
    ) {
        super(prettyName);
    }
}

export class UniformParameters {
    constructor(
        public name: string,         // name of this uniform
        public params: string[],     // names of parameters to load
        public bake: boolean = false // try to bake the paramter to the shader
    ) {}
}

export class Uniform2f extends UniformParameters {
    constructor(name: string, first: string, second: string,
                bake: boolean = false) {
        super(name, [ first, second ], bake);
    }
}

export class Uniform2d extends UniformParameters {
    constructor(name: string, first: string, second: string,
                bake: boolean = false) {
        super(name, [ first, second ], bake);
    }
}

// becomes a double when the shader is in double mode
export class Uniform2fd extends UniformParameters {
    constructor(name: string, first: string, second: string,
                bake: boolean = false) {
        super(name, [ first, second ], bake);
    }
}

export class Uniform4f extends UniformParameters {
    constructor(name: string, first: string, second: string, third: string,
                fourth: string, bake: boolean = false) {
        super(name, [ first, second, third, fourth ], bake);
    }
}

export abstract class EasyFractal extends Fractal {
    // the program currently in use
    protected prog: FractalProgram;

    // the window
    private win: Window;

    private doublePrecision: boolean = false;

    // the rendering context
    protected gl: WebGLRenderingContext;

    abstract easyCreate(): Promise<void>;

    constructor(private fragmentSrc: string,
                private supportDouble: boolean = true,
                private alwaysWebGL2: boolean = false) {
        super();
    }

    public create(gl: WebGLRenderingContext): Promise<void> {
        this.gl = gl;
        this.initWindow();

        this.prog = new FractalProgram(
            this.fragmentSrc, this.gl, this.getUniforms().map(x => x.name),
            this.getUniforms().filter(x => x.bake).map(x => x.name),
            this.alwaysWebGL2);
        this.prog.setDouble(this.doublePrecision && this.supportDouble);

        return this.easyCreate();
    }

    // Some common default implementations for these functions
    public setPos(x: number, y: number) { this.prog.set2fd("pos", x, y); };
    public setZoom(x: number, y: number) { this.prog.set2fd("zoom", x, y); };
    public use() { this.prog.use(); }
    public setRect(r: Rect) {
        this.prog.setRect("aPos", r.l * 2 - 1, r.t * 2 - 1, r.r * 2 - 1,
                          r.b * 2 - 1);
        this.prog.setRect("aTexCoords", r.l, r.t, r.r, r.b);
    }
    public draw() { this.prog.drawRect("aPos"); }

    // some changes require immediate re-rendering the Fractal
    protected invalidate() {
        this.dirty = true;
        expect(this.requestFrame).to.be.a('function');
        this.requestFrame();
    }

    public destroy() {
        expect(this.win).to.be.an.instanceOf(Window);
        this.win.destroy();
        this.win = null;
    }

    private paramValues: {[name: string]: any} = {};

    private initWindow() {
        this.win = new Window(0, 0, 240, 600, this.getName(), true, true);
        const params = this.getParameters();
        for (const name of Object.keys(params)) {
            const param = params[name];
            if (param instanceof ParameterNumeric) {
                expect(param.integer).is.a("boolean")
                expect(param.defaults).to.be.a("array");
                expect(param.defaults.length).to.be.greaterThan(0);

                this.win.addSlider(param.prettyName, param.defaults[0],
                                   param.min, param.max, param.base, (v) => {
                                       expect(v).to.be.a("number");
                                       if (v !== this.paramValues[name]) {
                                           this.paramValues[name] = v;
                                           this.invalidate();
                                       }
                                   }, param.defaults, param.integer);

            } else {
                console.error("unsupported Parameter", name, param);
            }
        }

        if (this.supportDouble) {
            this.win.addButton("Single Precision", () => {
                if (this.doublePrecision) {
                    this.doublePrecision = false;
                    this.win.renameButton("Double Precision",
                                          "Single Precision");
                } else {
                    this.doublePrecision = true;
                    this.win.renameButton("Single Precision",
                                          "Double Precision");
                }
                this.prog.setDouble(this.doublePrecision);
                this.invalidate();
            })
        }
    }

    private getPreset() {
        return (this.constructor as typeof EasyFractal)
            .presets[this.currentPreset];
    }

    // The first preset is the "Base" preset. Everything alls extends it.
    private getFirstPreset() {
        return (this.constructor as typeof EasyFractal)
            .presets[0];
    }

    private currentPreset: string;
    public loadPreset(name: string) {
        console
            .log("loading preset")

                this.currentPreset = name;
        this.prog.use();

        const preset = this.getPreset();
        const params = this.getParameters();
        for (const pName of Object.keys(params)) {
            const param = params[pName];
            if (param instanceof ParameterNumeric) {
                if (pName in preset)
                    this.paramValues[pName] = preset[pName];
                else
                    this.paramValues[pName] = param.defaults[0];
                this.win.setSlider(param.prettyName, this.paramValues[pName]);
            } else {
                console.error("unsupported Parameter", name, param);
            }
        }

        this.invalidate();
    }

    update() {
        const uniforms = this.getUniforms();
        for (const u of uniforms) {
            expect(u.bake).to.eq(false, "baking not supported"); // TODO
            expect(u.params).to.be.an("array");
            expect(this.paramValues).to.contain.keys(u.params);

            const vs = u.params.map(x => this.paramValues[x]);
            vs.forEach(x => expect(x).to.be.a("number").and.not.to.be.NaN);

            if (u instanceof Uniform2f) {
                expect(u.params.length).to.eq(2);
                this.prog.set2f(u.name, vs[0], vs[1]);
            } else if (u instanceof Uniform4f) {
                expect(u.params.length).to.eq(4);
                this.prog.set4f(u.name, vs[0], vs[1], vs[2], vs[3]);
            } else if (u instanceof Uniform2d) {
                expect(u.params.length).to.eq(2);
                this.prog.set2d(u.name, vs[0], vs[1]);
            } else if (u instanceof Uniform2fd) {
                expect(u.params.length).to.eq(2);
                console.error("TODO: Uniform2fd")
                // this.prog.set2f(u.name, vs[0], vs[1]);
            }
        }
    };

    defaults() {
        return Object.assign(Object.assign({},this.getFirstPreset()), this.getPreset());
    };

    // returns the parameters of the fractal
    // TODO: This might become parametric!
    abstract getParameters(): {[name: string]: Parameter};
    abstract getUniforms(): UniformParameters[];
}