import "./fatou/styles/styles.scss";

import { FractalRunner } from "./fatou/fractal";
import { finishLoading } from "./util/global";
import { initOptimizer } from "./fatou/GLSLOptimizer";

window.onload = () => {
    initOptimizer(() => {
        finishLoading();
        const runner = new FractalRunner();

        //setTimeout(() => { runner.destroy(); }, 1000);
    });
};
