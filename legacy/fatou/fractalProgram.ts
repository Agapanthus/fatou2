import {BakeableProgram} from "./bakeableProgram";
import {Program} from "./program";

// @ts-ignore
import vertex_pass from "./shaders/vertex/pass.vs"

export class FractalProgram extends Program {
    protected single: BakeableProgram;
    protected double: BakeableProgram;
    private isDouble: boolean;

    constructor(fragmentSrc: string, gl: WebGLRenderingContext,
                uniforms: string[], bakeable: string[],
                forceWebGL2: boolean = false) {
        super(gl);

        uniforms.push("zoom");
        uniforms.push("pos");

     //   console.log(vertex_pass,fragmentSrc)

        this.double = new BakeableProgram(
            vertex_pass, fragmentSrc, gl, uniforms, bakeable,
            [ "aPos", "aTexCoords" ], true, {precision : "64", INDEX : "A"});

        this.single =
            new BakeableProgram(vertex_pass, fragmentSrc, gl, uniforms,
                                bakeable, [ "aPos", "aTexCoords" ], forceWebGL2,
                                {precision : "32", INDEX : "1000"});

        this.setDouble(false);
    }

    public setDouble(isDouble: boolean): void {
        this.isDouble = isDouble;
        super.buildUniformMap(this.getProg().getUniforms());
        super.buildRects([ "aPos", "aTexCoords" ]);
    }

    public destroy(): void {
        this.single.destroy();
        this.double.destroy();
    }

    private getProg(): BakeableProgram {
        if (this.isDouble) {
            return this.double;
        } else {
            return this.single;
        }
    }

    public use(): void { this.getProg().use(); }

    public getUniformLocation(u: string): WebGLUniformLocation {
        return this.getProg().getUniformLocation(u);
    }

    public getAttribLocation(u: string): number {
        return this.getProg().getAttribLocation(u);
    }

    public set2fd(u: string, x: number, y: number) {
        if (this.isDouble) {
            this.set2d(u, x, y);
        } else {
            this.set2f(u, x, y);
        }
    }
}