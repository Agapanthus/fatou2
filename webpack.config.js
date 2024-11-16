module.exports = (env) => {
    console.log("Environment: ", env);

    const path = require("path");
    const { VueLoaderPlugin } = require("vue-loader");
    const VuetifyLoaderPlugin = require("vuetify-loader/lib/plugin");

    //const postcssPresetEnv = require("postcss-preset-env");

    const RELEASE = !(env && env.debug); // Development only! Use Release for Release!
    const RELEASE_SOURCE_MAPS = false; // false or "source-map" (extremely slow)

    let plugs = [new VueLoaderPlugin(), new VuetifyLoaderPlugin()];

    if (RELEASE) {
        //plugs.push(new BundleAnalyzerPlugin()); // For analyzing the bundle size
    } else {
        /*const BrowserSyncPlugin = require("browser-sync-webpack-plugin");

        plugs.push(
            new BrowserSyncPlugin({
                host: "localhost",
                port: 3000,
                proxy: "http://verein-der-altafraner.localhost/",
            })
        );*/
    }

    if (RELEASE) config = "Release";
    else config = "Debug";

    const distPath = path.resolve(__dirname, "./build/" + config + "/js");

    console.log("Writing to:", distPath);

    const excludes = ["node_modules", "legacy", "third_party", "build", "vendor"].map((x) =>
        path.resolve(__dirname, x)
    );
    console.log(excludes);

    return {
        performance: {
            maxEntrypointSize: 8120000,
            maxAssetSize: 8120000,
        },
        entry: {
           /* cpp: {
                import: "./src/app.ts",
                filename: "./build/"+config +"/js/app.js",
            },*/
            web: {
                import: "./src/webapp.ts",
                filename: "./public/js/app.js",
            },
        },
        cache: !RELEASE,
        devtool: RELEASE ? RELEASE_SOURCE_MAPS : "eval-cheap-module-source-map",
        mode: RELEASE ? "production" : "development",

        output: {
            //filename: "app.js",
            //chunkFilename: "[name].[chunkhash].bundle.js",
            path: __dirname,
            //  publicPath: "/js/",
        },
        resolve: {
            extensions: [
                ".webpack.js",
                ".web.js",
                ".ts",
                ".js",
                ".vue",
                ".json",
                ".png",
                ".jpg",
                ".scss",
            ],
            alias: {
                vue$: "vue/dist/vue.esm.js",
                //  "@": path.join(__dirname, "..", "resource")
            },
        },
        plugins: plugs,
        module: {
            rules: [
                {
                    test: /\.(png|jp(e*)g|svg)$/,
                    exclude: excludes,
                    use: [
                        {
                            loader: "url-loader",
                            options: {
                                limit: 8000, // Convert images < 8kb to base64 strings
                                name: "images/[hash]-[name].[ext]",
                            },
                        },
                    ],
                },
                {
                    test: /\.tsx?$/,
                    exclude: excludes,
                    use: {
                        loader: "ts-loader",
                        options: {
                            configFile: "tsconfig.json",
                            appendTsSuffixTo: [/\.vue$/],
                        },
                    },
                },

                {
                    test: /\.(s*)[ac]ss$/i,
                    use: [
                        // Creates `style` nodes from JS strings
                        "style-loader",
                        // Translates CSS into CommonJS
                        { loader: "css-loader", options: { url: false } },

                        // Compiles Sass to CSS
                        "sass-loader",
                    ],
                },
                {
                    test: /\.(eot|svg|ttf|woff|woff2)$/,
                    exclude: excludes,
                    loader: "file-loader",
                    options: {
                        name: "public/fonts/[name].[ext]",
                    },
                },
                {
                    test: /\.(md|txt)$/i,
                    exclude: excludes,
                    use: "raw-loader",
                },

                {
                    test: /\.(glsl|vs|fs|vert|frag)$/,
                    exclude: excludes,
                    use: ["raw-loader", "glslify-loader"],
                },
                {
                    test: /\.vue$/,
                    use: "vue-loader",
                },
            ],
        },
        externals: {
            fs: true,
        },
    };
};
