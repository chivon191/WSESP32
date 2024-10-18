#ifndef MYBUZZER_H
#define MYBUZZER_H

class MyBuzzer {
    public:
        const int buzzerPin;
        MyBuzzer(const int pin);
        void success();
        void error();
};

#endif