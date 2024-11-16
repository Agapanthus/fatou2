import {expect} from "chai";
import * as $ from "jquery";

export function updateLoader() {
    if ((window as any).loadingCounter < 0) {
        console.error("negative loading counter!");
    }
    if ((window as any).loadingCounter > 0) {
        $("#mainloader").css("display", "block"); //.css("opacity", "1");
    } else {
        $("#mainloader").css("display", "none"); //.css("opacity", "0");
    }
}

// format a number as approximate rational number
// similar to https://stackoverflow.com/a/19261123
export function rationalize(x: number, epsilon: number = 1.e-10): string {
    if (Math.abs(x - Math.round(x)) <= epsilon) {
        return x.toFixed(0);
    }

    let denominator = 0;
    let numerator;
    let error;

    const xNumerator = x;
    const xDenominator = 1;

    do {
        denominator++;
        numerator = Math.round((xNumerator * denominator) / xDenominator);
        error =
            Math.abs((xNumerator / xDenominator) - (numerator / denominator));
    } while (error > epsilon);
    return numerator.toFixed(0) + "/" + denominator.toFixed(0);
}

(window as any).loadingCounter = 1;

export function loading() {
    (window as any).loadingCounter++;
    updateLoader();
}

export function finishLoading() {
    (window as any).loadingCounter--;
    updateLoader();
}

export function uid() {
    if (!(window as any).uniqueIdCounter)
        (window as any).uniqueIdCounter = 1;
    return (window as any).uniqueIdCounter++;
}

export function crash(e: any) {
    setTimeout(() => { throw e; }, 0);
}

export function glthrow(gl: WebGLRenderingContext, where: string): void {
    const err = gl.getError();
    if (err !== gl.NO_ERROR)
        throw new Exception("GLError at " + where, err.toString());
}

export class Exception {
    constructor(title: string, details: string) {
        this.title = title;
        this.details = details;
    }
    public title: string;
    public details: string;
}

export class Size {
    constructor(width: number, height: number) {
        this.w = width;
        this.h = height;
    }
    public clone(): Size { return new Size(this.w, this.h); }
    public area(): number { return this.w * this.h; }

    public w: number;
    public h: number;
}

export class Rect {
    constructor(l: number, t: number, r: number, b: number) {
        this.l = l;
        this.t = t;
        this.r = r;
        this.b = b;
    }
    public l: number = 0;
    public t: number = 0;
    public r: number = 0;
    public b: number = 0;
}

export function topoSort<A>(edges: A[][]): A[] {
    const nodes: {[id: string]: {v: A, next: string[]}} = {};
    const sorted: A[] = [];
    const visited: {[id: string]: boolean} = {};
    const tmp: {[id: string]: boolean} = {};

    // build data structures
    for (const e of edges) {
        expect(e).to.be.an("array").of.length(2);
        const from = "" + e[0];
        const to = "" + e[1];
        if (!nodes[from])
            nodes[from] = {next : [], v : e[0]};
        if (!nodes[to])
            nodes[to] = {next : [], v : e[1]};
        nodes[from].next.push(to);
    }

    // topo sort
    const visit =
        (id: string) => {
            const node = nodes[id];
            tmp[id] = true;
            
            for (const next of node.next) {
                if (tmp[next]) // cycle
                    throw new Error('No DAG: ' + next + ' is in ' + id);
                if (!visited[next])
                    visit(next);
            }

            tmp[id] = false;
            visited[id] = true;
            sorted.push(node.v);
        }

    for (const id of Object.keys(nodes)) {
        if (!visited[id] && !tmp[id])
            visit(id);
    }

    return sorted;
}
