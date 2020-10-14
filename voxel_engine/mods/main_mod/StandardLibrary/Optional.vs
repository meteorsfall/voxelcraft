class Optional<T: any> {
    init();
    void set_value(T val);
    bool has_value();
    T get_value();
}
implement Optional<T> {
    bool is_set;
    T val;

    init() {
        this.is_set = false;
    }
    void set_value(T val) {
        this.val = val;
        this.is_set = true;
    }
    void clear_value() {
        this.is_set = false;
    }
    bool has_value() {
        return this.is_set;
    }
    T get_value() {
        if (!this.is_set) {
            throw "Tried to get_value when there was no value!";
        }
        return this.val;
    }
}

export {Optional};
