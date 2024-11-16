import * as f from "./global";

export class Texture {

    // Based on
    // https://developer.mozilla.org/en-US/docs/Web/API/WebGL_API/Tutorial/Using_textures_in_WebGL
    constructor(url: string, finished: (success: boolean) => void,
                gl: WebGLRenderingContext) {
        this.texture = gl.createTexture();
        this.gl = gl;
        gl.bindTexture(gl.TEXTURE_2D, this.texture);

        const level = 0;
        const internalFormat = gl.RGBA;
        const width = 1;
        const height = 1;
        const border = 0;
        const srcFormat = gl.RGBA;
        const srcType = gl.UNSIGNED_BYTE;
        const pixel = new Uint8Array([ 0, 0, 255, 255 ]); // opaque blue
        gl.texImage2D(gl.TEXTURE_2D, level, internalFormat, width, height,
                      border, srcFormat, srcType, pixel);
        const image = new Image();
        image.onload = () => {
            gl.bindTexture(gl.TEXTURE_2D, this.texture);
            gl.texImage2D(gl.TEXTURE_2D, level, internalFormat, srcFormat,
                          srcType, image);

            const makeMIPMAPS = false;
            if (makeMIPMAPS) {
                if (Texture.isPowerOf2(image.width) &&
                    Texture.isPowerOf2(image.height)) {
                    // Yes, it's a power of 2. Generate mips.
                    gl.generateMipmap(gl.TEXTURE_2D);
                } else {
                    finished(false);
                    throw new f.Exception("Invalid texture size",
                                          "Texture.constructor.makeMIPMAPS");
                }
            } else {
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S,
                                 gl.CLAMP_TO_EDGE);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T,
                                 gl.CLAMP_TO_EDGE);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER,
                                 gl.LINEAR);
            }
            try {
                f.glthrow(gl, "Texture.constructor");
                finished(true);
            } catch (e) {
                finished(false);
            }
        };
        image.onerror = () => { finished(false); };
        image.src = url;
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