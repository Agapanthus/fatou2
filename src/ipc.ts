export {};

declare global {
    interface Window {
        app: any;
        browserReady: () => string;
        loadPreset: (x: string) => string;
    }
}

window.app = window.app || {};

/*
window.browserReady =
    window.browserReady ||
    (() => {
        return "";
    });
*/
