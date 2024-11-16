

export enum CharacterState {
    left,
    right,
    idle,
}

// This controller can not do much
export class Controller {
    state: CharacterState // state the character should be in
    faceLeft: boolean // character should be facing left
    posX: number

    constructor() {
        this.state = CharacterState.idle
        this.faceLeft = false
        this.posX = 100
    }

    update(deltaTime, walkSpeed) {
        if (this.state == CharacterState.left) {
            this.posX -= deltaTime * walkSpeed
        } else if (this.state == CharacterState.right) {
            this.posX += deltaTime * walkSpeed
        }
    }
}

// Abstracts input methods to commands the character can perform
export class UserController extends Controller {
    constructor(onUp: () => void, onDown: () => void, onSpace: () => void) {
        super()

        let left = false
        let right = false
        let fired = {}
        document.addEventListener("keydown", (e) => {
            if (!fired[e.keyCode]) {
                fired[e.keyCode] = true;
            } else {
                return
            }

            if (e.keyCode == 38) {
                // up arrow
                onUp()
            }
            else if (e.keyCode == 40) {
                // down arrow
                onDown()
            }
            else if (e.keyCode == 37) {
                // left arrow
                left = true
                this.faceLeft = true
                this.state = CharacterState.left
            }
            else if (e.keyCode == 39) {
                // right arrow
                right = true
                this.faceLeft = false
                this.state = CharacterState.right
            } else if (e.keyCode == 32) {
                // SPACE
                onSpace()
            }

            return true
        })

        document.onkeyup = (e) => {
            fired[e.keyCode] = false;

            if (e.keyCode == 38) {
                // up arrow
            }
            else if (e.keyCode == 40) {
                // down arrow
            }
            else if (e.keyCode == 37) {
                // left arrow
                left = false
            }
            else if (e.keyCode == 39) {
                // right arrow
                right = false
            }

            if (left == false && right == false) {
                this.state = CharacterState.idle
            }
        }

    }
}
