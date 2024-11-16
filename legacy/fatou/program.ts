import { Rect } from "./geom";
import { Exception, Rect as GRect } from "./global";

export abstract class Program {
    private rects: { [name: string]: Rect };
    private uniformMap: { [name: string]: { loc: WebGLUniformLocation; val: any[] } } = {};

    constructor(protected gl: WebGLRenderingContext) {}

    // ###########################################

    abstract use(): void;
    abstract getUniformLocation(name: string): WebGLUniformLocation;
    abstract getAttribLocation(name: string): number;
    abstract destroy(): void;

    // ###########################################

    protected buildUniformMap(uniforms: string[]) {
        this.uniformMap = {};
        for (const u of uniforms) {
            this.uniformMap[u] = { loc: this.getUniformLocation(u), val: [] };
            if (!this.uniformMap[u].loc)
                throw new Exception("Program.constructor uniform not found!", u);
        }
    }

    private find(name: string): WebGLUniformLocation {
        if (this.uniformMap[name]) return this.uniformMap[name].loc;
        throw new Exception("Uniform not found", name);
    }

    public set2f(name: string, value1: number, value2: number): void {
        this.gl.uniform2f(this.find(name), value1, value2);
        this.uniformMap[name].val = [value1, value2];
    }

    public set4f(
        name: string,
        value1: number,
        value2: number,
        value3: number,
        value4: number
    ): void {
        this.gl.uniform4f(this.find(name), value1, value2, value3, value4);
        this.uniformMap[name].val = [value1, value2, value3, value4];
    }

    public set2d(name: string, value1: number, value2: number): void {
        this.gl.uniform4f(
            this.find(name),
            Math.fround(value1),
            Math.fround(value1 - Math.fround(value1)),
            Math.fround(value2),
            Math.fround(value2 - Math.fround(value2))
        );
        this.uniformMap[name].val = [value1, value2];
    }

    public seti(name: string, value: number): void {
        this.gl.uniform1i(this.find(name), value);
        this.uniformMap[name].val = [value];
    }

    public set1fv(name: string, values: number[]): void {
        this.gl.uniform1fv(this.find(name), values);
        this.uniformMap[name].val = values;
    }

    public get(name: string) {
        if (this.uniformMap[name]) return this.uniformMap[name].val;
        throw new Exception("Uniform not found", name);
    }

    // ###########################################

    protected buildRects(attributes: string[]) {
        this.rects = {};
        for (const a of attributes) {
            this.rects[a] = new Rect(this.gl, this, a);
            if (!this.rects[a]) throw new Exception("Program.constructor attribute not found!", a);
        }
    }

    private findRect(name: string): Rect {
        if (this.rects[name]) return this.rects[name];
        throw new Exception("Attribute not found", name);
    }

    public setRect(name: string, l: number, t: number, r: number, b: number) {
        this.findRect(name).set(l, t, r, b);
    }

    public setRect2(name: string, r: GRect) {
        this.findRect(name).set(r.l, r.t, r.r, r.b);
    }

    public drawRect(name: string) {
        this.findRect(name).draw();
    }
}
