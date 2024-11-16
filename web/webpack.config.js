var webpack = require("webpack");

const path = require("path");

const postcssPresetEnv = require("postcss-preset-env");
const BundleAnalyzerPlugin = require("webpack-bundle-analyzer").BundleAnalyzerPlugin;

const babelLoader = {
    loader: "babel-loader",
    options: {
        presets: [
            [
                "@babel/preset-env",
                {
                    modules: false,
                    targets: {
                        chrome: "58",
                        ie: "11",
                    },
                },
            ],
        ],
    },
};

module.exports = (env) => {
    let RELEASE = !(env && env.debug); // Development only! Use Release for Release!
    let SOURCEMAPS = !RELEASE;

    let plugs = [];

    if (RELEASE) {
        plugs.push(new BundleAnalyzerPlugin()); // For analyzing the bundle size
    }

    return {
        performance: {
            maxEntrypointSize: 812000,
            maxAssetSize: 812000,
        },
        entry: "./src/app.ts",
        cache: !RELEASE,
        devtool: RELEASE ? "source-map" : "cheap-source-map", // 'source-map' is much slower but gives better sourcemaps
        mode: RELEASE ? "production" : "development",

        output: {
            filename: "app.js",
            path: path.resolve(__dirname, "./public/js"),
            publicPath: "/js/",
        },
        resolve: {
            extensions: [".webpack.js", ".web.js", ".ts", ".js", ".vue"],
            alias: {
                modernizr$: path.resolve(__dirname, ".modernizrrc"),
                vue$: "vue/dist/vue.esm.js",
            },
        },
        plugins: plugs,
        module: {
            rules: [
                {
                    test: /\.(png|jp(e*)g|svg)$/,
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
                    test: /\.(txt|fs|vs)$/i,
                    use: [
                        {
                            loader: "raw-loader",
                            options: {
                                esModule: false,
                            },
                        },
                    ],
                },
                {
                    test: /\.tsx?$/,
                    use: [
                        {
                            loader: "ts-loader",
                            options: {
                                configFile: "tsconfig.json",
                                appendTsSuffixTo: [/\.vue$/],
                            },
                        },
                    ],
                    exclude: /node_modules/,
                },
                {
                    test: /\.modernizrrc$/,
                    loader: "modernizr-loader!json-loader",
                },
                {
                    test: /\.(s*)[ac]ss$/,
                    use: [
                        {
                            loader: "style-loader",
                            options: {
                                sourceMap: SOURCEMAPS,
                            },
                        },
                        {
                            loader: "postcss-loader",
                            options: {
                                ident: "postcss",
                                sourceMap: SOURCEMAPS,
                                plugins: () => [postcssPresetEnv({ autoprefixer: true })],
                            },
                        },
                        {
                            loader: "sass-loader",
                            options: {
                                sourceMap: SOURCEMAPS,
                                implementation: require("sass"),
                                sassOptions: {
                                    outputStyle: "compressed",
                                    fiber: require("fibers"),
                                },
                            },
                        },
                    ],
                },
                {
                    test: /\.js$/,
                    exclude: /(node_modules|bower_components|vendor)/,
                    use: babelLoader,
                },
                {
                    // Configure resource path here!
                    test: /\.(eot|svg|ttf|woff|woff2)$/,
                    loader: "file-loader?name=public/fonts/[name].[ext]",
                },
            ],
        },
        externals: {
            fs: true,
        },
    };
};
