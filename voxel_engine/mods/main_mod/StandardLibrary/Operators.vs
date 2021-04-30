trait Eq {
    bool is_equal(Eq e);
}
class Pair<U: any, V: any> {
    U first;
    V second;
    init(U first, V second);
}
implement Pair<U, V> {
    init(U first, V second) {
        this.first = first;
        this.second = second;
    }
}

export {Eq};
