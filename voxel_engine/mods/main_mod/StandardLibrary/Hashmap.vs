import Hashable;
import Operators;

class HashMap<Key: Hash + Eq, Value: any> {
    init();
    Value get(Key k);
    bool is_set(Key k);
    void set(Key k, Value v);
}

implement HashMap<Key, Value> {
    // Set of key/values
    Pair<Key, Value>[][] buckets;
    // Refers to buckets.size()
    int capacity;
    // Refers to # of elements in HashMap
    int size;

    init() {
        this.size = 0;
        this.capacity = 25;
        this.buckets = [];
        this.buckets.resize(25);
        for(int i = 0; i < 25; i++) {
            this.buckets[i] = [];
        }
    }

    Value get(Key k) {
        int hash = k.hash();
        Pair<Key, Value>[] bucket = this.buckets[hash % this.capacity];
        for(int i = 0; i < bucket.size(); i++) {
            if (k.is_equal(bucket[i].first)) {
                return bucket[i].second;
            }
        }
        throw "Key not found!";
    }

    bool is_set(Key k) {
        int hash = k.hash();
        Pair<Key, Value>[] bucket = this.buckets[hash % this.capacity];
        for(int i = 0; i < bucket.size(); i++) {
            if (k.is_equal(bucket[i].first)) {
                return true;
            }
        }
        return false;
    }

    void set(Key k, Value v) {
        int hash = k.hash();
        Pair<Key, Value>[] bucket = this.buckets[hash % this.capacity];
        int found = -1;
        for(int i = 0; i < bucket.size(); i++) {
            if (k.is_equal(bucket[i].first)) {
                // If key is found, Overwrite the value at that location and then return
                bucket[found].second = v;
                return;
            }
        }
        // If key is not found, add to the linked list
        this.size++;
        bucket.push(new Pair<Key, Value>(k, v));
    }

    void unset(Key k) {
        int hash = k.hash();
        Pair<Key, Value>[] bucket = this.buckets[hash % this.capacity];
        int found = -1;
        for(int i = 0; i < bucket.size(); i++) {
            if (k.is_equal(bucket[i].first)) {
                // If found, remove the value at that location and then return
                this.size--;
                bucket.remove(i);
                return;
            }
        }
        // If not found, there's nothing to do
    }
}

export {};
