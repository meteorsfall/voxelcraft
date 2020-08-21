type int = number;
type double = number;
type bool = boolean;

function applyMixins(derivedCtor: any, constructors: any[]) {
    let parents = [];
    for (let o of constructors) {
        parents.push(Object.getPrototypeOf(o))
    }
    let all_constructors = parents.concat(constructors);
    all_constructors.forEach((baseCtor) => {
        Object.getOwnPropertyNames(baseCtor.prototype).forEach((name) => {
            console.log(derivedCtor.name + " gets \"" + name + "\" from " + baseCtor.name);
            Object.defineProperty(
                derivedCtor.prototype,
                name,
                Object.getOwnPropertyDescriptor(baseCtor.prototype, name)!
            );
        });
    });
}

function cast<T>(arg : any, verify : null | string) : T {
    if (verify != null) {
        if (!arg[verify]) {
            throw new Error("Failed Type verification!");
        }
    }
    return (<T> <unknown> arg);
}

export {int, double, bool, applyMixins, cast};