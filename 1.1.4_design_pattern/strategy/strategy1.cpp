enum VacationEnum {
    VAC_Spring,
    VAC_Qixi,
    VAC_Wuyi,
    VAC_Guoqing
};

class Promotion {
    VacationEnum vac;
public:
    double CalcPromotion() {
        if (vac == VAC_Spring) {
            // 春节
        } else if (vac == VAC_Qixi) {
            // 七夕
        } else if (vac == VAC_Wuyi) {
            // 五一
        } else if (vac == VAC_Guoqing) {
            // 国庆
        }
        return 0;
    }
};
