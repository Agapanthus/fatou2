import { expect } from "chai";
import * as interact from "interactjs";

import * as $ from "jquery";
import {uid} from "./global";

export class Window {
    private refresher: any;

    constructor(posX: number, posY: number, w: number, h: number, title: string,
                resizeable: boolean, closeable: boolean) {
        this.id = uid();
        this.title = title;

        // Refresh stats
        this.refresher = setInterval(() => { this.updateDOM(); }, 500);

        if (posX < 0) {
            posX = 0.2; // TODO: Autochoose!
        }
        if (posY < 0) {
            posY = 0.2; // TODO: Autochoose!
        }
        $("#GUIBase")
            .append("<div class='window' id='w" + this.id +
                    "' style='touch-action: none'></div>");

        this.j = $('#w' + this.id);
        this.j.css("width", w + "px")
            .css("height", h + "px")
            .css("transform", "translate(" + (posY * $("#GUIBase").height()) +
                                  "px," + (posX * $("#GUIBase").width()) +
                                  "px)");
        this.j.append("<div class='title'>" + title + "</div>");
        this.j.append("<div class='content' id='c" + this.id + "'></div>");
        (interact as any)('#w' + this.id + " .title").draggable({
            onmove : (event) => {
                const target = (event.target as HTMLElement).parentElement;
                // keep the dragged position in the data-x/data-y attributes
                const x =
                    (parseFloat(target.getAttribute('data-x')) || 0) + event.dx;
                const y =
                    (parseFloat(target.getAttribute('data-y')) || 0) + event.dy;

                // translate the element
                target.style.webkitTransform = target.style.transform =
                    'translate(' + x + 'px, ' + y + 'px)';

                // update the posiion attributes
                target.setAttribute('data-x', x);
                target.setAttribute('data-y', y);
            },
            restrict : {
                restriction : document.getElementById('mainCanvas'),
                // elementRect: { top: 0, left: 0, bottom: 1, right: 1 }
            },
        });

        if (resizeable) {
            (interact as any)('#w' + this.id)
                .resizable({
                    // resize from all edges and corners
                    // TODO: Typescript korrigieren!
                    edges :
                        {left : true, right : true, bottom : true, top : true},

                    margin : 8,

                    // keep the edges inside the parent
                    restrictEdges : {
                        outer : document.getElementById('mainCanvas'),
                        endOnly : true,
                    },

                    // minimum size
                    restrictSize : {
                        min : {width : 150, height : 100},
                    },

                    inertia : true,
                } as any)
                .on('resizemove', (event) => {
                    const target = event.target;
                    let x = (parseFloat(target.getAttribute('data-x')) || 0);
                    let y = (parseFloat(target.getAttribute('data-y')) || 0);

                    // update the element's style
                    target.style.width = (event as any).rect.width + 'px';
                    target.style.height = (event as any).rect.height + 'px';

                    // translate when resizing from top or left edges
                    x += (event as any).deltaRect.left;
                    y += (event as any).deltaRect.top;

                    target.style.webkitTransform = target.style.transform =
                        'translate(' + x + 'px,' + y + 'px)';

                    target.setAttribute('data-x', x);
                    target.setAttribute('data-y', y);
                });
        }
    }

    public updateDOM() {

        // Update texts
        for (const title of Object.keys(this.texts)) {
            if (this.texts[title].values > 0) {
                const id = "text-" + this.id + "-" + this.texts[title].id;
                let v = this.texts[title].value
                if (this.texts[title].digits >= 0) {
                    v = (v as any).toFixed(this.texts[title].digits);
                }
                document.querySelector("#" + id + " .value").textContent =
                    v + "";
                this.texts[title].values = 0;
            }
        }
    }

    public content(): string { return "#c" + this.id; }

    private texts: {
        [key: string]:
            {id: number, values: number, value: number|string, digits: number}
    } = {};

    // set lazy to true to show the values only twice per second
    // set smooth to true, to show the mean of the values since the last
    public setText(title: string, value: string|number, lazy: boolean = false,
                   smooth: boolean = true, digits: number = -1) {

        let idn = 0;
        if (this.texts[title]) {
            idn = this.texts[title].id;
        } else {
            idn = uid();
            this.texts[title] = { id : idn, values : 0, value : null, digits }
        }

        const id = "text-" + this.id + "-" + idn;
        if (!document.getElementById(id)) {
            $(this.content())
                .append("<div class='text' id='" + id +
                        "'><span class='text-title'>" + title +
                        ": </span><span class='value'></span></div>")
        }

        if (lazy) {
            if (smooth) {
                this.texts[title].value =
                    ((this.texts[title].values as any) *
                         (this.texts[title].value as any) +
                     (value as any)) /
                    (this.texts[title].values as any + 1);
                this.texts[title].values++;
            } else if (this.texts[title].value !== value) {
                this.texts[title].values = 1;
                this.texts[title].value = value;
            }

        } else {
            this.texts[title].values = 0;
            if (this.texts[title].value !== value) {
                this.texts[title].value = value;
                document.querySelector("#" + id + " .value").textContent =
                    value + "";
            }
        }
    }

    private sliders: {
        [key: string]: {
            id: number,
            base: number,
            callback: ((v: number) => void),
            integer: boolean,
            defaults: number[]
        }
    } = {}

    public getSlider(name: string): number {
        const id = this.sliders[name].id;
        const slider = document.querySelector("#slider-" + id + " input") as
                       HTMLInputElement;
        const v = Math.pow(parseFloat(slider.value), this.sliders[name].base)
        return this.sliders[name].integer ? Math.round(v) : v
    }

    private sliderRefresh(name: string) {
        const id = this.sliders[name].id;
        const valSpan =
            document.querySelector("#slider-" + id + " span") as HTMLElement;

        const v = this.getSlider(name);
        this.sliders[name].callback(v)
        valSpan.textContent =
            this.sliders[name].integer ? v.toFixed(0) : v.toFixed(4)
        const def = this.sliders[name].defaults.length > 0
                        ? this.sliders[name].defaults[0]
                        : 0;
        if (v.toFixed(4) === def.toFixed(4))
            document.getElementById("slider-" + id)
                .classList.add("default-value");
        else
            document.getElementById("slider-" + id)
                .classList.remove("default-value");
    }

    public addSlider(name: string, initial: number, min: number, max: number,
                     base: number, callback: ((v: number) => void),
                     defaults: number[], integer: boolean = false): void {
        const id = uid();
        this.sliders[name] = {id, base, callback, defaults, integer};

        $(this.content())
            .append("<div class='slider-con' id='slider-" + id + "'>" +
                    "<div class='slider-title'>" + name + " <span></span>" +
                    defaults
                        .map(x => "<div class='defaults' val='" + x + "'>" + x +
                                  "</div>")
                        .join("") +
                    "</div>" +
                    "<input type='range' min='" + min +
                    "' step='0.00000001'  max='" + max + "' value='" + 0 +
                    "' class='slider'></div>");

        const slider = document.querySelector("#slider-" + id + " input") as
                       HTMLInputElement;
        defaults.forEach(x => {
            (document.querySelector("#slider-" + id + " .defaults[val='" + x + "']") as
             HTMLElement)
                .onclick = () => { this.setSlider(name, x); };
        });
        slider.oninput = () => { this.sliderRefresh(name); };
        this.setSlider(name, initial);
    }

    public setSlider(name: string, value: number) {
        if (!this.sliders[name]) {
            console.error("slider", name, "not found");
            return;
        }
        const slider =
            document.querySelector("#slider-" + this.sliders[name].id +
                                   " input") as HTMLInputElement;
        const iv = Math.sign(value) *
                   Math.pow(Math.abs(value), 1 / this.sliders[name].base)

        slider.value = iv + "";
        this.sliderRefresh(name);
    }

    private buttons: {[name: string]: number} = {};
    public addButton(name: string, onClick: () => void) {
        const id = uid();
        $(this.content())
            .append("<div class='button' id='button-" + id + "'>" + name +
                    "</div>");
        this.buttons[name] = id;
        document.getElementById("button-" + id).onclick = () => { onClick(); }
    }

    public renameButton(oldName: string, newName: string) {
        const id = this.buttons[oldName];
        expect(id).to.be.a("number");
        this.buttons[newName] = id;
        document.getElementById("button-" + id).textContent = newName;
    }

    public destroy() {
        clearInterval(this.refresher);
        this.sliders = null;
        this.j.remove();
    }

    public show(): void {
        // TODO
    }

    private id: number;
    private title: string;
    private j: $.JQuery;
}