// @ts-ignore
import { Color } from "../color";
import { Complex } from "../complex";
import mandelbrot_smooth from "../shaders/mandelbrot/smooth.fs";
import {
    EasyFractal,
    Parameter,
    ParameterNumeric,
    Uniform4f,
    UniformParameters,
} from "./easyFractal";

export class Mandelbrot extends EasyFractal {
    constructor() {
        super(mandelbrot_smooth, true);
    }

    easyCreate(): Promise<void> {
        return new Promise((res) => {
            this.loadPreset("Smooth");
            res();
        });
    }

    private smoothing: number;
    private radius: number;
    private phase: number;
    private shift: number;
    private contrast: number;
    private iGamma: number;
    private maxIter: number = -1;
    public updateCache() {
        // This is EXTREMELY slow
        // but it should be updated from time to time
        this.smoothing = this.getParamValue("smoothing");
        this.radius = this.getParamValue("radius");
        this.phase = this.getParamValue("phase");
        this.contrast = this.getParamValue("contrast");
        this.maxIter = this.getParamValue("iterations");
        this.shift = this.getParamValue("shift");
        this.iGamma = this.getParamValue("iGamma");
    }
    private escapeRadius(pos: Complex, zoom: Complex, lPos: Complex): number {
        const maxIter = this.maxIter;

        const tc = lPos.add(Complex.fromNum(-0.5, -0.5));
        const z = tc.hadaMul(zoom).add(pos);

        //let p = Complex.fromNum(0, 0);
        let Zr = 0;
        let Zi = 0;
        let Tr = 0;
        let Ti = 0;
        const Ci = z.y();
        const Cr = z.x();

        const c: number = this.radius;
        const cx2: number = c * c;
        for (let j = 0; j <= maxIter; j++) {
            //p = p.square().add(z);
            Zi = 2 * Zr * Zi + Ci;
            Zr = Tr - Ti + Cr;
            Tr = Zr * Zr;
            Ti = Zi * Zi;

            // approximate magnitude.
            // const mgs = p.getMagSquared()
            const mgs = Tr + Ti;
            if (mgs >= cx2) {
                // Smoothing
                const log_zn: number = Math.log(mgs) * 0.5;
                const nu: number =
                    Math.log(log_zn * 1.44269504088896) * 1.44269504088896 * this.smoothing;
                const v: number = (j + 1 - nu) * 0.02; // + zx.x * 10.0;

                return v;
            }
        }
        return maxIter + 1;
    }

    private makeColors(v: number): Color {
        // based on https://krazydad.com/tutorials/makecolors.php
        v = this.contrast * v + this.phase;
        //return pow(sin(vec3(v, v + 1 * this.getParamValue("shift"), v + 2 * this.getParamValue("shift"))) * 0.5 + 0.5, this.getParamValue("iGamma"));

        return new Color(
            Math.pow(Math.sin(v) * 0.5 + 0.5, this.iGamma) * 255,
            Math.pow(Math.sin(v + 1 * this.shift) * 0.5 + 0.5, this.iGamma) * 255,
            Math.pow(Math.sin(v + 2 * this.shift) * 0.5 + 0.5, this.iGamma) * 255
        );
    }

    public sample(pos: Complex, zoom: Complex, lPos: Complex): Color {
        if (this.maxIter < 0) this.updateCache();

        const v = this.escapeRadius(pos, zoom, lPos);

        const c = v >= this.maxIter - 1 ? new Color(0, 0, 0) : this.makeColors(v);
        c.a = 255;
        return c;
    }

    public getParameters(): { [name: string]: Parameter } {
        return {
            radius: new ParameterNumeric("Escape Radius", 0, 4, [4]),
            smoothing: new ParameterNumeric("Smoothing", -4, 4, [1, 0], 3),
            iterations: new ParameterNumeric("Iterations", 0, 30, [1000], 3, true),

            contrast: new ParameterNumeric("Contrast", 0, 6, [3, 0], 2),
            phase: new ParameterNumeric("Phase", 0, 2 * Math.PI, [0]),
            shift: new ParameterNumeric("Shift", 0, (2 / 3) * Math.PI, [1]),
            iGamma: new ParameterNumeric("Inverse Gamma", 0, 1, [0.7]),

            play: new ParameterNumeric("play", -1, 1, [0]),
        };
    }

    public getUniforms(): UniformParameters[] {
        return [
            new Uniform4f("c", "radius", "play", "smoothing", "iterations"),
            new Uniform4f("c2", "contrast", "phase", "shift", "iGamma"),
        ];
    }

    destroy() {
        this.prog.destroy();
        super.destroy();
    }

    public getName() {
        return "Mandelbrot";
    }

    public static presets = {
        zoomDeeps: {
            // https://www.youtube.com/watch?v=LhOSM6uCWxk&t=2552s
            zoom: Math.pow(2, -37),
            x: -1.9997740601362903593126807559602500475710416233856384007148508574291012335984591928248364190215796259575718318799960175396106897245889581254834492701372949636783094955897931317174101259095891469501748126725148714587333938548443819033709904187344921523413310221887295870857771431011674873342592895504186325482220668710775749899926429101099841583206278295793058921625817004481783699245865364627140554117737774937789463895102748671351750212506004241754983473339789940659968568850689353099462034492524909310777724611601104714214019347435268544619054369865904944457792527241696528695821059623303046651934176389789308453627525109367436309636375268231073110318555064708363221007235298404379856922536028913291478442839193381367508575286692330907891402483843152933153748354825108021776358693600801782904774626935265722056455978643513448489091026679036353407968495795003386248005939867069799946547181378474054113117046900560609110812439442002663909295191705374444149326937073460052706389967886211172676612720028299452788285465688867116337489531157494508508315428488520037968118008255840569742557333862639124341116894229885253643651920014148109308402199399127712572209466874971603743536096235390414412927589954662603878558182262865151900604451937214289079939337905846647369517138325441736853526711818853134657265043099539402286244220638999824999819000131999789999857999958,
            y: -0.0000000032900403214794350534969786759266805967852946505878410088326046927853549452991056352681196631150325234171525664335353457621247922992470898021063583060218954321140472066153878996044171428801408137278072521468882260382336298800961530905692393992277070012433445706657829475924367459793505729004118759963065667029896464160298608486277109065108339157276150465318584383757554775431988245033409975361804443001325241206485033571912765723551757793318752425925728969073157628495924710926832527350298951594826689051400340011140584507852761857568007670527511272585460136585523090533629795012272916453744029579624949223464015705500594059847850617137983380334184205468184810116554041390142120676993959768153409797953194054452153167317775439590270326683890021272963306430827680201998682699627962109145863135950941097962048870017412568065614566213639455841624790306469846132055305041523313740204187090956921716703959797752042569621665723251356946610646735381744551743865516477084313729738832141633286400726001116308041460406558452004662264165125100793429491308397667995852591271957435535504083325331161340230101590756539955554407081416407239097101967362512942992702550533040602039494984081681370518238283847808934080198642728761205332894028474812918370467949299531287492728394399650466260849557177609714181271299409118059191938687461,
            radius: 4,
            iterations: 1000,
            superSamples: 2,
        },
        Smooth: {
            zoom: 3.3,
            x: -0.6,
            y: 0,
            radius: 4,
            iterations: 1000,
            superSamples: 2,
        },
        Tortoise: {
            zoom: Math.pow(2, -35.3),
            x: -1.76890224278382635,
            y: -0.00249310263813761,
            iterations: 2000,
        },
        Inf: {
            // TODO: Use quad precision! Zoom to center! This is a fascinating series!
            zoom: Math.pow(2, -32),
            x: -0.743643887037158704752191506114774,
            y: 0.131825904205311970493132056385139,
            iterations: 5000,
            contrast: 0.23,
        },

        Corals: {
            zoom: Math.pow(2, -22),
            x: -0.69586585925114464,
            y: 0.44646844758065846,
            iterations: 800,
        },
        "Black Sun": {
            zoom: Math.pow(2, -22.6),
            x: -0.6958659321672108,
            y: 0.4464684050481072,
            iterations: 6000,
        },

        Zebra: {
            zoom: 0.00000000041556109,
            x: -1.47926642477697179,
            y: 0.01049892980285544,
            iterations: 3000,
        },
        Sputnik: {
            zoom: 0.00000003537777001,
            x: -1.94158036006772883,
            y: 0.00084079117678854,
            iterations: 2000,
        },
        "Sputnik-Star": {
            zoom: Math.pow(2, -22.8),
            x: -1.94158036006772883,
            y: 0.00084079117678854,
            iterations: 2000,
            contrast: 36,
            smoothing: 0,
        },
        "Sputnik-Flower": {
            zoom: Math.pow(2, -22.8),
            x: -1.94158036006772883,
            y: 0.00084079117678854,
            iterations: 2000,
            contrast: 4.2,
            smoothing: -10.2,
        },

        "Sea Shells": {
            zoom: 0.00000030811596324,
            x: 0.25248304937365174,
            y: -0.0001844395141528,
            smoothing: 3,
            contrast: 3.96,
            iterations: 980,
            shift: 0.7,
        },
        Kraken: {
            zoom: 0.00000000262600591,
            x: -0.52337883341641245,
            y: 0.68058363785114295,
            iterations: 10000,
        },

        "Zebra-Snail": {
            zoom: 0.00000001253239359,
            x: -0.53655782061638835,
            y: 0.66622670863609945,
            iterations: 12000,
        },

        filigran: {
            zoom: Math.pow(2, -3.422),
            x: -0.40968,
            y: 0.59177,
            iterations: 96,
            iGamma: 0,
        },

        filigran2: {
            zoom: Math.pow(2, -3.422),
            x: -0.40968,
            y: 0.59177,
            iterations: 96,
            iGamma: 0.12,
            play: -0.3686,
        },

        filigran3: {
            zoom: Math.pow(2, -9.474),
            x: -0.643091,
            y: 0.4299041,
            iterations: 1171,
            iGamma: 0.06,
            play: -0.2158,
            shift: 1,
            contrast: 36,
        },
        Domination: {
            zoom: Math.pow(2, -27.295),
            x: 0.2524801648402278,
            y: -0.0001847804869756,
            iterations: 7000,
            // TODO: Finde eine Instanz, wo die untere Spirale einen
            // geringeren Radius hat!
        },

        Contraposition: {
            zoom: 0.0003249702384,
            x: 0.2600006026390369,
            y: -0.00160370450573859,
        },

        Drops: {
            zoom: Math.pow(2, -14.646),
            x: 0.2514953857859145,
            y: -0.000093649473209,
            iterations: 245,
        },

        Balls: {
            zoom: 0.00225727670266614,
            x: -0.72229992560474776,
            y: 0.19226090615766106,
            radius: 0.95,
            iterations: 1000,
            smoothing: 2.0,
        },

        "smooth lines": {
            zoom: Math.pow(2, -12.649),
            x: 0.256107052,
            y: -0.00094147,
            radius: 4,
            iterations: 2000,
            smoothing: 0,
            contrast: 16.262,
            play: 0.0082,
        },

        "geometric curves": {
            zoom: Math.pow(2, -11.104),
            x: 0.2640092,
            y: -0.00284293,
            radius: 4,
            iterations: 2000,
            smoothing: -0.5,
            contrast: 16.262,
            play: 0.0082,
        },
        "geometric gold": {
            zoom: Math.pow(2, -11.104),
            x: 0.2640092,
            y: -0.00284293,
            radius: 4,
            iterations: 283,
            smoothing: -41,
            contrast: 0.6069,
            iGamma: 0.527,
            play: 0.0082,
            phase: 5.4,
        },
    };
}
