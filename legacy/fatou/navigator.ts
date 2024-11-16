import * as Hammer from "hammerjs";
import * as $ from "jquery";



// TODO: Bug, when moving and putting a second finger on the touch screen!

export class Navigator {
    constructor(element: HTMLElement, defaults: any = {}) {
        this.element = element;

        this.setPos(defaults.x || 0.0, defaults.y || 0.0, defaults.zoom);

        this.moving = false;

        /*const tFun = ()=> {
            this.posX = Math.sin((new Date()).getTime() / 2000.0) * 900 - 1200;
            if(typeof this.onChangeFun === "function") {
                this.onChangeFun();
            }
            setTimeout(tFun, 10);
        };
        tFun();*/

        const me = this;
        $(element).bind("mousewheel DOMMouseScroll", (event) => {
            let de = (event.originalEvent as any).wheelDelta / 2500;
            if (!de)
                de = -(event.originalEvent as any).detail / 20;

            if (de > 0) {
                me.zoom /= (1 + de);
            } else if (de < 0) {
                me.zoom *= (1 - de);
            }
            me.zooma = me.zoom;

            if (typeof this.onChangeFun === "function") {
                this.onChangeFun(this);
            }
        });

        if (this.hammer)
            this.hammer.destroy();
        this.hammer = new Hammer(element);
        this.hammer.get("pan").set({direction : Hammer.DIRECTION_ALL});
        // hammertime.get("swipe").set({ direction: Hammer.DIRECTION_ALL });
        this.hammer.get("pinch").set({enable : true});
        this.hammer.on("pan", (ev) => {
            // console.log(ev.scale+ " " + ev.deltaX + " " + ev.deltaY);
            // if(ev.isFirst) console.log("first!");
            // if(ev.isFinal) console.log("done!");

            this.posX = this.posXa - this.zoom * ev.deltaX;
            this.posY = this.posYa + this.zoom * ev.deltaY;

            // if(ev.isFinal) {
            if (4 === ev.eventType) {
                this.moving = false;
                this.posXa = this.posX;
                this.posYa = this.posY;
            } else {
                this.moving = true;
            }

            if (typeof this.onChangeFun === "function") {
                this.onChangeFun(this);
            }
        });

        this.hammer.on("pinch", (ev) => {
            this.posX = this.posXa - this.zoom * ev.deltaX;
            this.posY = this.posYa + this.zoom * ev.deltaY;
            this.zoom = this.zooma / ev.scale;

            if (4 === ev.eventType) {
                this.moving = false;
                // this.posXa = this.posX;
                // this.posYa = this.posY;
                this.zooma = this.zoom;
            } else {
                this.moving = true;
            }

            if (typeof this.onChangeFun === "function") {
                this.onChangeFun(this);
            }
        });

        /* hammertime.on("tap", (ev)=> {
             console.log("tap");
         });*/
        this.hammer.on("press doubletap", (ev) => { console.log("Interact"); });
    }

    public interpolate(delta: number): void {
        /*
        // TODO: Sieht noch nicht so hübsch aus...
          const g = Math.pow(1.01, -delta);
          this.rPosX = this.rPosX * (1-g) + this.posX * g;
          this.rPosY = this.rPosY * (1-g) + this.posY * g;
          this.rZoom = this.rZoom * (1-g) + this.zoom * g;
          */

        this.rPosX = this.posX;
        this.rPosY = this.posY;
        this.rZoom = this.zoom;
    }

    public setPos(x: number, y: number, z: number) {
        x *= $(this.element).width();
        y *= $(this.element).height();
        this.posX = x;
        this.posY = y;
        this.zoom = z;
        this.posXa = x;
        this.posYa = y;
        this.zooma = z;
        this.rPosX = x;
        this.rPosY = y;
        this.rZoom = z;
    }

    public destroy() { this.hammer.destroy(); }

    public setOnChangeFun(cf: (navi:Navigator) => void): void { this.onChangeFun = cf; }
    private onChangeFun: (navi:Navigator) => void;

    public getPosX(): number { return this.rPosX / $(this.element).width(); }
    public getPosY(): number { return this.rPosY / $(this.element).height(); }
    public getZoom(): number { return this.rZoom; }
    public isMoving(): boolean { return this.moving; }

    private posX: number; // Target Values
    private posY: number;
    private zoom: number;

    private posXa: number; // Base Values for operations
    private posYa: number;
    private zooma: number;

    private posXap: number; // Base Values for pan operations
    private posYap: number;

    private moving: boolean; // you should rerender using targetValues every
                             // time it changes from true to false

    private rPosX: number; // The real Values
    private rPosY: number;
    private rZoom: number;

    private hammer: HammerManager;

    /*private dPosX: number;
    private dPosY: number;
    private dZoom: number;*/

    private element: HTMLElement;
}