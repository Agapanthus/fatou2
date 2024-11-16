
let scriptblocker = false;
const sp = document.getElementById("err-js");
if (sp === undefined)
    scriptblocker = true;
try {
    sp.parentNode.removeChild(sp);
} catch (err) {
    scriptblocker = true;
}
Element.prototype.remove = function() { this.parentElement.removeChild(this); };
const createCriticalPopupInner = (id, title, innerHTML, delayClass) => {
    document.body.innerHTML +=
        '<div class="' + delayClass + ' fullpopup" id="' + id +
        '"><div class="h"><div class="centered">' + title +
        '</div></div><div class="centered">' + innerHTML + '</div></div>';
};

/* tslint:disable */
export function enableJS(): void {
    if (false === scriptblocker) {
        window["removeCriticalPopup"]("javascript_popup");
    }
}
window["removeCriticalPopup"] =
    (id) => { document.getElementById(id).remove(); };
window["createCriticalPopup"] = (id, title, innerHTML) => {
    createCriticalPopupInner(id, title, innerHTML, "showdelayed");
};
window["createCriticalPopup0"] =
    (id, title,
     innerHTML) => { createCriticalPopupInner(id, title, innerHTML, ""); };
window["createCriticalPopup15"] = (id, title, innerHTML) => {
    createCriticalPopupInner(id, title, innerHTML, "showdelayed15");
};
/* tslint:enable */