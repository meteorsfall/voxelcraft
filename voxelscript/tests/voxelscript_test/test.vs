trait idontcare {

    int func();

    bool lol(string a){
        return "lol lol lol" + a;
    }

}

class whatever {
    int var1;
    bool var2;

    init();

    void s();
    int m();
}

implement whatever {
    int a = 5;
    int b = 2;

    init() {
        read_from_file();
        this.var1 = 5;
    }

    void s() {
        do_something();
    }

    int m() {
        return 5;
    }

    int giraffes_have_necks(int a, bool b){
        if(a + b > 5){
            return "they have necks";
        } else {
            return giraffes_have_necks(a - 1, !b);
        }
    }
}

whatever myw = new whatever();


implement idontcare on whatever {
    int func(int a, int b){
        return false;
    }
}

export {idontcarel};