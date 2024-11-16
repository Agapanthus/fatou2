import { Character, createVariatedSkin, TRACK_MAIN } from "./character"
//import { getRandomSubarray } from "../util/util";
import { Controller, UserController } from "../interface/controller";
import { getRandomSubarray } from "../util/util";


function setUpTransitions(char) {
    // Setup transitions between animations!
    char.stateData.setMix('idle', 'dance', 1.0);
    char.stateData.setMix('dance', 'walk', 0.2);
    char.stateData.setMix('walk', 'idle', 0.2);
    char.stateData.setMix('idle', 'walk', 0.2);
    char.stateData.setMix('idle', 'dress-up', 0.2);
    char.stateData.setMix('dance', 'dress-up', 0.2);
    char.stateData.setMix('dress-up', 'walk', 0.4);
    char.stateData.setMix('dress-up', 'dance', 0.4);
}

export class MainCharacter extends Character {

    constructor() {
        super('res/chars/mix-and-match/export/mix-and-match.json')
    }

    setup(res, app) {
        const spine = super.setup(res, app)

        //const anim = ["idle", "dress-up", "blink", "dance", "aware", "walk", 

        setUpTransitions(spine)

        const skins = ["legs/boots-pink", "clothes/dress-blue", "clothes/dress-green", "clothes/hoodie-blue-and-scarf", "clothes/hoodie-orange", "accessories/bag", "hair/blue", "hair/brown", "legs/boots-pink", "legs/pants-jeans", "nose/long", "nose/short", "accessories/scarf", "accessories/cape-blue", "accessories/cape-red", "accessories/hat-red-yellow", "hair/short-red"]
        const fskins = ["full-skins/boy", "full-skins/girl", "full-skins/girl-spring-dress", "full-skins/girl-blue-cape"]
        let baseSkin = getRandomSubarray(fskins, 1)[0]
        let variations = []
        createVariatedSkin(baseSkin, variations, spine)

        /*setInterval(() => {
            if (Math.random() < 0.4) {
                variations = [getRandomSubarray(skins, 1)[0], ...variations]
            } else if (Math.random() > 0.5 && variations.length > 0) {
                variations = getRandomSubarray(variations, variations.length - 1)
            }
            console.log(variations)
            createVariatedSkin(baseSkin, variations, spineMix)
    
    
            //spineMix.state.setAnimation(0, getRandomSubarray(anim,1)[0], true);
    
        }, 1000);*/

        /*

        this.controller = new UserController(() => {
            spine.state.setAnimation(TRACK_MAIN, 'dance', true);
        }, () => {
            spine.state.setAnimation(TRACK_MAIN, 'dress-up', false);
        }, () => {
            baseSkin = getRandomSubarray(fskins, 1)[0]
            variations = []
            createVariatedSkin(baseSkin, variations, spine)
        })*/

        return spine
    }
}
