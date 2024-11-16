import { Controller, CharacterState } from "../interface/controller";
import * as PIXI from "pixi.js";
import "pixi-spine";

export const TRACK_MAIN = 0;
export const TRACK_BLINK = 1;
export const TRACK_SPECIAL = 2;

function addBlinking(app, char, mini, maxi) {
    let blinkIn = new Date().getTime() + mini + Math.random() * (maxi - mini);

    app.ticker.add((deltaTime) => {
        if (blinkIn <= new Date().getTime()) {
            blinkIn = mini + new Date().getTime() + Math.random() * (maxi - mini);
            char.state.addAnimation(TRACK_BLINK, "blink", false, 0);
        }
    });
}

export function createVariatedSkin(baseSkin: string, skins: string[], char) {
    const skin = "combined-skin-" + Math.random(); // TODO: global counter

    /*
    // create new empty skin
    const newSkin = <any>new PIXI.spine.core.Skin(skin);
    // add base skin
    newSkin.addSkin(char.spineData.findSkin(baseSkin));
    // add parital skins over base
    for (let s of skins) {
        newSkin.addSkin(char.spineData.findSkin(s));
    }

    char.skeleton.setSkin(newSkin);

    //Reset slots to remove not used attachments
    char.skeleton.setSlotsToSetupPose();
*/
    return skin;
}

var char_count = 0;
export class Character {
    resName: string;
    json: string;
    walkSpeed: number = 5.0;
    controller: Controller = new Controller();
    scale: number = 0.2;

    constructor(json: string) {
        this.resName = "character_" + char_count;
        char_count += 1;
        this.json = json;
    }
    load(loader) {
        return loader.add(this.resName, this.json);
    }

    setup(res, app) {
        // create a spine boy
       /* const spine = new PIXI.spine.Spine(res[this.resName].spineData);

        spine.scale.set(this.scale);

        // play animation
        spine.state.setAnimation(0, "idle", true);

        addBlinking(app, spine, 3000, 8000);

        app.stage.addChild(spine);

        let idling = true;
        let directionMul = 1.0;
        app.ticker.add((deltaTime) => {
            // set the position
            spine.x = this.controller.posX;
            spine.y = app.screen.height;

            if (this.controller.state != CharacterState.idle) {
                if (idling) {
                    spine.state.setAnimation(TRACK_MAIN, "walk", true);
                    idling = false;
                }
            } else {
                if (!idling) {
                    spine.state.setAnimation(TRACK_MAIN, "idle", true);
                    idling = true;
                }
            }

            if (this.controller.faceLeft) {
                directionMul = -1;
                // Use this instead for pseudo-3d-rotation
                // Math.max(-1, directionMul - deltaTime*0.2)
            } else {
                directionMul = 1;
            }
            spine.scale.x = directionMul * this.scale;

            this.controller.update(deltaTime, this.walkSpeed);
        });
        return spine;*/
    }
}
