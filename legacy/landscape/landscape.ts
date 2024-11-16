import * as PIXI from 'pixi.js';


// points are always (cp2x, cp2y, x, y) and top,left is added to all coordinates
export function bezierLandscapeThrough(top: number, bottom: number,
    left: number, color, points: number[][]) {
    const shape = new PIXI.Graphics()
        .beginFill(0xa00000)
        .moveTo(left, bottom)
        .lineTo(left, top);


    let right = left;

    let prev = [0, 0, 0, 0];
    for (let p of points) {
        shape.bezierCurveTo(2 * prev[2] + left - prev[0],
            2 * prev[3] + top - prev[1],
            left + p[0], top + p[1], left + p[2], top + p[3])
        prev = p;
        right = left + p[2];
    }


    shape.lineTo(right, bottom).lineTo(left, bottom);
    return shape;
}
