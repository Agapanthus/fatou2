import { expect } from "chai";
import Vue from "vue";
import Vuex from "vuex";

Vue.use(Vuex);

// TODO
export interface PopupInfo {}

export interface EditorInfo {
    lastSaved: string;
    users: any;
}

export interface RenderParams {
    currentProgress?: number;
    finishedLayer?: number;
    width?: number;
    height?: number;
    targetEffort?: number;
    renderTime?: number;
    fps?: number;
}

export interface State {
    popupInfo: PopupInfo | null;
    popupResolve: ((x: string) => void) | null;
    popupReject: ((x?: string) => void) | null;

    editorInfo: EditorInfo | Record<string, never>;

    currentMonitor: number;
    monitors: any[];
    devices: any[];

    x: string;
    y: string;
    z: string;

    renderParams?: RenderParams;
}

export const store = new Vuex.Store<State>({
    state: {
        popupInfo: null,
        popupResolve: null,
        popupReject: null,

        editorInfo: {},

        currentMonitor: 0,
        monitors: [],
        devices: [],

        x: "0",
        y: "0",
        z: "1",

        renderParams: undefined,
    },
    getters: {
        popupInfo: (state): PopupInfo | null => {
            return state.popupInfo;
        },
        getEditorInfo: (state) => {
            return state.editorInfo;
        },

        getCurrentMonitor: (state) => {
            return state.currentMonitor;
        },
        getMonitors: (state) => {
            return state.monitors;
        },
        getDevices: (state) => {
            return state.devices;
        },

        getX: (state) => {
            return state.x;
        },
        getY: (state) => {
            return state.y;
        },
        getZ: (state) => {
            return state.z;
        },
        getRenderParams: (state) => {
            return state.renderParams;
        },
    },

    mutations: {
        setPopupInfo: (state, value: PopupInfo) => {
            expect(state.popupInfo).to.eq(null);
            expect(state.popupResolve).to.eq(null);
            state.popupInfo = value;
        },
        setEditorInfo: (state, value: EditorInfo) => {
            state.editorInfo = value;
        },
        setPopupResolve: (state, { res, rej }: { res: (x: string) => void; rej: () => void }) => {
            state.popupResolve = res;
            state.popupReject = rej;
        },
        resolvePopup(state, data: string | null = null) {
            if (data) {
                if (state.popupResolve) state.popupResolve(data);
            } else {
                if (state.popupReject) state.popupReject();
            }
        },
        clearPopup(state) {
            state.popupInfo = null;
            state.popupResolve = null;
            state.popupReject = null;
        },

        setCurrentMonitor: (state, value: number) => {
            state.currentMonitor = value;
        },
        appendMonitor: (state, value: any) => {
            state.monitors.push(value);
        },
        appendDevice: (state, value: any) => {
            state.devices.push(value);
        },
        setX: (state, x: any) => {
            state.x = x;
        },
        setY: (state, y: any) => {
            state.y = y;
        },
        setZ: (state, z: any) => {
            state.z = z;
        },
        setRenderParams: (state, params: any) => {
            if (!state.renderParams) state.renderParams = {};
            for (let k in params) {
                Vue.set(state.renderParams, k, params[k]);
            }
        },
    },

    actions: {
        async openPopup({ dispatch, commit }, popupInfo: PopupInfo): Promise<string> {
            return new Promise((res, rej) => {
                commit("setPopupInfo", popupInfo);
                commit("setPopupResolve", { res, rej });
            });
        },
        resolvePopup({ dispatch, commit }, data: string | null = null) {
            commit("resolvePopup", data);
            commit("clearPopup");
        },
    },
});
