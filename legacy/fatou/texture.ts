import * as f from "./global";

export class Texture {
    // Based on
    // https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/Tutorial/Using_textures_in_WebGL
    // either url or array
    constructor(
        data: string | Uint8Array,
        finished: (success: boolean) => void,
        gl: WebGLRenderingContext,
        width: number = 0,
        height: number = 0
    ) {
        this.texture = gl.createTexture();
        this.gl = gl;

        if (typeof data === "string") {
            gl.bindTexture(gl.TEXTURE_2D, this.texture);

            const level = 0;
            const internalFormat = gl.RGBA;
            const width = 1;
            const height = 1;
            const border = 0;
            const srcFormat = gl.RGBA;
            const srcType = gl.UNSIGNED_BYTE;

            // create preview texture
            const pixel = new Uint8Array([0, 0, 255, 255]); // opaque blue
            gl.texImage2D(
                gl.TEXTURE_2D,
                level,
                internalFormat,
                width,
                height,
                border,
                srcFormat,
                srcType,
                pixel
            );

            // now load the real image
            const image = new Image();
            image.onload = () => {
                gl.bindTexture(gl.TEXTURE_2D, this.texture);
                gl.texImage2D(gl.TEXTURE_2D, level, internalFormat, srcFormat, srcType, image);

                this.finishCreateTexture(image.width, image.height);
                try {
                    f.glthrow(gl, "Texture.constructor");
                    finished(true);
                } catch (e) {
                    finished(false);
                }
            };
            image.onerror = () => {
                finished(false);
            };
            image.src = data;
        } else {
            const type = this.gl.RGBA;
            this.gl.bindTexture(this.gl.TEXTURE_2D, this.texture);
            this.gl.texImage2D(
                this.gl.TEXTURE_2D,
                0,
                type,
                width,
                height,
                0,
                type,
                this.gl.UNSIGNED_BYTE,
                data
            );
            this.finishCreateTexture(width, height);
            // Other texture setup here, like filter modes and mipmap generation
            finished(true);
        }
    }

    private finishCreateTexture(width: number, height: number) {
        const gl = this.gl;
        const makeMIPMAPS = false;
        if (makeMIPMAPS) {
            if (Texture.isPowerOf2(width) && Texture.isPowerOf2(height)) {
                // Yes, it's a power of 2. Generate mips.
                gl.generateMipmap(gl.TEXTURE_2D);
            } else {
                throw new f.Exception("Invalid texture size", "Texture.constructor.makeMIPMAPS");
            }
        } else {
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        }
    }

    public destroy() {
        this.gl.deleteTexture(this.texture);
    }

    public static isPowerOf2(value: number): boolean {
        return (value & (value - 1)) === 0;
    }

    public bind(texture = this.gl.TEXTURE0): void {
        this.gl.activeTexture(texture);
        this.gl.bindTexture(this.gl.TEXTURE_2D, this.texture);
    }

    private gl: WebGLRenderingContext;
    private texture: WebGLTexture;
}
