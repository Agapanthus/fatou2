
import * as f from "./global";
import { Program } from "./program";

export class Rect {
    constructor(gl: WebGLRenderingContext, program: Program, attr: string) {
        this.gl = gl;
        this.program = program;
        this.buffer = gl.createBuffer();
        this.position = this.program.getAttribLocation(attr);
        f.glthrow(gl, "Rect.constructor");
    }
    public set(left: number, top: number, right: number, bottom: number): void {
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.buffer);
        const verts = [
            left,
            bottom,
            left,
            top,
            right,
            top,
            left,
            bottom,
            right,
            top,
            right,
            bottom,
        ];
        this.gl.bufferData(this.gl.ARRAY_BUFFER, new Float32Array(verts),
                           this.gl.DYNAMIC_DRAW);
        this.gl.vertexAttribPointer(this.position, 2, this.gl.FLOAT, false, 0,
                                    0);
        this.gl.enableVertexAttribArray(this.position);
    }

    public draw(): void { this.gl.drawArrays(this.gl.TRIANGLES, 0, 6); }

    private position: number;
    private program: Program;
    private gl: WebGLRenderingContext;
    private buffer: WebGLBuffer;
}
