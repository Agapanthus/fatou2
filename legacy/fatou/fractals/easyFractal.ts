import { expect } from "chai";
import { assert } from "console";
import { Program } from "pixi.js";
import { Complex } from "../complex";
import { FractalProgram } from "../fractalProgram";
import { Exception, Rect, Size } from "../global";
import { Window } from "../gui";
import { SimpleProgram } from "../simpleProgram";
import { Texture } from "../texture";
import { Fractal } from "./abstractFractal";

// @ts-ignore
import texture_shader from "../shaders/util/texture.fs";
// @ts-ignore
import vertex_default from "../shaders/vertex/default.vs";

export class Parameter {
    constructor(
        public prettyName: string // Name to show to the user
    ) {}
}

export class ParameterNumeric extends Parameter {
    constructor(
        prettyName: string, // Name to show to the user
        public min: number, // minimum value
        public max: number, // maximum value
        public defaults: number[], // different default values - the first one
        // is used when not overridden
        public base: number = 1, // when scaling exponentially, use this base!
        public integer: boolean = false // allow only integer values
    ) {
        super(prettyName);
    }
}

export class ParameterOptions extends Parameter {
    constructor(
        prettyName: string, // Name to show to the user
        public options: {
            [option: string]: number;
        }, // Options to choose from and their numeric values
        public fallback: string // the default option
    ) {
        super(prettyName);
    }
}

export class UniformParameters {
    constructor(
        public name: string, // name of this uniform
        public params: string[], // names of parameters to load
        public bake: boolean = false // try to bake the paramter to the shader
    ) {}
}

export class Uniform2f extends UniformParameters {
    constructor(name: string, first: string, second: string, bake: boolean = false) {
        super(name, [first, second], bake);
    }
}

export class Uniform2d extends UniformParameters {
    constructor(name: string, first: string, second: string, bake: boolean = false) {
        super(name, [first, second], bake);
    }
}

// becomes a double when the shader is in double mode
export class Uniform2fd extends UniformParameters {
    constructor(name: string, first: string, second: string, bake: boolean = false) {
        super(name, [first, second], bake);
    }
}

export class Uniform4f extends UniformParameters {
    constructor(
        name: string,
        first: string,
        second: string,
        third: string,
        fourth: string,
        bake: boolean = false
    ) {
        super(name, [first, second, third, fourth], bake);
    }
}

export abstract class EasyFractal extends Fractal {
    // the program currently in use
    protected prog: FractalProgram;

    // the window
    private win: Window;

    protected useSoftwareRenderer: boolean = false;

    private doublePrecision: boolean = false;

    // the rendering context
    protected gl: WebGLRenderingContext;

    abstract easyCreate(): Promise<void>;

    private textureProg: SimpleProgram;

    constructor(
        private fragmentSrc: string,
        private supportDouble: boolean = true,
        private alwaysWebGL2: boolean = false
    ) {
        super();
    }

    public create(gl: WebGLRenderingContext): Promise<void> {
        this.gl = gl;
        this.initWindow();

        /////////////

        this.prog = new FractalProgram(
            this.fragmentSrc,
            this.gl,
            this.getUniforms().map((x) => x.name),
            this.getUniforms()
                .filter((x) => x.bake)
                .map((x) => x.name),
            this.alwaysWebGL2
        );
        this.prog.setDouble(this.doublePrecision && this.supportDouble);

        //////////////

        this.textureProg = new SimpleProgram(
            vertex_default,
            texture_shader,
            this.gl,
            ["screenTexture"],
            ["aPos", "aTexCoords"]
        );
        this.textureProg.seti("screenTexture", 0);
        this.textureProg.setRect("aPos", 0.0, 0.0, 1.0, 1.0);
        this.textureProg.setRect("aTexCoords", 0.0, 0.0, 1.0, 1.0);

        ///////////

        return this.easyCreate();
    }

    // Some common default implementations for these functions
    public setPos(x: number, y: number) {
        this.prog.set2fd("pos", x, y);
    }
    public setZoom(x: number, y: number) {
        this.prog.set2fd("zoom", x, y);
    }
    public use() {
        this.prog.use();
    }
    public setRect(r: Rect) {
        this.prog.setRect("aPos", r.l * 2 - 1, r.t * 2 - 1, r.r * 2 - 1, r.b * 2 - 1);
        this.prog.setRect("aTexCoords", r.l, r.t, r.r, r.b);
    }
    public draw() {
        this.prog.drawRect("aPos");
    }

    // some changes require immediate re-rendering the Fractal
    protected invalidate() {
        this.dirty = true;
        expect(this.requestFrame).to.be.a("function");
        if (this.requestFrame) this.requestFrame();
    }

    public destroy() {
        expect(this.win).to.be.an.instanceOf(Window);
        this.win.destroy();
        //this.win = null;

        this.textureProg.destroy();
    }

    private paramValues: { [name: string]: any } = {};

    private initWindow() {
        this.win = new Window(0, 0, 240, 600, this.getName(), true, true);
        const params = this.getParameters();
        for (const name of Object.keys(params)) {
            const param = params[name];
            if (param instanceof ParameterNumeric) {
                expect(param.integer).is.a("boolean");
                expect(param.defaults).to.be.a("array");
                expect(param.defaults.length).to.be.greaterThan(0);

                this.win.addSlider(
                    param.prettyName,
                    param.defaults[0],
                    param.min,
                    param.max,
                    param.base,
                    (v) => {
                        expect(v).to.be.a("number");
                        if (v !== this.paramValues[name]) {
                            this.paramValues[name] = v;
                            this.invalidate();
                        }
                    },
                    param.defaults,
                    param.integer
                );
            } else {
                console.error("unsupported Parameter", name, param);
            }
        }

        if (this.supportDouble) {
            this.win.addButton("Single Precision", () => {
                if (this.doublePrecision) {
                    this.doublePrecision = false;
                    this.win.renameButton("Double Precision", "Single Precision");
                } else {
                    this.doublePrecision = true;
                    this.win.renameButton("Single Precision", "Double Precision");
                }
                this.prog.setDouble(this.doublePrecision);
                this.invalidate();
            });
        }

        this.win.addButton("CPU", () => {
            if (this.useSoftwareRenderer) {
                this.useSoftwareRenderer = false;
                this.win.renameButton("GPU", "CPU");
            } else {
                this.useSoftwareRenderer = true;
                this.win.renameButton("CPU", "GPU");
            }
            this.invalidate();
        });
    }

    private getPreset() {
        return (this.constructor as typeof EasyFractal).presets[this.currentPreset];
    }

    // The first preset is the "Base" preset. Everything alls extends it.
    private getFirstPreset() {
        return (this.constructor as typeof EasyFractal).presets[0];
    }

    private currentPreset: string;
    public loadPreset(name: string) {
        console.log("loading preset");

        this.currentPreset = name;
        this.prog.use();

        const preset = this.getPreset();
        const params = this.getParameters();
        for (const pName of Object.keys(params)) {
            const param = params[pName];
            if (param instanceof ParameterNumeric) {
                if (pName in preset) this.paramValues[pName] = preset[pName];
                else this.paramValues[pName] = param.defaults[0];
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

            const vs = u.params.map((x) => this.paramValues[x]);
            vs.forEach((x) => expect(x).to.be.a("number").and.not.to.be.NaN);

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
                console.error("TODO: Uniform2fd");
                // this.prog.set2f(u.name, vs[0], vs[1]);
            }
        }
    }

    defaults() {
        return Object.assign(Object.assign({}, this.getFirstPreset()), this.getPreset());
    }

    public getParamValue(name: string): any {
        expect(this.paramValues).to.contain.keys(name);
        return this.paramValues[name];
    }

    // returns the parameters of the fractal
    // TODO: This might become parametric!
    abstract getParameters(): { [name: string]: Parameter };
    abstract getUniforms(): UniformParameters[];

    private done = false;
    public fullRender(r: Rect, pos: Complex, zoom: Complex, s: Size): void {
        if (this.useSoftwareRenderer) {
            //if (this.done) return;

            this.done = true;
            const rgba = new Uint8Array(s.w * s.h * 4);
            const sw = s.w;
            const sh = s.h;

            // Positioning isn't perfect (edges are rough)

            for (let x = 0; x < sw; x += 1) {
                for (let y = r.t; y <= r.b; y += 1 / sh) {
                    const c = this.sample(pos, zoom, new Complex(x / sw, y ));
                    const i = (Math.round(y * sh) * sw + x) * 4;
                    rgba[i + 0] = c.r;
                    rgba[i + 1] = c.g;
                    rgba[i + 2] = c.b;
                    rgba[i + 3] = c.a;
                }
            }

            // Creating and Destroying so often might be quite inefficient
            const tex = new Texture(rgba, () => {}, this.gl, sw, sh);
            tex.bind();

            // TODO: Avoid allocations of arrays and texture!
            // TODO: Arbitrary precision!
            // TODO: Multithreaded CPU renderer!
            // https://github.com/cslarsen/mandelbrot-js
            // http://linas.org/art-gallery/escape/escape.html

            this.textureProg.use();
            this.textureProg.setRect("aPos", r.l * 2 - 1, r.t * 2 - 1, r.r * 2 - 1, r.b * 2 - 1);
            this.textureProg.setRect2("aTexCoords", r);
            this.textureProg.drawRect("aPos");

            tex.destroy();
        } else {
            this.use();
            this.setRect(r);

            this.setPos(pos.x(), pos.y());
            this.setZoom(zoom.x(), zoom.y());

            this.update();

            this.draw();
        }
    }
}
