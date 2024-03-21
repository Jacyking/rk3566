#ifndef PSYCHROPY_H
#define PSYCHROPY_H

#ifdef __cplusplus
extern "C" {
#endif
double Part_press(double P, double W);
double Sat_press(double Tdb);
double Hum_rat(double Tdb, double Twb, double P);
double Hum_rat2(double Tdb, double RH, double P);
double Rel_hum(double Tdb, double Twb, double P);
double Rel_hum2(double Tdb, double W, double P);
double Wet_bulb(double Tdb, double RH, double P);
double Enthalpy_Air_H2O(double Tdb, double W);
double T_drybulb_calc(double h, double W);
double Dew_point(double P, double W);
double Dry_Air_Density(double P, double Tdb, double W);
double psych(double P, const char *in0Type, double in0Val, const char *in1Type,
             double in1Val, const char *outType, const char *unitType);
#ifdef __cplusplus
}
#endif

#endif // PSYCHROPY_H
