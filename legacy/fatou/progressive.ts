import { BigFloat } from "bigfloat.js";
import { Complex } from "./complex";
import { Exception, rationalize, Rect, Size } from "./global";
import { Window } from "./gui";
import { Navigator } from "./navigator";
import { Program } from "./program";
import { InterlacedProjection, Projection, Renderer } from "./renderer";

const minLive = 1 / 1024; // 1 / 8

class Optimizer {
    constructor() {
        this.sps = 500000; // 10000; // 100000; // 30000; // 1000000

        // (in firefox) Less than 30! Faster is not allowed! (otherwise, in
        // Firefox, the control system will run to infinite when the framerate
        // is capped) - best framerates ar slightly faster than a multiple 1/60
        this.targetFPS = 25;
    }
    private static c = 0;

    // estimated iterations per second for GPU
    private gpuPower = 100000;

    // estimated iterations per sample for scene
    private estimIterPerSample = 100;

    public postFrame(millis: number, isContinuos: boolean) {
        if (!isContinuos) {
            const t = this.sps;
            // aim low
            //this.sps = this.lastFrameSps / 10;
            this.lastFrameSps = t;
            return;
        }

        // TODO: This is bad. We can't really control the framerate using a controller! We have to build a model of the difficulty of the current scene (CPU sampling) and a model of the GPU-power and then combine this to get samples per second!

        /*
        const effortPerPixel = millis / this.usedSamples;
        const targetMillis = 1000 / this.targetFPS;
        if (millis < targetMillis - 2) {
            this.sps = this.sps * 1.1 + 1;
        } else if (millis > targetMillis * 4) {
            this.sps /= 10;
        } else if (millis > targetMillis * 2) {
            this.sps /= 2;
        } else {
            this.sps /= 1.1;
        }

        const smoothing = 0.1;
        this.gpuPower =
            this.gpuPower * (1 - smoothing) +
            ((this.usedSamples * this.estimIterPerSample) / millis) * 1000 * smoothing;

        //    console.log(Math.log(this.gpuPower));

        this.sps = Math.max(Math.min(this.sps, 10e6), 128);
        */

        //console.log(Math.log2(this.sps))
    }

    private old = new Date().getTime();

    public updateSampler(navi: Navigator, aspect: number) {
        // currently assuming mandelbrot
        // TODO: use a software renderer of the fractal instead!
        // TODO: also, use the real iteration boundary!
        // this.estimIterPerSample =
    }

    public getSPS(): number {
        // TODO: Constant overhead etc. modellieren!
        // TODO: Timing allein ist schwierig. Stattdessen für jedes Fraktal
        // einen Probin-Algo schreiben, der auf der CPU parallel Stichproben
        // berechnet. Die dort ermittelte Iterationszahl wird als Schätzung für
        // die Samples per Frame verwendet!

        return this.sps;
    }

    public postRender(
        usedSamples: number,
        bufferCount: number,
        changes: number,
        reset: boolean
    ): void {
        this.usedSamples = usedSamples;
        this.bufferCount = bufferCount;
        this.changes = changes;
    }

    // Cached values
    private usedSamples: number;
    private lastSamples: number;
    private changes: number;
    private bufferCount: number;
    private lastFrameSps: number = 0;

    private sps: number;
    private targetFPS: number;
}

class Downchain {
    constructor(gl: WebGLRenderingContext, tProg: Program) {
        this.gl = gl;
        this.tProg = tProg;
    }
    public setSize(source: Size, target: Size): void {
        this.destroy();
        const tsize = source.clone();
        while (tsize.w > target.w * 2) {
            tsize.w /= 2;
            tsize.h /= 2;
            this.gpb.push(new Projection(tsize, this.gl, false, true));
        }
    }
    // Will - if necessary - read the current texture and smoothly downscale
    // it until it is at most 2 times the target's size
    // Assumes, that the source's dimensions are both the same real multiple
    // of the target's dimensions
    // TODO: Only walk the whole chain when something changed
    public rebind(): void {
        for (const b of this.gpb) {
            b.render();
            this.tProg.use();
            this.tProg.setRect("aPos", -1.0, -1.0, 1.0, 1.0);
            this.tProg.setRect("aTexCoords", 0.0, 0.0, 1.0, 1.0);
            this.tProg.drawRect("aPos");
            this.gl.flush();
            b.readFrom();
        }
    }

    public destroy() {
        for (const g of this.gpb) {
            g.destroy();
        }
        this.gpb = [];
    }

    private gpb: Projection[] = []; // General purpose buffers
    private gl: WebGLRenderingContext;
    private tProg: Program;
}

export class Progressive {
    constructor(
        gl: WebGLRenderingContext,
        tProg: Program,
        vProg: Program,
        target: Renderer,
        supersamples: number,
        win: Window
    ) {
        this.gl = gl;
        this.tProg = tProg;
        this.vProg = vProg;
        this.down = new Downchain(this.gl, this.tProg);
        this.firstFrame = true;
        this.target = target;
        this.optim = new Optimizer();
        this.setSupersamples(supersamples);
        this.win = win;
    }

    public destroy() {
        this.down.destroy();
        for (const u of this.upchain) {
            u.destroy();
        }
    }

    public setSupersamples(supersamples: number): void {
        this.supersamples = supersamples;
        this.setSize(this.target.getSize());
    }

    public refreshSize(): void {
        if (this.target.getSize() !== this.size) {
            this.setSize(this.target.getSize());
        }
        // TODO: Aufwands-differenz berechnen!
    }

    public setProgram(navi: Navigator, renderToRect: (r: Rect, s: Size) => void): void {
        this.renderToRect = renderToRect;
        this.change(navi);
        // TODO: Aufwands-differenz berechnen!
    }

    public change(navi: Navigator): void {
        this.currentLevel = this.upchain.length;
        this.currentProgress = 0;
        this.framesSinceReset = 0;

        if (navi) {
            const ratio = this.target.getSize().h / this.target.getSize().w;
            /* const x = navi.getPosX();
            const y = navi.getPosY();
            const zx = navi.getZoom();
           
            const zy = zx * ratio;*/
            this.optim.updateSampler(navi, ratio); //new Rect(x - 1 * zx, y - 1 * zy, x + 1 * zx, y + 1 * zy));
        }

        this.push();
        // TODO: Aufwands-differenz abschätzen und an Optim senden!
    }

    public render(millis: number, continuos: boolean): boolean {
        /*if(this.currentLevel === 0) {
            this.currentLevel = this.upchain.length;
            this.currentProgress = 0;
        }*/

        let changed = false;

        if (!this.firstFrame) {
            // TODO: Frames nach Pausen müssen gekennzeichnet werden!
            this.optim.postFrame(millis, continuos);
        }
        this.firstFrame = false;

        this.framesSinceReset++;

        let left = this.optim.getSPS();
        let consumed = 0;

        if (this.currentLevel === this.upchain.length) {
            // Choose the largest buffer you can finish
            let i = this.upchain.length - 1;
            for (; i > 0; i--) {
                if (i < 1) break;
                if (this.upchain[i - 1].getSize().area() > left) {
                    break;
                }
            }

            // this.gl.disable(this.gl.STENCIL_TEST);
            this.upchain[i].render();

            this.renderToRect(new Rect(0, 0, 1, 1), this.upchain[i].getSize());

            this.gl.flush();

            left -= this.upchain[i].getSize().area();
            consumed = this.upchain[i].getSize().area();

            this.currentLevel = i + 1;
            this.currentProgress = 1.0;

            // console.log("rendered first " + this.currentLevel);

            this.copyBack();

            changed = true;
        }

        // Continue work
        // TODO: don't try to continue when animating! Safe the time for other
        // stuff!
        for (let dummy = 0; dummy < 20; dummy++) {
            if (dummy > 10) {
                console.log("SUS: 274 progressive");
            }
            if (this.currentLevel === 0) {
                break;
            }
            // If there are less then 5% left, don't try it. The overhead is not
            // worth it.
            // TODO: arbitrary number. This isn't always true, is it?
            if (left < consumed * 0.05) {
                break;
            }

            // Don't care about vertical and horizontal buffers - the buffer
            // will be filled anyway.

            const size = this.upchain[this.currentLevel - 1].getSize();

            // Samples = (bottom - curProg) * size.w * size.h / 2
            let bottom = (~~(left / size.w) / size.h) * 2 + this.currentProgress;
            if (bottom > 1.0) {
                bottom = 1.0;
            }
            if (bottom === this.currentProgress) {
                // You must do something!
                if (consumed === 0) {
                    bottom = (1 / size.h) * 2 + this.currentProgress;
                } else {
                    break;
                }
            }

            this.upchain[this.currentLevel - 1].startWrite(true);

            // TODO: What are the exact coordinates? Following problem:
            // rendering level 1 hori, starts with pos 0
            // rendering level 3 hori, starts with pos 0 again! Why are these different rows now? shouldn't they be shifted a tiny little bit?
            this.renderToRect(
                new Rect(0, this.currentProgress, 1, bottom),
                new Size(
                    size.w,
                    size.h //* Math.abs(bottom - this.currentProgress)
                )
            );

            const consume = ((bottom - this.currentProgress) * size.h * size.w) / 2;
            left -= consume;
            consumed += consume;
            this.currentProgress = bottom;

            // console.log("Level " + this.currentLevel + " " +
            // this.currentProgress);

            this.upchain[this.currentLevel - 1].endWrite();

            this.gl.flush();

            if (this.currentProgress === 1.0) {
                this.copyBack();
                changed = true;
            }

            if (this.liveChanges()) {
                changed = true;
                this.push();
            }
        }

        this.win.setText("prog", this.progress().toFixed(2) + "", true, false);
        if (this.progress() === 0) this.win.setText("spp", this.spp() + " (max)", true, false);
        else if (this.liveChanges())
            this.win.setText(
                "spp",
                rationalize(this.spp()) + " -> " + rationalize(this.spp() * 2) + " (live)",
                true,
                false
            );
        else
            this.win.setText(
                "spp",
                rationalize(this.spp()) + " -> " + rationalize(this.spp() * 2) + " (offscreen)",
                true,
                false
            );

        this.win.setText("spf", consumed, true, true, 0);

        // If progressive Rendering hasn't finished
        if (this.currentLevel !== 0) {
            this.push();
        }

        this.optim.postRender(consumed, this.currentLevel, 0, false); // TODO: Die letzten beiden Parameter sind falsch...

        return changed;
    }

    // the overall progress - counting towards zero
    public progress() {
        return this.currentLevel - this.currentProgress;
    }

    // current completed samples per pixels
    public spp() {
        return (this.supersamples * this.supersamples) / Math.pow(2, this.currentLevel);
    }

    // show progress live
    private liveChanges(): boolean {
        // Showing the liveChanges reduces smoothing of the preview, especially
        // for very low spp For spp > 4 a Downchain is needed and liveChanges
        // are too computationally expensive. Therefore, show live changes only
        // for medium spp
        return this.spp() > minLive && this.spp() < 4;
    }

    // Copies the current layer to the back of the next layer
    private copyBack(): void {
        this.currentLevel--;
        this.currentProgress = 0;

        // console.log(this.currentLevel);

        if (this.currentLevel > 0) {
            this.upchain[this.currentLevel - 1].startWrite(false);

            this.upchain[this.currentLevel].readFrom();

            this.tProg.use();

            this.tProg.setRect("aPos", -1.0, -1.0, 1.0, 1.0);
            this.tProg.setRect("aTexCoords", 0.0, 0.0, 1.0, 1.0);
            this.tProg.drawRect("aPos");

            this.upchain[this.currentLevel - 1].endWrite();

            if (this.liveChanges()) {
                // #########################################
                // Additionally, write a mixture to the front:
                // (writing without interpolation looks jumpy)

                this.upchain[this.currentLevel - 1].startWrite(true);

                this.upchain[this.currentLevel].setLinear(true);
                this.upchain[this.currentLevel].readFrom();

                this.tProg.use();

                this.tProg.setRect("aPos", -1.0, -1.0, 1.0, 1.0);
                let dx = 1 / this.upchain[this.currentLevel].getSize().w / 4;
                let dy = 1 / this.upchain[this.currentLevel].getSize().h / 4;
                if (this.currentLevel % 2 === 0) dx = 0;
                if (this.currentLevel % 2 === 1) dy = 0;
                this.tProg.setRect("aTexCoords", 0.0 + dx, 0.0 + dy, 1.0 + dx, 1.0 + dy);
                this.tProg.drawRect("aPos");

                this.upchain[this.currentLevel - 1].endWrite();
                this.upchain[this.currentLevel].setLinear(false);
            }

            // --- flush
            this.gl.flush();
        }
    }

    public startRead() {
        if (this.currentLevel >= this.upchain.length)
            throw new Exception("nothing rendered so far", "progressive.bind");

        if (this.liveChanges() && this.framesSinceReset > 1 && this.currentLevel > 0) {
            this.upchain[this.currentLevel - 1].setLinear(true);
            this.upchain[this.currentLevel - 1].readFrom();
        } else {
            this.upchain[this.currentLevel].setLinear(true);
            this.upchain[this.currentLevel].readFrom();
        }

        if (this.spp() > 4) {
            // TODO: Only use the downchain when downsampling is needed!
            // Furthermore, only downsample as many levels as necessary!
            this.down.rebind();
        }
    }

    public endRead() {
        if (this.liveChanges() && this.framesSinceReset > 1 && this.currentLevel > 0) {
            this.upchain[this.currentLevel - 1].setLinear(false);
        } else {
            this.upchain[this.currentLevel].setLinear(false);
        }
    }

    private setSize(size: Size): void {        
        this.upchain = [];
        this.size = size.clone();
        const tsize = new Size(size.w * this.supersamples, size.h * this.supersamples);
        this.down.setSize(tsize, size);
        let area: number = tsize.area();
        let vert: boolean = true;
        while (area > 2000) {
            if (vert) {
                this.upchain.push(
                    new InterlacedProjection(tsize, this.gl, vert, this.vProg, false)
                );
                tsize.w = ~~(tsize.w / 2); // Integer division
            } else {
                this.upchain.push(
                    new InterlacedProjection(tsize, this.gl, vert, this.vProg, false)
                );
                tsize.h = ~~(tsize.h / 2);
            }
            vert = !vert;
            area = tsize.area();
        }
        this.currentLevel = this.upchain.length;
        this.push();
    }

    public setPush(pushFun: () => void): void {
        this.pushFunction = pushFun;
    }

    private push(): void {
        if (typeof this.pushFunction === "function") {
            this.pushFunction();
        }
    }

    public getSupersamples() {
        return this.supersamples;
    }

    private currentProgress: number;
    private currentLevel: number; // the index of the largest finished buffer or upchain.length
    private upchain: InterlacedProjection[]; // the buffers from largest to smallest
    private firstFrame: boolean;
    private framesSinceReset: number = 0;
    private optim: Optimizer;
    private size: Size;
    private target: Renderer;
    private gl: WebGLRenderingContext;
    private tProg: Program;
    private vProg: Program;
    private renderToRect: (r: Rect, s: Size) => void;
    private pushFunction: (() => void) | null = null;
    private supersamples: number;
    private down: Downchain;
    private win: Window;
}

// TODO: tiled Renderer, um in ultra-hohen Supersampling Modi Weniger RAM zu
// verbrauchen
