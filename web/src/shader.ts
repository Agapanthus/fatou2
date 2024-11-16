import {Exception} from "./global";
import {optimizeFragmentShader} from "./GLSLOptimizer";

export class ShaderCompileError extends Exception {
    constructor(title: string, details: string, source: string) {
        super(title, details);
        this.source = source;
    }
    private source: string;
}

export class Shader {
    constructor(src: string, type: number, gl: WebGLRenderingContext,
                optimize) {
        this.shader = gl.createShader(type);
        this.gl = gl;

        if (type === gl.FRAGMENT_SHADER) {
            if (optimize) {
                src = this.getOptimized(src);
            }
        }

        gl.shaderSource(this.shader, src);
        gl.compileShader(this.shader);
        if (!gl.getShaderParameter(this.shader, gl.COMPILE_STATUS)) {
            throw new ShaderCompileError("Shader Compile Error [" + type + "]",
                                         gl.getShaderInfoLog(this.shader), src);
        }
    }
    public destroy() { this.gl.deleteShader(this.shader); }

    private getOptimized(src: string): string {
        //console.log(src);

        try {
            const optimizedSrc = optimizeFragmentShader(src);
            if (optimizedSrc.length < 15) {
                console.error("SHADER OPTIMIZATION FAILED", optimizedSrc)
                return src;
            }
            return optimizedSrc;

        } catch (e) {
            console.error("SHADER OPTIMIZATION FAILED", e, src)
            return src;
        }
    }
    private gl: WebGLRenderingContext;
    private shader: WebGLShader;
    public internal(): WebGLShader { return this.shader; }
}