/* eslint-disable no-bitwise */
import axios, { Method } from "axios";
import { expect } from "chai";

declare global {
    function reportError(e: unknown, payload: unknown): void;
}

window.reportError = (e: unknown, payload: unknown = null): void => {
    // Only submit an error report once! (to avoid too many of them)

    // eslint-disable-next-line no-console
    console.error(e, payload);
};

export function getRandomSubarray(arr: any[], size: number) {
    var shuffled = arr.slice(0),
        i = arr.length,
        temp,
        index;
    while (i--) {
        index = Math.floor((i + 1) * Math.random());
        temp = shuffled[index];
        shuffled[index] = shuffled[i];
        shuffled[i] = temp;
    }
    return shuffled.slice(0, size);
}

export function isEmpty(obj: Record<string, unknown>): boolean {
    for (const key in obj) {
        if (obj.hasOwnProperty(key)) return false;
    }
    return true;
}

export function defined(variable: unknown): boolean {
    return !(typeof variable === "undefined");
}

// https://stackoverflow.com/a/9436948/6144727
export function isString(myVar: unknown): myVar is string {
    return typeof myVar === "string" || myVar instanceof String;
}

export function wh2query(s: { width: number; height: number }, scaleBy: number = 1): string {
    return "?w=" + s.width * scaleBy + "&h=" + s.height * scaleBy;
}

/**
 * Fits a rect inside a rect such that it is fully visible
 *
 * @param w
 * @param h
 * @param maxWidth
 * @param maxHeight
 * @returns width and height
 */
export function fitInside(
    w: number,
    h: number,
    maxWidth: number,
    maxHeight: number
): { width: number; height: number } {
    if (h > maxHeight) {
        w = w * (maxHeight / h);
        h = maxHeight;
    }

    if (w > maxWidth) {
        h = h * (maxWidth / w);
        w = maxWidth;
    }

    return { width: Math.round(w), height: Math.round(h) };
}

// https://stackoverflow.com/a/26230989/6144727
export function getCoords(elem: HTMLElement): { top: number; left: number } {
    // crossbrowser version
    const box = elem.getBoundingClientRect();

    const body = document.body;
    const docEl = document.documentElement;

    const scrollTop = window.pageYOffset || docEl.scrollTop || body.scrollTop;
    const scrollLeft = window.pageXOffset || docEl.scrollLeft || body.scrollLeft;

    const clientTop = docEl.clientTop || body.clientTop || 0;
    const clientLeft = docEl.clientLeft || body.clientLeft || 0;

    const top = box.top + scrollTop - clientTop;
    const left = box.left + scrollLeft - clientLeft;

    return { top: Math.round(top), left: Math.round(left) };
}

/** might produce weird results like "[object object] but will try its best and always produce a string */
export function force2string(e: unknown): string {
    if (isString(e)) return e;

    try {
        return JSON.stringify(e);
    } catch (_) {}

    // eslint-disable-next-line @typescript-eslint/restrict-plus-operands, @typescript-eslint/no-base-to-string
    return e + "";
}

export function scrollIntoView(
    hash: string,
    event: Event | null = null,
    smooth: boolean = true
): void {
    // On-page links only
    if (hash === "" || hash === "0") return;

    let target = document.getElementById(hash);
    if (!target) {
        const t = document.getElementsByName(hash);
        if (t.length > 0) target = t[0];
    }

    if (target) {
        if (event) event.preventDefault();
        // https://stackoverflow.com/a/43704679/6144727
        target.scrollIntoView({
            behavior: smooth ? "smooth" : "auto",
            block: "start",
        });
    }
}

// https://stackoverflow.com/a/201378/6144727
export function validateEmail(s: string): boolean {
    return !!(
        s &&
        /(?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*|"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\[(?:(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9]))\.){3}(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9])|[a-z0-9-]*[a-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\])/.exec(
            s
        )
    );
}

export type FormValue = Date | File | FileList | boolean | number | string | null;

function normalizeValue(v: FormValue): Blob | string {
    if (v instanceof FileList) return v[0];
    if (v instanceof File) return v;
    if (v === true) return "1";
    if (v === false) return "0";
    return force2string(v);
}

export function toFormData(rawData: unknown): FormData {
    let data: FormData | null = null;

    if (rawData instanceof FormData) {
        data = rawData;
    } else if (Array.isArray(rawData)) {
        data = new FormData();
        for (const prop of rawData) {
            // eslint-disable-next-line @typescript-eslint/no-unsafe-argument, @typescript-eslint/no-unsafe-member-access
            data.append(prop.name, normalizeValue(prop.value));
        }
    } else if (typeof rawData === "object" && rawData !== null) {
        data = new FormData();
        for (const [key, value] of Object.entries(rawData))
            data.append(key, normalizeValue(value as FormValue));
    } else throw new Error("invalid rawData");
    return data;
}

export async function getFileWithDialog(accept: string | null = null): Promise<File> {
    let reject: (() => void) | null = null;
    return new Promise<File>((res, reject2) => {
        reject = reject2;
        const input = document.createElement("input");
        input.type = "file";
        if (accept) input.accept = accept;
        input.multiple = false;

        let fileChanged: (() => void) | null = null;
        const focusHandler = (): void => {
            if (fileChanged) fileChanged();
        };

        // reject delayed, because the change event fires after some time and we want to resolve if we can
        const rej = (): void => {
            //document.body.removeEventListener("focus", focusHandler);
            document.body.onfocus = null;

            setTimeout(() => {
                // when rejecting, make sure that we can't resolve later
                if (reject) {
                    reject();
                }
                reject = null;
            }, 1000);
        };

        // either the dialog lost focus or a file was chosen
        fileChanged = (): void => {
            let files: File[] = [];
            if (input.files) files = Array.from(input.files);

            if (files.length > 0) {
                // only resolve if not yet rejected
                if (reject) {
                    //document.body.removeEventListener("focus", focusHandler);
                    document.body.onfocus = null;
                    res(files[0]);
                    reject = null;
                }
            } else rej();
        };

        // can't detect cancel directly. https://stackoverflow.com/a/15738200/6144727
        // but can direct when the dialog looses focus
        // but it only works with onfocus, not with addEventListener
        // document.body.addEventListener("focus",  focusHandler);
        document.body.onfocus = focusHandler;

        input.onchange = fileChanged;
        input.onerror = rej;
        input.onabort = rej;
        input.click();
    });
}

export async function apiAction(
    url: string,
    payload: FormData | Record<string, Date | number | string> | null = null,
    method: Method = "POST"
): Promise<any> {
    // for the debug setup with browserSync Proxy:
    if (url.startsWith("http://verein-der-altafraner.localhost")) {
        url = url.substr("http://verein-der-altafraner.localhost".length);
    }

    /**
     * TODO:
     * If you cannot use API routes which is the recommended method you will need to add code to your axios error handler to do a new GET request to obtain a valid CSRF token then retry your original request. I believe axios interceptors are what your looking for
     */
    return new Promise((res, rej) => {
        axios
            .request({
                url,
                method,
                data: payload,
                // eslint-disable-next-line @typescript-eslint/naming-convention
                headers: { "Cache-Control": "no-cache" },
                // withCredentials: true (already default)
            })
            .then(
                (resp) => {
                    res(resp.data);
                },
                (error) => {
                    if (axios.isAxiosError(error)) {
                        if (!error.response) {
                            rej("unknown");
                            return;
                        }

                        // eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
                        const resp: {
                            data?: {
                                message?: string;
                            };
                            status: number;
                        } = error.response as any;

                        let msg = resp.data?.message ?? "";
                        if (msg) {
                            msg = "\nDetails: " + msg;
                        }

                        if (!isEmpty(resp.data as Record<string, unknown>)) {
                            if (msg) {
                                if (msg === "CSRF token mismatch.") {
                                    rej("Sitzung abgelaufen. Bitte lade die Seite neu!");
                                    return;
                                } else if (resp.status === 422) {
                                    rej(resp.data);
                                    return;
                                }
                            }
                            if (resp.status === 422) {
                                rej(resp.data);
                                return;
                            }
                        }

                        if (resp.status === 404) {
                            rej(
                                "Dieses Feature oder die verwendete Ressource existieren nicht." +
                                    msg
                            );
                            return;
                        } else if (resp.status === 429) {
                            rej(
                                "Du wurdest wegen zu vieler Anfragen vorübergehend blockiert. Bitte versuche es in ein paar Minuten erneut."
                            );
                            return;
                        } else if (resp.status === 500) {
                            rej("Unerwarteter Serverseitiger Fehler" + msg);
                            return;
                        } else if (resp.status === 401) {
                            rej(
                                "Du bist nicht angemeldet oder es fehlt eine zusätzliche Authorisierung." +
                                    msg
                            );
                            return;
                        } else if (resp.status === 403) {
                            rej("Das ist dir nicht erlaubt." + msg);
                            return;
                        }

                        reportError("Unerwarteter Serverseitiger Fehler", {
                            status: resp.status,
                            resp: JSON.stringify(resp.data),
                        });

                        rej(
                            "Unerwarteter Fehler!\nCode: " + resp.status + msg
                            // (msg || "\nDetails: " + JSON.stringify(resp.data))
                        );
                        return;
                    }
                    rej("Unbekannter Client-seitiger Axios-Fehler");
                    return;
                }
            );
    });
}

const bluredContext = {};

export function blur(ctx: string): void {
    bluredContext[ctx] = true;
    if (Object.keys(bluredContext).length > 0) {
        document.querySelectorAll(".blurable").forEach((el) => {
            el.classList.add("blured");
        });
    }

    document.body.classList.add("hideFadeables");
}

export function unblur(ctx: string): void {
    // eslint-disable-next-line @typescript-eslint/no-dynamic-delete
    delete bluredContext[ctx];
    if (Object.keys(bluredContext).length <= 0) {
        document.querySelectorAll(".blurable").forEach((el) => {
            el.classList.remove("blured");
        });
    }
}

export function tryStubborn(interval: number, timeout: number, f: () => boolean): boolean | null {
    if (!f()) {
        if (timeout <= 0) return false;
        setTimeout(() => {
            tryStubborn(interval, timeout - interval, f);
        });
        return null;
    } else return true;
}

// https://stackoverflow.com/a/11598864/6144727
export function removeNonPrintable(str: string): string {
    const re =
        /[\0-\x1F\x7F-\x9F\xAD\u0378\u0379\u037F-\u0383\u038B\u038D\u03A2\u0528-\u0530\u0557\u0558\u0560\u0588\u058B-\u058E\u0590\u05C8-\u05CF\u05EB-\u05EF\u05F5-\u0605\u061C\u061D\u06DD\u070E\u070F\u074B\u074C\u07B2-\u07BF\u07FB-\u07FF\u082E\u082F\u083F\u085C\u085D\u085F-\u089F\u08A1\u08AD-\u08E3\u08FF\u0978\u0980\u0984\u098D\u098E\u0991\u0992\u09A9\u09B1\u09B3-\u09B5\u09BA\u09BB\u09C5\u09C6\u09C9\u09CA\u09CF-\u09D6\u09D8-\u09DB\u09DE\u09E4\u09E5\u09FC-\u0A00\u0A04\u0A0B-\u0A0E\u0A11\u0A12\u0A29\u0A31\u0A34\u0A37\u0A3A\u0A3B\u0A3D\u0A43-\u0A46\u0A49\u0A4A\u0A4E-\u0A50\u0A52-\u0A58\u0A5D\u0A5F-\u0A65\u0A76-\u0A80\u0A84\u0A8E\u0A92\u0AA9\u0AB1\u0AB4\u0ABA\u0ABB\u0AC6\u0ACA\u0ACE\u0ACF\u0AD1-\u0ADF\u0AE4\u0AE5\u0AF2-\u0B00\u0B04\u0B0D\u0B0E\u0B11\u0B12\u0B29\u0B31\u0B34\u0B3A\u0B3B\u0B45\u0B46\u0B49\u0B4A\u0B4E-\u0B55\u0B58-\u0B5B\u0B5E\u0B64\u0B65\u0B78-\u0B81\u0B84\u0B8B-\u0B8D\u0B91\u0B96-\u0B98\u0B9B\u0B9D\u0BA0-\u0BA2\u0BA5-\u0BA7\u0BAB-\u0BAD\u0BBA-\u0BBD\u0BC3-\u0BC5\u0BC9\u0BCE\u0BCF\u0BD1-\u0BD6\u0BD8-\u0BE5\u0BFB-\u0C00\u0C04\u0C0D\u0C11\u0C29\u0C34\u0C3A-\u0C3C\u0C45\u0C49\u0C4E-\u0C54\u0C57\u0C5A-\u0C5F\u0C64\u0C65\u0C70-\u0C77\u0C80\u0C81\u0C84\u0C8D\u0C91\u0CA9\u0CB4\u0CBA\u0CBB\u0CC5\u0CC9\u0CCE-\u0CD4\u0CD7-\u0CDD\u0CDF\u0CE4\u0CE5\u0CF0\u0CF3-\u0D01\u0D04\u0D0D\u0D11\u0D3B\u0D3C\u0D45\u0D49\u0D4F-\u0D56\u0D58-\u0D5F\u0D64\u0D65\u0D76-\u0D78\u0D80\u0D81\u0D84\u0D97-\u0D99\u0DB2\u0DBC\u0DBE\u0DBF\u0DC7-\u0DC9\u0DCB-\u0DCE\u0DD5\u0DD7\u0DE0-\u0DF1\u0DF5-\u0E00\u0E3B-\u0E3E\u0E5C-\u0E80\u0E83\u0E85\u0E86\u0E89\u0E8B\u0E8C\u0E8E-\u0E93\u0E98\u0EA0\u0EA4\u0EA6\u0EA8\u0EA9\u0EAC\u0EBA\u0EBE\u0EBF\u0EC5\u0EC7\u0ECE\u0ECF\u0EDA\u0EDB\u0EE0-\u0EFF\u0F48\u0F6D-\u0F70\u0F98\u0FBD\u0FCD\u0FDB-\u0FFF\u10C6\u10C8-\u10CC\u10CE\u10CF\u1249\u124E\u124F\u1257\u1259\u125E\u125F\u1289\u128E\u128F\u12B1\u12B6\u12B7\u12BF\u12C1\u12C6\u12C7\u12D7\u1311\u1316\u1317\u135B\u135C\u137D-\u137F\u139A-\u139F\u13F5-\u13FF\u169D-\u169F\u16F1-\u16FF\u170D\u1715-\u171F\u1737-\u173F\u1754-\u175F\u176D\u1771\u1774-\u177F\u17DE\u17DF\u17EA-\u17EF\u17FA-\u17FF\u180F\u181A-\u181F\u1878-\u187F\u18AB-\u18AF\u18F6-\u18FF\u191D-\u191F\u192C-\u192F\u193C-\u193F\u1941-\u1943\u196E\u196F\u1975-\u197F\u19AC-\u19AF\u19CA-\u19CF\u19DB-\u19DD\u1A1C\u1A1D\u1A5F\u1A7D\u1A7E\u1A8A-\u1A8F\u1A9A-\u1A9F\u1AAE-\u1AFF\u1B4C-\u1B4F\u1B7D-\u1B7F\u1BF4-\u1BFB\u1C38-\u1C3A\u1C4A-\u1C4C\u1C80-\u1CBF\u1CC8-\u1CCF\u1CF7-\u1CFF\u1DE7-\u1DFB\u1F16\u1F17\u1F1E\u1F1F\u1F46\u1F47\u1F4E\u1F4F\u1F58\u1F5A\u1F5C\u1F5E\u1F7E\u1F7F\u1FB5\u1FC5\u1FD4\u1FD5\u1FDC\u1FF0\u1FF1\u1FF5\u1FFF\u200B-\u200F\u202A-\u202E\u2060-\u206F\u2072\u2073\u208F\u209D-\u209F\u20BB-\u20CF\u20F1-\u20FF\u218A-\u218F\u23F4-\u23FF\u2427-\u243F\u244B-\u245F\u2700\u2B4D-\u2B4F\u2B5A-\u2BFF\u2C2F\u2C5F\u2CF4-\u2CF8\u2D26\u2D28-\u2D2C\u2D2E\u2D2F\u2D68-\u2D6E\u2D71-\u2D7E\u2D97-\u2D9F\u2DA7\u2DAF\u2DB7\u2DBF\u2DC7\u2DCF\u2DD7\u2DDF\u2E3C-\u2E7F\u2E9A\u2EF4-\u2EFF\u2FD6-\u2FEF\u2FFC-\u2FFF\u3040\u3097\u3098\u3100-\u3104\u312E-\u3130\u318F\u31BB-\u31BF\u31E4-\u31EF\u321F\u32FF\u4DB6-\u4DBF\u9FCD-\u9FFF\uA48D-\uA48F\uA4C7-\uA4CF\uA62C-\uA63F\uA698-\uA69E\uA6F8-\uA6FF\uA78F\uA794-\uA79F\uA7AB-\uA7F7\uA82C-\uA82F\uA83A-\uA83F\uA878-\uA87F\uA8C5-\uA8CD\uA8DA-\uA8DF\uA8FC-\uA8FF\uA954-\uA95E\uA97D-\uA97F\uA9CE\uA9DA-\uA9DD\uA9E0-\uA9FF\uAA37-\uAA3F\uAA4E\uAA4F\uAA5A\uAA5B\uAA7C-\uAA7F\uAAC3-\uAADA\uAAF7-\uAB00\uAB07\uAB08\uAB0F\uAB10\uAB17-\uAB1F\uAB27\uAB2F-\uABBF\uABEE\uABEF\uABFA-\uABFF\uD7A4-\uD7AF\uD7C7-\uD7CA\uD7FC-\uF8FF\uFA6E\uFA6F\uFADA-\uFAFF\uFB07-\uFB12\uFB18-\uFB1C\uFB37\uFB3D\uFB3F\uFB42\uFB45\uFBC2-\uFBD2\uFD40-\uFD4F\uFD90\uFD91\uFDC8-\uFDEF\uFDFE\uFDFF\uFE1A-\uFE1F\uFE27-\uFE2F\uFE53\uFE67\uFE6C-\uFE6F\uFE75\uFEFD-\uFF00\uFFBF-\uFFC1\uFFC8\uFFC9\uFFD0\uFFD1\uFFD8\uFFD9\uFFDD-\uFFDF\uFFE7\uFFEF-\uFFFB\uFFFE\uFFFF]/g;
    return str.replace(re, "");
}

// https://stackoverflow.com/a/1677660/6144727
export function isAsciiPrintableString(str: string): boolean {
    return !/[\x00-\x1F\x80-\xFF]/.test(str);
}

export function repeat<T>(val: T, n: number): T[] {
    const result: T[] = [];
    for (let i = 0; i < n; i++) result.push(val);
    return result;
}

export const getAbsoluteUrl = ((): ((url: string) => {
    href: string;
    hostname: string;
    pathname: string;
}) => {
    let a: HTMLAnchorElement | null = null;
    return (
        url: string
    ): {
        href: string;
        hostname: string;
        pathname: string;
    } => {
        a = a ?? document.createElement("a");
        a.href = url;
        return { href: a.href, hostname: a.hostname, pathname: a.pathname };
    };
})();

// https://stackoverflow.com/a/6969486/6144727
export function escapeRegExp(str: string): string {
    return str.replace(/[\-\[\]\/\{\}\(\)\*\+\?\.\\\^\$\|]/g, "\\$&");
}

export function reduce<T, B>(
    array: T[],
    callback: (acc: B, val: T, i?: number, arr?: T[]) => B,
    initialVal?: B
): B | undefined {
    let accumulator = initialVal;
    const context: B | undefined = undefined;
    for (let i = 0; i < array.length; i++) {
        if (accumulator !== undefined)
            // eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
            accumulator = callback.call(context, accumulator, array[i], i, array);
        else {
            //if(typeof accumulator !== typeof array[i]) console.error("incompatible types: " + typeof accumulator + " " + typeof array[i]);
            // eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
            accumulator = array[i] as any;
        }
    }
    return accumulator;
}

// https://stackoverflow.com/a/7616484
export function strHash(str: string): number {
    let hash = 0;
    for (let i = 0; i < str.length; i++) {
        const chr = str.charCodeAt(i);
        hash = (hash << 5) - hash + chr;
        hash |= 0; // Convert to 32bit integer
    }
    return hash;
}

declare global {
    //const _paq: any[];

    interface Window {
        uidCounter: number | undefined;
    }
}

export function makeUniqueID(): string {
    // Change state uniquely
    if (!defined(window.uidCounter)) {
        window.uidCounter = Math.random();
    }
    if (window.uidCounter) window.uidCounter++;
    else throw new Error("impossible");

    // Generate ID
    let v = strHash("" + window.uidCounter) as number | string;
    if (v < 0) {
        v = "A" + -v;
    }

    return "uid_" + v;
}

// https://awik.io/determine-color-bright-dark-using-javascript/
export function isBright(color: string, thresh = 127.5): boolean {
    let r = 0;
    let g = 0;
    let b = 0;
    let hsp = 0;
    if (/^rgb/.exec(color)) {
        const color2 = /^rgba?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*(\d+(?:\.\d+)?))?\)$/.exec(color);
        if (!color2) return false;
        r = parseInt(color2[1], 10);
        g = parseInt(color2[2], 10);
        b = parseInt(color2[3], 10);
    } else {
        const color2 = +(
            "0x" + color.slice(1).replace(color.length < 5 ? /./g : /dontmatch/, "$&$&")
        );
        r = color2 >> 16;
        g = (color2 >> 8) & 255;
        b = color2 & 255;
    }

    // HSP (Highly Sensitive Poo) equation from http://alienryderflex.com/hsp.html
    hsp = Math.sqrt(0.299 * (r * r) + 0.587 * (g * g) + 0.114 * (b * b));

    // Using the HSP value, determine whether the color is light or dark
    return hsp > thresh;
}
export function getBright(dominant: string): string {
    const arr = dominant.split("#");
    for (const a of arr) if (isBright(a)) return "#" + a;
    return "transparent";
}

// https://stackoverflow.com/a/3809435/6144727
export const regexUrl =
    "https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%_\\+.~#?&//=]*)";

// Returns true iff the arrays intersect
export function testIntersect<T>(a: T[], b: T[]): boolean {
    for (const c of a) {
        if (b.includes(c)) return true;
    }
    return false;
}

// Returns true iff the arrays intersect
export function intersect<T>(a: T[], b: T[]): T[] {
    const intersection = [] as T[];
    for (const c of a) {
        if (b.includes(c)) {
            if (!intersection.includes(c)) {
                intersection.push(c);
            }
        }
    }
    return intersection;
}

// https://stackoverflow.com/a/37511463
export function removeAllAccents(str: string): string {
    return str.normalize("NFD").replace(/[\u0300-\u036f]/g, "");
}

export function formatBytes(bytes: number, decimals = 2): string {
    if (bytes === 0) return "0 Bytes";

    const k = 1024;
    const dm = decimals < 0 ? 0 : decimals;
    const sizes = ["Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"];

    const i = Math.floor(Math.log(bytes) / Math.log(k));

    return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + " " + sizes[i];
}

export function updateURLParameter(url: string, param: string, value: string): string {
    const u = new URL(url);
    if (value) u.searchParams.set(param, value);
    else u.searchParams.delete(param);
    return u.toString();
}

export function setSiteParam(key: string, value: string): void {
    window.history.replaceState("", "", updateURLParameter(window.location.href, key, value));
}

export function setSiteHash(value: string | null): void {
    const u = new URL(window.location.href);
    if (value) u.hash = value;
    else u.hash = "";
    window.history.replaceState("", "", u.toString());
}

export function getSiteHash(): string | null {
    const u = window.location.hash;
    if (!u) return null;
    if (!u.startsWith("#")) reportError("hash didn't start with #", u);
    return u.substr(1);
}

// https://stackoverflow.com/a/901144/6144727
export function getSiteParam(key: string, url: string | null = null): string | null {
    if (!url) url = window.location.href;
    key = key.replace(/[\[\]]/g, "\\$&");
    const regex = new RegExp("[?&]" + key + "(=([^&#]*)|&|#|$)");
    const results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return "";
    return decodeURIComponent(results[2].replace(/\+/g, " "));
}

// https://stackoverflow.com/a/14810722/6144727
export function objectMap<T, S>(
    object: Record<string, T>,
    mapFn: (x: T, key?: string) => S
): Record<string, S> {
    return Object.keys(object).reduce((result: Record<string, S>, key: string) => {
        result[key] = mapFn(object[key], key);
        return result;
    }, {});
}

export function wordCount(str: string): number {
    return (str.match(/\s/g) ?? []).length + 1;
}

export function removePunctuation(str: string): string {
    return str.replace(/[.,\/#!$%\^&\*;:{}=\-_`~()\s]/g, "");
}

// https://stackoverflow.com/a/30850912/6144727
export function argMax(a: number[]): number {
    return a.reduce((iMax, x, i, arr) => (x > arr[iMax] ? i : iMax), 0);
}

export function randi(min: number, max: number): number {
    min = Math.ceil(min);
    max = Math.floor(max);
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

export function abbreviate(str: string, MIN_LEN: number = 6): string {
    const oStr = str.trim();

    // Original string
    if (str.length <= MIN_LEN) return str;

    const uppedWords = oStr
        .split(/[\s,\.\-\+\&\\\/]+/)
        .filter((x) => x.length > 0)
        .map((x) => x[0].toUpperCase() + x.substring(1));
    str = uppedWords.join("");
    // No space
    if (str.length <= MIN_LEN) return str;

    const noVowelWords = uppedWords.map((x) => x.replace(/[aeiouyäöü]/g, ""));
    str = noVowelWords.join("");
    // No vocals
    if (str.length <= MIN_LEN) return str;

    const initials = noVowelWords.map((x) => x[0]);

    str = initials.slice(0, -1).join("") + uppedWords.pop();
    // initials, last without space
    if (str.length <= MIN_LEN) return str;

    str = initials.slice(0, -1).join("") + noVowelWords.pop();
    // initials, last without vocals
    if (str.length <= MIN_LEN) return str;

    // Initials only
    if (initials.length > MIN_LEN / 2) return initials.join("").substring(0, MIN_LEN);

    // Truncate first as initial, the words
    return str.substring(0, MIN_LEN);
}

export function shortText(str: string, length: number = 20): string {
    str = str.trim();
    if (str.length > length) {
        str = str.slice(0, length);
        str = str.slice(0, str.lastIndexOf(" "));
        str += "…";
    }
    return str;
}

export function fromEntries<T>(arr: [number | string, T][]): Record<string, T> {
    return Array.from(arr).reduce((acc, [key, val]) => Object.assign(acc, { [key]: val }), {});
}

// based on https://stackoverflow.com/a/15030117/6144727
export function flatten<T>(arr: T[][]): T[] {
    return arr.reduce((flat, toFlatten) => {
        return flat.concat(toFlatten);
    }, []);
}

export function compare<T>(a: T, b: T): -1 | 0 | 1 {
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

export function unique<T>(a: T[]): T[] {
    return Array.from(new Set(a));
}

export function mergeUnique<T>(a: T[], b: T[], c: T[] = [], d: T[] = []): T[] {
    return unique([...a, ...b, ...c, ...d]);
}

export function deepCopy<T>(a: T): T {
    expect(defined(a)).to.eq(true, "deepCopy got undefined");
    return JSON.parse(JSON.stringify(a)) as T;
}

interface KuchenNodeTemp {
    doc?: KuchenNodeTemp[];
    content?: KuchenNodeTemp[];
    type?: string;
    text?: string;
}

export function kuchenExtractText(nodes: KuchenNodeTemp | KuchenNodeTemp[] | null): string {
    if (!nodes) return "";
    if (Array.isArray(nodes)) {
        let extracted = "";
        for (const node of nodes) {
            if (node.type === "text") extracted += node.text;
            else if (node.content) extracted += kuchenExtractText(node.content);
        }
        return extracted;
    }
    if (nodes.doc) return kuchenExtractText(nodes.doc);
    if (nodes.content) return kuchenExtractText(nodes.content);
    return "";
}

export function downloadCSV(filename: string, text: string): void {
    const element = document.createElement("a");
    element.setAttribute("href", "data:text/csv;charset=utf-8," + encodeURIComponent(text));
    element.setAttribute("download", filename);

    element.style.display = "none";
    document.body.appendChild(element);

    element.click();

    document.body.removeChild(element);
}

export function formAsArray(form: HTMLFormElement): { name: string; value: FormValue }[] {
    const query: {
        name: string;
        value: FormValue;
    }[] = [];
    for (const el of form.elements as any) {
        if (el instanceof HTMLInputElement) {
            if (["text", "email", "password", "number", "date", "hidden"].includes(el.type))
                query.push({ name: el.name, value: el.value });
            else if (["file"].includes(el.type)) query.push({ name: el.name, value: el.files });
            else if (["radio"].includes(el.type)) {
                if (el.checked) query.push({ name: el.name, value: el.value });
            } else if (["checkbox"].includes(el.type))
                query.push({ name: el.name, value: el.checked ? 1 : 0 });
            else reportError("is an unknown input element", el);
        } else if (el instanceof HTMLSelectElement) {
            query.push({ name: el.name, value: el.value });
        } else if (el instanceof HTMLButtonElement) {
            // drop
        } else {
            reportError("is an unknown input element", el);
        }
    }
    return query;
}

// https://stackoverflow.com/a/34064434/6144727
export function htmlDecode(input: string): string | null {
    const doc = new DOMParser().parseFromString(input, "text/html");
    return doc.documentElement.textContent;
}
