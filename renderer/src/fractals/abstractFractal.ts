import { Rect } from "../global";


export abstract class Fractal {
    // the constructor shouldn't do much
    constructor() {}

    // initialize everything
    abstract create(gl: WebGLRenderingContext): Promise<void>;

    // update the fractal. Has to be called before every frame. Might request
    // animations through "requestFrame". // TODO: pass the time delta!
    abstract update(): void;

    // get an object with default values (position, rendering) // TODO: use more
    // specific getters
    abstract defaults(): any;

    // set the position in the fractal
    abstract setPos(x: number, y: number): void;

    // set the zoom of the fractal. X and Y should already normalize the aspect
    // ratio.
    abstract setZoom(x: number, y: number): void;

    // set the rect to draw
    abstract setRect(r: Rect): void;

    // draw into the rect
    abstract draw(): void;

    // bind the Fractal-Program
    abstract use(): void;

    // loads a preset from the name
    abstract loadPreset(name: string): void;

    // safely destruct the fractal
    abstract destroy(): void;

    // get a pretty Name for this fractal-class
    abstract getName(): string;

    // a boolean indicating that the renderer should discard its state and start
    // rendering the Fractal afresh
    public dirty: boolean = true;

    // a callback to be called whenever the fractal becomes invalid and has to
    // render a frame
    protected requestFrame: () => void = null;
    public setRequestFrame(requestFrame: () => void): void {
        this.requestFrame = requestFrame;
    }

    // return a list of presets - doesn't need create to be called, therefore it
    // is "almost static"
    static presets: {[name: string]: any};
}
