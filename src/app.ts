import { finishLoading } from "./util/global";

import Vue from "vue";
import vuetify from "./vuetify"; // path to vuetify export

import { store } from "./storage/storage";
import App from "./gui/main.vue";

import "./ipc";

window.onload = () => {
    window.app.store = store;

    const v = new Vue({
        el: "#GUIBase",
        render: (h) => h(App),
        vuetify,
        store: store,
    });

    console.log("loading done!");

    if (window.browserReady) window.browserReady();

    finishLoading();
};
