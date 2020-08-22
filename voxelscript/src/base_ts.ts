type int = number;
type double = number;
type bool = boolean;

declare global {
    interface Array<T> {
        _VS_push(...elem: T[]): number;
    }
}

if (!Array.prototype._VS_push) {
  Array.prototype._VS_push = function<T>(...args: T[]): number {
      return this.push(...args);
  }
}

function applyMixins(derivedCtor: any, constructors: any[]) {
    let parents = [];
    for (let o of constructors) {
        parents.push(Object.getPrototypeOf(o))
    }
    let all_constructors = parents.concat(constructors);
    all_constructors.forEach((baseCtor) => {
        Object.getOwnPropertyNames(baseCtor.prototype).forEach((name) => {
            //console.log(derivedCtor.name + " gets \"" + name + "\" from " + baseCtor.name);
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

let _VS_console = console;
declare global {
    interface Console {
        _VS_log(...elem: any[]): void;
    }
}
console._VS_log = function(...args : any[]) {
    this.log(...args);
};

export {int, double, bool, applyMixins, cast, _VS_console};