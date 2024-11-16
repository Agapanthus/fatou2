import "@mdi/font/css/materialdesignicons.css"; // Ensure you are using css-loader
import Vue from "vue";
import Vuetify from "vuetify/lib";

import de from "vuetify/src/locale/de";
import en from "vuetify/src/locale/en";

Vue.config.productionTip = false; // window.route.debug;
Vue.config.devtools = false; // window.route.debug;

// Vue.directive("observe-visibility", ObserveVisibility);

const opts = {
    lang: {
        locales: { de, en },
        current: "de",
    },
    icons: {
        iconfont: "mdi",
    },
    theme: {
        themes: {
            light: {
                /*
                primary: '#000',
                secondary: '#FFD65E',
                accent: '#EDAFB8',
                error: '#FF5252',
                info: '#2196F3',
                success: '#4CAF50',
                warning: '#FFC107'
                */
            },
        },
    },
};

Vue.use(Vuetify, opts);

// eslint-disable-next-line @typescript-eslint/no-unsafe-argument
export default new Vuetify(opts as any);
