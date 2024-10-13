class myBuzzer {
    public:
        const int buzzerPin;
        myBuzzer(const int pin);
        void success();
        void error();
};