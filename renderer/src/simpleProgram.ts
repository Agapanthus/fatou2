import {glthrow} from "./global";
import {Program} from "./program";
import {Shader} from "./shader";

export interface IModActiveInfo extends WebGLActiveInfo {
    typeName: string;
}

// See here http://webglstats.com/webgl
export interface IProgramInfo {
    attributes: IModActiveInfo[];
    uniforms: IModActiveInfo[];
    attributeCount: number;
    uniformCount: number;
    maxVUnits: number;
    maxTUnits: number;
    maxTextures: number;
    maxViewport: number[];
    maxTextureSize: number;
}

export class SimpleProgram extends Program {
    private vs: Shader;
    private fs: Shader;
    protected prog: WebGLProgram;

    constructor(vertex: string, fragment: string, gl: WebGLRenderingContext,
                uniforms: string[], attributes: string[], optimize = true,
                reuseVertex: Shader = null) {
        super(gl);

        this.prog = gl.createProgram();
        this.gl = gl;
        this.vs =
            reuseVertex || new Shader(vertex, gl.VERTEX_SHADER, gl, optimize);
        gl.attachShader(this.prog, this.vs.internal());
        this.fs = new Shader(fragment, gl.FRAGMENT_SHADER, gl, optimize);
        gl.attachShader(this.prog, this.fs.internal());
        gl.linkProgram(this.prog);
        gl.useProgram(this.prog);

        // console.log(this.getProgramInfo());

        super.buildUniformMap(uniforms);
        super.buildRects(attributes);

        glthrow(gl, "Program.constructor");
    }

    public getUniformLocation(u: string): WebGLUniformLocation {
        return this.gl.getUniformLocation(this.prog, u)
    }

    // export the vertex shader from the program
    public vertexTakeout(): Shader {
        this.gl.detachShader(this.prog, this.vs.internal());
        const tmp = this.vs;
        this.vs = null;
        return tmp;
    }

    public destroy() {
        if (this.vs)
            this.gl.detachShader(this.prog, this.vs.internal());
        if (this.fs)
            this.gl.detachShader(this.prog, this.fs.internal());
        this.gl.deleteProgram(this.prog);
        if (this.vs)
            this.vs.destroy();
        if (this.fs)
            this.fs.destroy();
    }

    // https://bocoup.com/blog/counting-uniforms-in-webgl
    public getProgramInfo(): IProgramInfo {
        const gl = this.gl;
        const result: IProgramInfo = {
            attributes : [],
            uniforms : [],
            attributeCount : 0,
            uniformCount : 0,
            maxTUnits : gl.getParameter(gl.MAX_FRAGMENT_UNIFORM_VECTORS),
            maxVUnits : gl.getParameter(gl.MAX_VERTEX_UNIFORM_VECTORS),
            maxTextures : gl.getParameter(gl.MAX_TEXTURE_IMAGE_UNITS),
            maxViewport : gl.getParameter(gl.MAX_VIEWPORT_DIMS),
            maxTextureSize : gl.getParameter(gl.MAX_TEXTURE_SIZE)
        };
        const activeUniforms =
            this.gl.getProgramParameter(this.prog, this.gl.ACTIVE_UNIFORMS);
        const activeAttributes =
            this.gl.getProgramParameter(this.prog, this.gl.ACTIVE_ATTRIBUTES);

        // Taken from the WebGl spec:
        // http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.14
        const enums = {
            0x8B50 : 'FLOAT_VEC2',
            0x8B51 : 'FLOAT_VEC3',
            0x8B52 : 'FLOAT_VEC4',
            0x8B53 : 'INT_VEC2',
            0x8B54 : 'INT_VEC3',
            0x8B55 : 'INT_VEC4',
            0x8B56 : 'BOOL',
            0x8B57 : 'BOOL_VEC2',
            0x8B58 : 'BOOL_VEC3',
            0x8B59 : 'BOOL_VEC4',
            0x8B5A : 'FLOAT_MAT2',
            0x8B5B : 'FLOAT_MAT3',
            0x8B5C : 'FLOAT_MAT4',
            0x8B5E : 'SAMPLER_2D',
            0x8B60 : 'SAMPLER_CUBE',
            0x1400 : 'BYTE',
            0x1401 : 'UNSIGNED_BYTE',
            0x1402 : 'SHORT',
            0x1403 : 'UNSIGNED_SHORT',
            0x1404 : 'INT',
            0x1405 : 'UNSIGNED_INT',
            0x1406 : 'FLOAT'
        };

        let i: number = 0;
        // Loop through active uniforms
        for (i = 0; i < activeUniforms; i++) {
            const uniform =
                this.gl.getActiveUniform(this.prog, i) as IModActiveInfo;
            uniform.typeName = enums[uniform.type];
            result.uniforms.push(uniform);
            result.uniformCount += uniform.size;
        }

        // Loop through active attributes
        for (i = 0; i < activeAttributes; i++) {
            const attribute =
                this.gl.getActiveAttrib(this.prog, i) as IModActiveInfo;
            attribute.typeName = enums[attribute.type];
            result.attributes.push(attribute);
            result.attributeCount += attribute.size;
        }

        return result;
    }

    public getAttribLocation(attr: string): number {
        return this.gl.getAttribLocation(this.prog, attr);
    }

    public use(): void { this.gl.useProgram(this.prog); }
}
