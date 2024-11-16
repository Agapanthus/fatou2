import * as $ from "jquery";
import * as f from "./global";
import { Program } from "./program";

export class Renderer {
    protected gl: WebGLRenderingContext;
    public getSize: () => f.Size;
    public render: () => void;
    public onResize: (size: f.Size) => void;
}

let removePixelRatioListener: null | (() => void) = null;
const updatePixelRatio = (callback) => {
    if (removePixelRatioListener != null) {
        removePixelRatioListener();
    }
    const mqString = `(resolution: ${window.devicePixelRatio}dppx)`;
    const media = matchMedia(mqString);
    media.addListener(updatePixelRatio);
    removePixelRatioListener = function () {
        media.removeListener(updatePixelRatio);
    };

    console.log("devicePixelRatio: " + window.devicePixelRatio);
    callback();
};

export class PlaceholderRenderer extends Renderer {
    constructor(placeholder: string) {
        super();
        this.placeholder = placeholder;
        $("<canvas id='glscreen'></canvas>").prependTo($(placeholder));
        this.gl = this.getCanvas().getContext("webgl2", { stencil: true }) as WebGLRenderingContext; // "experimental-webgl" // TODO: What to use
        // here?! Webgl2 is needed for bitwise logic,
        // but only supported by ca. 80% of devices!
        if (!this.gl.getContextAttributes().stencil)
            throw new f.Exception("No Stencilbuffer", "PlaceholderRenderer.constructor");
        //        onRender(0);
        //   this.onRender = onRender;
        const me = this;
        $(window).resize(() => {
            me.refresh();
        });
        this.refresh();
        this.getSize = () => {
            return new f.Size(this.gl.drawingBufferWidth, this.gl.drawingBufferHeight);
        };
        this.render = () => {
            this.bind();
        };

        updatePixelRatio(() => {
            me.refresh();
        });
    }

    public getCanvas(): HTMLCanvasElement {
        return $("#glscreen", $(this.placeholder))[0] as HTMLCanvasElement;
    }

    public getGL() {
        return this.gl;
    }
    public bind(): void {
        this.gl.bindFramebuffer(this.gl.FRAMEBUFFER, null);
        this.gl.viewport(0, 0, this.gl.drawingBufferWidth, this.gl.drawingBufferHeight);
    }

    private refresh(this: PlaceholderRenderer): void {
        const placeholder = $(this.placeholder);

        const canvas = this.getCanvas();
        // https://www.khronos.org/webgl/wiki/HandlingHighDPI
        canvas.style.width = placeholder.width() + "px";
        canvas.style.height = placeholder.height() + "px";
        const dpr = window.devicePixelRatio || 1;
        canvas.width = Math.round(placeholder.width() * dpr);
        canvas.height = Math.round(placeholder.height() * dpr);
        console.log(
            "drawing buffer size changed:",
            this.gl.drawingBufferWidth,
            this.gl.drawingBufferHeight
        );
        this.gl.viewport(0, 0, this.gl.drawingBufferWidth, this.gl.drawingBufferHeight);
        if (typeof this.onResize === "function") this.onResize(this.getSize());
    }

    public destroy() {
        this.gl.getExtension("WEBGL_lose_context").loseContext();
    }

    private placeholder: string;
}

export class Projection extends Renderer {
    constructor(
        size: f.Size,
        gl: WebGLRenderingContext,
        useRenderbuffer: boolean = false,
        linear: boolean = true
    ) {
        super();
        this.gl = gl;
        this.useRenderbuffer = useRenderbuffer;
        this.framebuffer = gl.createFramebuffer();
        gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);

        this.texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, this.texture);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, linear ? gl.LINEAR : gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, linear ? gl.LINEAR : gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        // gl.generateMipmap(gl.TEXTURE_2D);
        f.glthrow(gl, "Projection.constructor1");

        this.scaleI(size);
        gl.framebufferTexture2D(
            gl.FRAMEBUFFER,
            gl.COLOR_ATTACHMENT0,
            gl.TEXTURE_2D,
            this.texture,
            0
        );

        if (this.useRenderbuffer) {
            this.renderbuffer = gl.createRenderbuffer();
            gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderbuffer);
            gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_STENCIL, this.size.w, this.size.h);
            f.glthrow(gl, "projection.construct2");
            gl.framebufferRenderbuffer(
                gl.FRAMEBUFFER,
                gl.DEPTH_STENCIL_ATTACHMENT,
                gl.RENDERBUFFER,
                this.renderbuffer
            );
        }

        if (typeof gl.checkFramebufferStatus === "function") {
            // Not supported in IE and Edge
            if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) !== gl.FRAMEBUFFER_COMPLETE)
                throw new f.Exception(
                    "Projection.constructor",
                    "Framebuffer: " + gl.checkFramebufferStatus(gl.FRAMEBUFFER).toString()
                );
        }

        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindRenderbuffer(gl.RENDERBUFFER, null);
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);

        f.glthrow(gl, "projection.construct3");
        this.getSize = () => {
            return this.size.clone();
        };
        this.render = () => {
            this.writeTo();
        };
    }

    public destroy() {
        const gl = this.gl;
        gl.deleteTexture(this.texture);
        gl.deleteFramebuffer(this.framebuffer);
        if (this.renderbuffer) gl.deleteRenderbuffer(this.renderbuffer);
    }

    public scale(size: f.Size): void {
        this.scaleI(size);
    }

    private scaleI(size: f.Size): void {
        this.size = size.clone();
        const gl = this.gl;
        gl.bindTexture(gl.TEXTURE_2D, this.texture);
        gl.texImage2D(
            gl.TEXTURE_2D,
            0,
            gl.RGBA,
            this.size.w,
            this.size.h,
            0,
            gl.RGBA,
            gl.UNSIGNED_BYTE,
            null
        );
        f.glthrow(gl, "projection.scale");
        if (typeof this.onResize === "function") this.onResize(this.getSize());
    }

    public writeTo(): void {
        this.gl.bindFramebuffer(this.gl.FRAMEBUFFER, this.framebuffer);
        this.gl.viewport(0, 0, this.size.w, this.size.h);

        // ACHTUNG: Wenn mipmaps, nach dem Schreiben neu generieren!
    }

    public readFrom(texture = this.gl.TEXTURE0): void {
        this.gl.activeTexture(texture);
        this.gl.bindTexture(this.gl.TEXTURE_2D, this.texture);
    }

    /*  public framebufferRead(): void {
          this.gl.BindFramebuffer(this.gl.READ_FRAMEBUFFER, this.framebuffer);
      }

      public framebufferWrite(): void {
          this.gl.BindFramebuffer(this.gl.DRAW_FRAMEBUFFER, this.framebuffer);
      }*/

    private useRenderbuffer: boolean;
    private framebuffer: WebGLFramebuffer;
    private texture: WebGLTexture;
    private renderbuffer: WebGLRenderbuffer;
    private size: f.Size;
}

// This is a RenderBuffer where you can write each second row (or column).
// One set of lines is called "back" the other "front"
export class InterlacedProjection extends Projection {
    constructor(
        size: f.Size,
        gl: WebGLRenderingContext,
        vertical: boolean,
        prog: Program,
        linear: boolean
    ) {
        super(size, gl, true, linear);
        this.vertical = vertical;
        this.prog = prog;
        this.createRenderBuffer();
    }

    public scale(size: f.Size): void {
        super.scale(size);
        this.createRenderBuffer();
    }

    public isVertical(): boolean {
        return this.vertical;
    }

    private createRenderBuffer() {
        this.render();
        const gl = this.gl;
        gl.enable(gl.STENCIL_TEST);
        gl.stencilFunc(gl.ALWAYS, 1, 0xff); // Set any stencil to 1
        gl.stencilOp(gl.KEEP, gl.KEEP, gl.REPLACE);
        gl.stencilMask(0xff); // Write to stencil buffer
        gl.clear(gl.STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)

        gl.colorMask(false, false, false, false);
        gl.depthMask(false);

        this.prog.use();

        if (this.vertical) {
            const w = this.getSize().w;
            const s = 2.0 / w;
            for (let i = 0; i < w; i += 2) {
                this.prog.setRect("aPos", s * i - 1.0, -1.0, s * (i + 1) - 1.0, 1.0);
                this.prog.drawRect("aPos");
            }
        } else {
            const h = this.getSize().h;
            const s = 2.0 / h;
            for (let i = 0; i < h; i += 2) {
                this.prog.setRect("aPos", -1.0, s * i - 1.0, 1.0, s * (i + 1) - 1.0);
                this.prog.drawRect("aPos");
            }
        }

        gl.stencilFunc(gl.EQUAL, 1, 0xff); // Stencil value 1
        gl.colorMask(true, true, true, true);
        gl.disable(gl.STENCIL_TEST);
        f.glthrow(gl, "InterlacedProjection.createRenderBuffer");
    }

    public startWrite(front: boolean): void {
        const gl = this.gl;
        gl.enable(gl.STENCIL_TEST);
        gl.stencilFunc(gl.EQUAL, front ? 1 : 0, 0xff);
        super.writeTo();
    }

    // Switch to linear filtering instead of nearest filtering
    public setLinear(linear: boolean): void {
        const gl = this.gl;
        this.readFrom();
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, linear ? gl.LINEAR : gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, linear ? gl.LINEAR : gl.NEAREST);
    }

    public endWrite(): void {
        const gl = this.gl;
        gl.disable(gl.STENCIL_TEST);
    }

    private prog: Program;
    private vertical: boolean;
}

export class RenderLoop {
    constructor(
        renderF: (delta: number, continuos: boolean) => boolean,
        drawF: () => void,
        onSleep: () => void
    ) {
        this.renderF = renderF;
        this.drawF = drawF;
        this.onSleep = onSleep;

        requestAnimationFrame((millis: number) => {
            this.render(millis);
        });
    }

    // If the RenderLoop sleeps, the last frame is kept
    // Just call "push" whenever you need to start rendering again
    public push(): void {
        if (this.onTerminate) console.error("Pushing a dead loop!");
        else this.active = true;
    }

    public fps(): number | null {
        if (this.floatingDelta < 0) return null;
        return 1000 / this.floatingDelta;
    }

    private render(millis: number) {
        const ac = this.active;
        this.active = false;
        const delta: number = millis - this.renderFunLast;
        this.renderFunLast = millis;

        if (this.onTerminate) {
            this.onTerminate();
        } else {
            // the loop must continue running even when sleeping! Otherwise,
            // when starting again, there might be MULTIPLE slow frames!
            requestAnimationFrame((mi: number) => {
                this.render(mi);
            });
        }

        // The render function returns if there are changes which should be
        // presented to the user
        if (ac) {
            if (this.renderF(delta, this.con)) {
                this.drawF();
            }
            this.con = true;

            // recalculate fps
            const fpsSmoothing = 30;
            const ratio = Math.min(this.framesAwake, fpsSmoothing) / (fpsSmoothing + 1);
            this.floatingDelta = ratio * this.floatingDelta + (1 - ratio) * delta;
            this.framesAwake++;
        } else {
            this.framesAwake = 0;
            this.floatingDelta = -1;
            this.con = false;
            this.onSleep();
        }
    }

    private onTerminate: (() => void) | null = null;
    private con = false;
    private active: boolean;
    private renderF: (delta: number, continuos: boolean) => boolean;
    private drawF: () => void;
    private onSleep: () => void;
    private renderFunLast: number = 0;
    private floatingDelta: number = -1; // Smoothed medium frame-length in this awak-period.
    private framesAwake: number = 0; // number of frames since last sleep

    public terminate(): Promise<void> {
        this.active = false;
        return new Promise((res, rej) => {
            this.onTerminate = () => {
                res();
            };
        });
    }
}
