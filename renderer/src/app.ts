import "./styles/styles.scss";

import {FractalRunner} from "./fractal";
import {finishLoading} from "./global";
import {enableJS} from "./sciptblocker";
import { initOptimizer } from "./GLSLOptimizer";

window.onload = () => {
    enableJS();

    initOptimizer(() => {
        finishLoading();
        const runner = new FractalRunner();

        //setTimeout(() => { runner.destroy(); }, 1000);
    });
};
