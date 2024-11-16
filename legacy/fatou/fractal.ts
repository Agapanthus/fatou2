import { Complex } from "./complex";
import { Fractal } from "./fractals/abstractFractal";
import { Antifractal } from "./fractals/antifractal";
import { Mandelbrot } from "./fractals/mandelbrot";
import { TestFractal } from "./fractals/test";
// import {Newton} from "./fractals/newton";
import { finishLoading, loading, Rect, Size } from "./global";
import { Window } from "./gui";
import { Navigator } from "./navigator";
import { Program } from "./program";
import { Progressive } from "./progressive";
import { PlaceholderRenderer, RenderLoop } from "./renderer";
// @ts-ignore
import texture_shader from "./shaders/util/texture.fs";
// @ts-ignore
import white_shader from "./shaders/util/white.fs";
// @ts-ignore
import vertex_default from "./shaders/vertex/default.vs";
// @ts-ignore
import pos_shader from "./shaders/vertex/pos.vs";
import { SimpleProgram } from "./simpleProgram";

export class FractalRunner {
    private renderer: PlaceholderRenderer;
    private renderLoop: RenderLoop | null = null;
    private dataWin: Window;
    private presetWin: Window;
    private frac: Fractal | null = null;
    private fracClass: typeof Fractal;
    private progressive: Progressive;
    private navi: Navigator;
    private textureProg: Program;
    private vProg: Program;

    constructor() {
        this.renderer = new PlaceholderRenderer("#mainCanvas");
        this.presetWin = new Window(0, 0, 420, 200, "Presets", true, true);
        this.dataWin = new Window(0, 0, 240, 240, "Renderer", true, true);

        const gl = this.renderer.getGL();
        this.textureProg = new SimpleProgram(
            vertex_default,
            texture_shader,
            gl,
            ["screenTexture"],
            ["aPos", "aTexCoords"]
        );
        this.textureProg.seti("screenTexture", 0);
        this.textureProg.setRect("aPos", 0.0, 0.0, 1.0, 1.0);
        this.textureProg.setRect("aTexCoords", 0.0, 0.0, 1.0, 1.0);

        this.vProg = new SimpleProgram(pos_shader, white_shader, gl, [] as any, ["aPos"]);
        this.vProg.setRect("aPos", 0.0, 0.0, 1.0, 1.0);

        const addPresets = (name: string, fracClass: typeof Fractal, cons: () => Fractal) => {
            for (const preset of Object.keys(fracClass.presets)) {
                this.presetWin.addButton(name + ": " + preset, () => {
                    const compatibleSS =
                        !("superSamples" in fracClass.presets[preset]) ||
                        this.progressive.getSupersamples() ===
                            fracClass.presets[preset].superSamples;
                    if (fracClass === this.fracClass && compatibleSS) {
                        console.log("Changing Preset to:", preset);
                        this.loadPreset(preset);
                    } else {
                        this.fracClass = fracClass;
                        this.setFractal(cons()).then(() => {
                            this.loadPreset(preset);
                        });
                    }
                });
            }
        };

        addPresets("Test", TestFractal, () => {
            return new TestFractal();
        });

        addPresets("Antifractal", Antifractal, () => {
            return new Antifractal();
        });
        // addPresets("Newton", Newton, () => { return new Newton(); });

        addPresets("Mandelbrot", Mandelbrot, () => {
            return new Mandelbrot();
        });

        this.fracClass = Mandelbrot;
        const m = new Mandelbrot();
        this.setFractal(m).then(() => {
            this.loadPreset("filigran");
        });
    }

    public setFractal(frac: Fractal): Promise<void> {
        if (this.frac) this.frac.destroy();
        this.frac = frac;

        return new Promise((res, rej) => {
            loading();
            console.log("Loading fractal", this.frac.getName());

            (this.renderLoop ? this.renderLoop.terminate() : Promise.resolve())
                .then(() => {
                    // No renderLoop yet
                    this.renderLoop = null;

                    // is we invalidate either way, we can load requestFrame
                    // with a dummy
                    this.frac.setRequestFrame(() => {});

                    return frac.create(this.renderer.getGL());
                })
                .then(() => {
                    this.showFractal();
                    res();
                });
        });
    }

    private loadPreset(preset: string) {
        this.frac.loadPreset(preset);

        const p = this.fracClass.presets[preset];
        this.navi.setPos(p.x || 0, p.y || 0, p.zoom || 1.0);

        this.invalidate();
    }

    private invalidate() {
        if (this.renderLoop) this.renderLoop.push();
        this.progressive.change();
    }

    private showFractal() {
        const gl = this.renderer.getGL();

        if (this.navi) this.navi.destroy();
        this.navi = new Navigator(this.renderer.getCanvas(), this.frac.defaults());

        // const p = new Projection(new Size(256, 256), gl);
        if (this.progressive) this.progressive.destroy();
        this.progressive = new Progressive(
            gl,
            this.textureProg,
            this.vProg,
            this.renderer,
            this.frac.defaults().superSamples || 4,
            this.dataWin
        );
        this.renderer.onResize = () => {
            this.progressive.refreshSize();
        };
        this.progressive.setProgram(this.navi, (r: Rect, s: Size) => {
            this.dataWin.setText("zoom", Math.log2(this.navi.getZoom()).toFixed(3) + " (log)");
            this.dataWin.setText("posX", this.navi.getPosX().toFixed(16));
            this.dataWin.setText("posY", this.navi.getPosY().toFixed(16));
            this.dataWin.setText(
                "Resolution",
                gl.drawingBufferWidth +
                    ", " +
                    gl.drawingBufferHeight +
                    " (dpr" +
                    window.devicePixelRatio +
                    ")"
            );

            const aspect = this.renderer.getSize().h / this.renderer.getSize().w;
            const zoom = Complex.fromNum(this.navi.getZoom(), this.navi.getZoom() * aspect);
            const pos = Complex.fromNum(this.navi.getPosX(), this.navi.getPosY());

            this.frac.fullRender(r, pos, zoom, s);
        });

        this.renderLoop = new RenderLoop(
            (delta: number, continuos: boolean) => {
                this.navi.interpolate(delta);

                if (this.frac.dirty) {
                    this.invalidate();
                    this.frac.dirty = false;
                }

                if (this.renderLoop.fps() === null)
                    this.dataWin.setText("FPS", "sleeping", false, false);
                else this.dataWin.setText("FPS", this.renderLoop.fps().toFixed(1), true, false);

                return this.progressive.render(delta, continuos);
            },
            () => {
                this.progressive.startRead();
                this.renderer.render();
                this.textureProg.use();
                this.textureProg.setRect("aPos", -1.0, -1.0, 1.0, 1.0);
                this.textureProg.setRect("aTexCoords", 0.0, 0.0, 1.0, 1.0);
                this.textureProg.drawRect("aPos");
                gl.flush();
                this.progressive.endRead();
            },
            () => {
                this.dataWin.setText("FPS", "sleeping", false, false);
            }
        );

        this.progressive.setPush(this.renderLoop.push.bind(this.renderLoop));
        this.navi.setOnChangeFun(this.progressive.change.bind(this.progressive));
        this.frac.setRequestFrame(this.renderLoop.push.bind(this.renderLoop));

        gl.depthMask(false);
        gl.activeTexture(gl.TEXTURE0);

        finishLoading();
        console.log("loaded fractal", this.frac.getName());
        this.renderLoop.push();
    }

    public destroy() {
        this.renderLoop.terminate().then(() => {
            this.frac.destroy();
            this.progressive.destroy();
            this.textureProg.destroy();
            this.renderer.destroy();
            this.dataWin.destroy();
            this.presetWin.destroy();
            this.navi.destroy();
        });
    }
}
