import * as GLSLOpt from "glsl-optimizer-js";

const GLSLOptim = new GLSLOpt();

let optimizeGLSL = (a, b, c) => {
    alert('not loaded yet!');
    return "";
};

function postRun() {
    optimizeGLSL = GLSLOptim.cwrap('optimize_glsl', 'string',
                                   [ 'string', 'number', 'number' ]);
}

export function optimizeFragmentShader(source: string) {
    const results =
        optimizeGLSL(source, 2, false); // OpenGL ES 2.0, Fragment shader

    if (results.indexOf('Error:') > -1) {
        // GLSLOptimizer.onError(results);
        throw results;
        return "";
    } else {
        // GLSLOptimizer.onSuccess(results);
        // console.log(results);
        return results;
    }
}

export function initOptimizer(readyCall: () => void) {
    GLSLOptim.then(a => {
        postRun();
        readyCall();
    });
    const r = GLSLOptim.run();
}
