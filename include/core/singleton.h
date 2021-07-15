#pragma once

template<class T>
class Singleton {
protected:
    Singleton() {}

private:
    Singleton(const Singleton &s);
    Singleton &operator=(Singleton &s);

public:
    virtual ~Singleton() {}
    static T *getInstance() {
        static T _inst;
        return &_inst;
    }
};
