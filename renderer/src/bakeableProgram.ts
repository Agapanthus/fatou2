import {topoSort} from "./global";
import {Program} from "./program";
import {Shader} from "./shader";
import {SimpleProgram} from "./simpleProgram";

export class BakeableProgram extends Program {
    protected prog: SimpleProgram = null;
    protected cachedVertexShader: Shader = null;

    constructor(private vertexSrc: string, private fragmentSrc: string,
                gl: WebGLRenderingContext, private uniforms: string[],
                private bakeable: string[], private attributes: string[],
                useWebgl2: boolean, context: {[name: string]: string}) {
        super(gl);
        this.bake(useWebgl2, context);
    }

    private static includeRegex = /#include\s*"([\w\-]+)\.fs"\s*\n/gm;

    private removeIncludes(s: string): string {
        return s.replace(BakeableProgram.includeRegex, "");
    }

    private getIncludes(s: string): string[] {
        const res = [];
        while (true) {
            const match = BakeableProgram.includeRegex.exec(s);
            if (match === null)
                break;
            res.push(match[1])
        }

        return res;
    }

    private preprocess(s: string, context: {[name: string]: string}): string {
        for (const bake of Object.keys(context)) {
            s = s.replace(new RegExp("{{" + bake + "}}", 'g'), context[bake]);
        }
        return s;
    }

    private resolveIncludes(s: string,
                            context: {[name: string]: string}): string {
        s = this.preprocess(s, context);

        const visited = [];
        const getIncludesRe = (id: string, incs: string[]):
            string[][] => {
                let res: string[][] = [];
                visited.push(id);
                for (const inc of incs) {
                    res.push([ id, inc ]);
                    if (visited.includes(inc))
                        continue;
                    const src = this.preprocess(
                        require("./shaders/preamble/" + inc + ".fs"), context);
                    res = res.concat(getIncludesRe(inc, this.getIncludes(src)));
                }
                return res;
            }

        let preamble = "\n"
        const sortedIncs = topoSort(getIncludesRe(null, this.getIncludes(s)));
        for (let inc of sortedIncs) {
            let src = s;
            if (inc !== null) {
                src = require("./shaders/preamble/" + inc + ".fs");
            } else
                inc = "main";

            preamble += "\n// ####### " + inc + " #######\n\n" +
                        this.removeIncludes(this.preprocess(src, context)) +
                        "\n";
        }

        return preamble;
    }

    public bake(useWebgl2: boolean, context: {[name: string]: string}) {
        if (this.prog) {
            this.cachedVertexShader = this.prog.vertexTakeout();
            this.prog.destroy();
            this.prog = null;
        }

        let fs = this.fragmentSrc;
        let vs = this.vertexSrc;

        if (useWebgl2) {
            vs = "#version 300 es\n// <![CDATA[\n" + vs + "\n// ]]>\n";
            fs = "#version 300 es\n// <![CDATA[\n" +
                 this.resolveIncludes(fs, context) + "\n// ]]>\n";
        } else {
            // TODO: Implement INDEX using baking!
            vs = "#version 100\n// <![CDATA[\n" + vs + "\n// ]]>\n";
            fs = "#version 100\n// <![CDATA[\n" +
                 this.resolveIncludes(fs, context) + "\n// ]]>\n";

            // Downgrade GLSL-ES-3.0 to GLSL-100
            vs = vs.replace(/^out\s+([^\n\;]+)\;\s*\n/gm, "varying $1;\n");
            vs = vs.replace(
                /^(layout\(location\s*=\s*\d+\)\s+)?in\s+([^\n\;]+)\;\s*\n/gm,
                "attribute $2;\n");

            fs = fs.replace(/^in\s+([^\n\;]+)\;\s*\n/gm, "varying $1;\n");
            fs = fs.replace(/^out\s+([^\n\;]+)\;\s*\n/gm, "\n");
            fs = fs.replace(/([^A-Za-z0-9\_]|^)fragColor([^A-Za-z0-9\_]|$)/gm,
                            "$1gl_FragColor$2");
        }

        // TODO: Bake bakeable uniforms!

        //console.log(fs.split("\n").slice(0,131).join("\n"));

        this.prog =
            new SimpleProgram(vs, fs, this.gl, this.uniforms, this.attributes,
                              true, this.cachedVertexShader);

        super.buildUniformMap(this.getUniforms());
        super.buildRects(this.attributes);
    }

    public getUniforms(): string[] {
        // TODO: exclude baked uniforms!
        return this.uniforms;
    }

    public getUniformLocation(u: string): WebGLUniformLocation {
        return this.prog.getUniformLocation(u);
    }

    public getAttribLocation(u: string): number {
        return this.prog.getAttribLocation(u);
    }

    public use() { this.prog.use(); }

    public destroy() { this.prog.destroy(); }
}
