// https://github.com/davidmartinez10/bigfloat is crap. The float limit is ignored. Multiplying leads to large negative exponents and extremely big integers. Bad!
//import { BigFloat, set_precision } from "bigfloat.js";

/*
export class Complex {
    private re: BigFloat;
    private im: BigFloat;
    constructor(re: BigFloat, im: BigFloat) {
        this.re = re;
        this.im = im;
    }

    public static fromNum(re: number, im: number) {
        return new Complex(new BigFloat(re), new BigFloat(im));
    }

    public static globalPrecision(n: number) {
        set_precision(-n);
    }

    public static add(a: Complex, b: Complex) {
        return a.add(b);
    }

    public copy(): Complex {
        return new Complex(new BigFloat(this.re), new BigFloat(this.im));
    }

    public toString(): string {
        return this.re.toString() + " + " + this.im.toString() + "i";
    }

    square(): Complex {
        // (a+ib)^2 = a^2 + b^2 - 2ab
        return new Complex(
            this.re.mul(this.re).add(this.im.mul(this.im)).sub(this.re.mul(this.im).mul(2)),
            new BigFloat(0)
        );
    }

    getMagSquared(): BigFloat {
        // |(a+ib)|^2 = (a+ib)(a-ib) = a^2 + b^2
        return this.re.mul(this.re).add(this.im.mul(this.im));
    }

    add(c: Complex): Complex {
        return new Complex(this.re.add(c.re), this.im.add(c.im));
    }

    normalize(): Complex {
        return new Complex(this.re.div(1), this.im.div(1));
    }

    mul(c: Complex): Complex {
        // (a+ib)*(c+id) = (ac - bd) + i(ad + bc)
        return new Complex(
            this.re.mul(c.re).sub(this.im.mul(c.im)),
            this.re.mul(c.im).add(this.im.mul(c.re))
        );
    }

    hadaMul(c: Complex): Complex {
        // (a+ib) (*) (c+id)= ac + ibd
        return new Complex(this.re.mul(c.re), this.im.mul(c.im));
    }
}
*/

export class Complex {
    private re: number;
    private im: number;
    constructor(re: number, im: number) {
        this.re = re;
        this.im = im;
    }

    public static fromNum(re: number, im: number) {
        return new Complex(re, im);
    }

    public static globalPrecision(n: number) {
        // set_precision(-n);
    }

    public static add(a: Complex, b: Complex) {
        return a.add(b);
    }

    public copy(): Complex {
        return new Complex(this.re, this.im);
    }

    public toString(): string {
        return this.re.toString() + " + " + this.im.toString() + "i";
    }

    square(): Complex {
        // (a+ib)^2 = a^2 + b^2 - 2ab
        return new Complex(this.re * this.re + this.im * this.im - this.re * this.im * 2, 0);
    }

    getMagSquared(): number {
        // |(a+ib)|^2 = (a+ib)(a-ib) = a^2 + b^2
        return this.re * this.re + this.im * this.im;
    }

    add(c: Complex): Complex {
        return new Complex(this.re + c.re, this.im + c.im);
    }

    mul(c: Complex): Complex {
        // (a+ib)*(c+id) = (ac - bd) + i(ad + bc)
        return new Complex(this.re * c.re - this.im * c.im, this.re * c.im + this.im * c.re);
    }

    hadaMul(c: Complex): Complex {
        // (a+ib) (*) (c+id)= ac + ibd
        return new Complex(this.re * c.re, this.im * c.im);
    }

    x(): number {
        return this.re;
    }

    y(): number {
        return this.im;
    }
}
