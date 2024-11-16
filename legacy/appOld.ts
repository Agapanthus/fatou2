import * as PIXI from 'pixi.js';
import "pixi-spine"
import "./styles/app.scss"

/*
import { bezierLandscapeThrough } from './landscape/landscape'
import { Character } from './character/character'
import { MainCharacter } from "./character/mainCharacter";
*/

const vertShader = require('./shaders/main.vert').default
const shader1 = require('./shaders/clouds2/sky.frag').default

// The application will create a renderer using WebGL, if possible,
// with a fallback to a canvas render. It will also setup the ticker
// and the root stage PIXI.Container
const app = new PIXI.Application({
    width: window.innerWidth,
    height: 963, // window.innerHeight,
    antialias: true
});


// Listen for window resize events
window.addEventListener('resize', resize);

// Resize function window
function resize() {
    // Resize the renderer
    app.renderer.resize(window.innerWidth, 963); //window.innerHeight);
}
resize();
app.renderer.backgroundColor = 0xFFFFFF;

app.stage.interactive = true;

//////////////////////////////////////


// TODO: For WebGL only
PIXI.settings.SCALE_MODE = PIXI.SCALE_MODES.NEAREST;
PIXI.settings.ANISOTROPIC_LEVEL = 0;
PIXI.settings.MIPMAP_TEXTURES = 0;


// load the texture we need

//const mainChar = new MainCharacter()
//mainChar.load(app.loader)
app.loader
    .add('bunny', 'src/shaders/blue noise.png')

    .load((loader, res) => {

        // The application will create a canvas element for you that you
        // can then insert into the DOM
        document.body.appendChild(app.view);

        ///////////////////////////////////////


        const fullScreen = new PIXI.Sprite(res.bunny.texture);


        const timeStart = Date.now();
        const uniforms = {
            "u_resolution": { x: app.renderer.width, y: app.renderer.height },
            "u_time":0,
            "u_mouse": {x:0, y:app.renderer.height / 2},
            
        };
        //Get shader code as a string
        let shaderCode = shader1
        console.log(shaderCode)
        //Create our Pixi filter using our custom shader code
        let myShader = new PIXI.Filter(vertShader, shaderCode, uniforms);
        //Apply it to our object
        fullScreen.filters = [myShader]

        let timeBonus = 0;

        fullScreen.anchor.x = 0.5;
        fullScreen.anchor.y = 0.5;

        let left = false;
        let right = false;

        app.stage.addChild(fullScreen);
        app.ticker.add((deltaTime) => {

            let w = app.renderer.width;
            let h = app.renderer.height;
            if(w < 1024) w = 1024;
            if(h < 1024) h = 1024;
            fullScreen.x = w / 2;
            fullScreen.y = h / 2;
            fullScreen.width = w;
            fullScreen.height = h;

            if(left) {
                timeBonus += deltaTime*1000.0;
            } else if(right) {
                timeBonus -= deltaTime*1000.0;

            }

            myShader.uniforms.u_resolution = { x: app.renderer.width, y: app.renderer.height };
            myShader.uniforms.u_time = (Date.now() - timeStart + timeBonus) / 1000;
            //myShader.uniforms.u_mouse = ;
        });

        document.addEventListener("keydown", (e) => {

            if (e.keyCode == 37) {
                // left arrow
                left = true
                
            }
            else if (e.keyCode == 39) {
                right = true
            }
        });
        document.addEventListener("keyup", (e) => {

            if (e.keyCode == 37) {
                // left arrow
                left = false
                
            }
            else if (e.keyCode == 39) {
                right = false
            }
        });

        fullScreen.interactive = true;
 
        fullScreen.on('mousemove', (event) => {
            myShader.uniforms.u_mouse = {x: event.data.global.x, y: app.renderer.height - event.data.global.y};
        });


        //////////////////////////////////////

        //app.stage.addChild(bezierLandscapeThrough(100, 200, 10, 0xa00000, [[90, -50, 180, 0], [310, 50, 340, 0], [400, 0, 400, 0]]));

        ////////////////////////////////////////

        //mainChar.setup(res, app);

    });

