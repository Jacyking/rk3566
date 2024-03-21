#include <math.h>
#include <stdio.h>
#include <string.h>
#include "psychropy.h"

double Part_press(double P, double W) {
    /* Function to compute partial vapor pressure in [kPa]
        From page 6.9 equation 38 in ASHRAE Fundamentals handbook (2005)
            P = ambient pressure [kPa]
            W = humidity ratio [kg/kg dry air]
    */
    double result = P * W / (0.62198 + W);
    return result;
}

double Sat_press(double Tdb) {
    /* Function to compute saturation vapor pressure in [kPa]
        ASHRAE Fundamentals handbood (2005) p 6.2, equation 5 and 6
            Tdb = Dry bulb temperature [degC]
            Valid from -100C to 200 C
    */
    double C1 = -5674.5359;
    double C2 = 6.3925247;
    double C3 = -0.009677843;
    double C4 = 0.00000062215701;
    double C5 = 2.0747825E-09;
    double C6 = -9.484024E-13;
    double C7 = 4.1635019;
    double C8 = -5800.2206;
    double C9 = 1.3914993;
    double C10 = -0.048640239;
    double C11 = 0.000041764768;
    double C12 = -0.000000014452093;
    double C13 = 6.5459673;
    double TK = Tdb + 273.15;  // Converts from degC to degK
    double result = 0;
    if (TK <= 273.15) {
        result = exp(C1 / TK + C2 + C3 * TK + C4 * pow(TK, 2) + C5 * pow(TK, 3) + C6 * pow(TK, 4) +
                     C7 * log(TK)) /
                 1000;
    } else {
        result =
                exp(C8 / TK + C9 + C10 * TK + C11 * pow(TK, 2) + C12 * pow(TK, 3) + C13 * log(TK)) /
                1000;
    }
    return result;
}

double Hum_rat(double Tdb, double Twb, double P) {
    /* Function to calculate humidity ratio [kg H2O/kg air]
        Given dry bulb and wet bulb temp inputs [degC]
        ASHRAE Fundamentals handbood (2005)
            Tdb = Dry bulb temperature [degC]
            Twb = Wet bulb temperature [degC]
            P = Ambient Pressure [kPa]
    */
    double Pws = Sat_press(Twb);
    double Ws = 0.62198 * Pws / (P - Pws);  // Equation 23, p6.8
    double result = 0;
    if (Tdb >= 0) {  // Equation 35, p6.9
        result = (((2501 - 2.326 * Twb) * Ws - 1.006 * (Tdb - Twb)) /
                  (2501 + 1.86 * Tdb - 4.186 * Twb));
    } else {  // Equation 37, p6.9
        result = (((2830 - 0.24 * Twb) * Ws - 1.006 * (Tdb - Twb)) /
                  (2830 + 1.86 * Tdb - 2.1 * Twb));
    }
    return result;
}

double Hum_rat2(double Tdb, double RH, double P) {
    /* Function to calculate humidity ratio [kg H2O/kg air]
        Given dry bulb and wet bulb temperature inputs [degC]
        ASHRAE Fundamentals handbood (2005)
            Tdb = Dry bulb temperature [degC]
            RH = Relative Humidity [Fraction or %]
            P = Ambient Pressure [kPa]
    */
    double Pws = Sat_press(Tdb);
    double result = 0.62198 * RH * Pws / (P - RH * Pws);  // Equation 22, 24, p6.8
    return result;
}

double Rel_hum(double Tdb, double Twb, double P) {
    /* Calculates relative humidity ratio
        ASHRAE Fundamentals handbood (2005)
            Tdb = Dry bulb temperature [degC]
            Twb = Wet bulb temperature [degC]
            P = Ambient Pressure [kPa]
    */
    double W = Hum_rat(Tdb, Twb, P);
    double result = Part_press(P, W) / Sat_press(Tdb);  // Equation 24, p6.8
    return result;
}

double Rel_hum2(double Tdb, double W, double P) {
    /* Calculates the relative humidity given:
            Tdb = Dry bulb temperature [degC]
            P = ambient pressure [kPa]
            W = humidity ratio [kg/kg dry air]
    */
    double Pw = Part_press(P, W);
    double Pws = Sat_press(Tdb);
    double result = Pw / Pws;
    return result;
}

double Wet_bulb(double Tdb, double RH, double P) {
    /* Calculates the Wet Bulb temp given:
            Tdb = Dry bulb temperature [degC]
            RH = Relative humidity ratio [Fraction or %]
            P = Ambient Pressure [kPa]
        Uses Newton-Rhapson iteration to converge quickly
    */
    double W_normal = Hum_rat2(Tdb, RH, P);
    double result = Tdb;
    // Solves to within 0.001% accuracy using Newton-Rhapson
    double W_new = Hum_rat(Tdb, result, P);
    while (fabs((W_new - W_normal) / W_normal) > 0.00001) {
        double W_new2 = Hum_rat(Tdb, result - 0.001, P);
        double dw_dtwb = (W_new - W_new2) / 0.001;
        result = result - (W_new - W_normal) / dw_dtwb;
        W_new = Hum_rat(Tdb, result, P);
    }
    return result;
}

double Enthalpy_Air_H2O(double Tdb, double W) {
    /* Calculates enthalpy in kJ/kg (dry air) given:
            Tdb = Dry bulb temperature [degC]
            W = Humidity Ratio [kg/kg dry air]
        Calculations from 2005 ASHRAE Handbook - Fundamentals - SI P6.9 eqn 32
    */
    double result = 1.006 * Tdb + W * (2501 + 1.86 * Tdb);
    return result;
}

double T_drybulb_calc(double h, double W) {
    /* Calculates dry bulb Temp in deg C given:
            h = enthalpy [kJ/kg K]
            W = Humidity Ratio [kg/kg dry air]
        back calculated from enthalpy equation above
        ***Warning 0 state for Imp is ~0F, 0% RH ,and  1 ATM, 0 state
              for SI is 0C, 0%RH and 1 ATM
    */
    double result = (h - (2501 * W)) / (1.006 + (1.86 * W));
    return result;
}

double Dew_point(double P, double W) {
    /* Function to compute the dew point temperature (deg C)
        From page 6.9 equation 39 and 40 in ASHRAE Fundamentals handbook (2005)
            P = ambient pressure [kPa]
            W = humidity ratio [kg/kg dry air]
        Valid for Dew Points less than 93 C
    */
    double C14 = 6.54;
    double C15 = 14.526;
    double C16 = 0.7389;
    double C17 = 0.09486;
    double C18 = 0.4569;

    double Pw = Part_press(P, W);
    double alpha = log(Pw);
    double Tdp1 =
            C14 + C15 * alpha + C16 * pow(alpha, 2) + C17 * pow(alpha, 3) + C18 * pow(Pw, 0.1984);
    double Tdp2 = 6.09 + 12.608 * alpha + 0.4959 * pow(alpha, 2);
    double result = 0;
    if (Tdp1 >= 0) {
        result = Tdp1;
    } else {
        result = Tdp2;
    }
    return result;
}

double Dry_Air_Density(double P, double Tdb, double W) {
    /* Function to compute the dry air density (kg_dry_air/m**3), using pressure
        [kPa], temperature [C] and humidity ratio
        From page 6.8 equation 28 ASHRAE Fundamentals handbook (2005)
        [rho_dry_air] = Dry_Air_Density(P, Tdb, w)
        Note that total density of air-h2o mixture is:
        rho_air_h2o = rho_dry_air * (1 + W)
        gas constant for dry air
    */
    double R_da = 287.055;
    double result = 1000 * P / (R_da * (273.15 + Tdb) * (1 + 1.6078 * W));
    return result;
}

double psych(double P,
             const char* in0Type,
             double in0Val,
             const char* in1Type,
             double in1Val,
             const char* outType,
             const char* unitType) {
    double outVal = 0;
    if (strcmp(in0Type, "h") != 0 && strcmp(in0Type, "W") != 0 && strcmp(in0Type, "Tdb") != 0) {
        outVal = NAN;
    } else if (strcmp(in0Type, in1Type) == 0) {
        outVal = NAN;
    }

    double Tdb = 0, W = 0, h = 0;
    double Twb = 0, Dew = 0, RH = 0;
    if (strcmp(unitType, "SI") == 0) {
        P = P / 1000;                // converts P to kPa
        if (strcmp(in0Type, "Tdb") == 0) {  // assign the first input
            Tdb = in0Val;
        } else if (strcmp(in0Type, "W") == 0) {
            W = in0Val;
        } else if (strcmp(in0Type, "h") == 0) {
            h = in0Val;
        }

        if (strcmp(in1Type, "Tdb") == 0) {  // assign the second input
            Tdb = in1Val;
        } else if (strcmp(in1Type, "Twb") == 0) {
            Twb = in1Val;
        } else if (strcmp(in1Type, "DP") == 0) {
            Dew = in1Val;
        } else if (strcmp(in1Type, "RH") == 0) {
            RH = in1Val;
        } else if (strcmp(in1Type, "W") == 0) {
            W = in1Val;
        } else if (strcmp(in1Type, "h") == 0) {
            h = in1Val;
        }
    } else {  // converts to SI if not already
        P = (P * 4.4482216152605) / (pow(0.0254, 2) * 1000);
        if (strcmp(in0Type, "Tdb") == 0) {
            Tdb = (in0Val - 32) / 1.8;
        } else if (strcmp(in0Type, "W") == 0) {
            W = in0Val;
        } else if (strcmp(in0Type, "h") == 0) {
            h = ((in0Val * 1.055056) / 0.45359237) - 17.884444444;
        }

        if (strcmp(in1Type, "Tdb") == 0) {
            Tdb = (in1Val - 32) / 1.8;
        } else if (strcmp(in1Type, "Twb") == 0) {
            Twb = (in1Val - 32) / 1.8;
        } else if (strcmp(in1Type, "DP") == 0) {
            Dew = (in1Val - 32) / 1.8;
        } else if (strcmp(in1Type, "RH") == 0) {
            RH = in1Val;
        } else if (strcmp(in1Type, "W") == 0) {
            W = in1Val;
        } else if (strcmp(in1Type, "h") == 0) {
            h = ((in1Val * 1.055056) / 0.45359237) - 17.884444444;
        }
    }

    if (strcmp(in0Type, "h") == 0 && strcmp(in1Type, "W") == 0) {  // calculate Tdb if not given
        Tdb = T_drybulb_calc(h, W);
    } else if (strcmp(in0Type, "W") == 0 && strcmp(in1Type, "h") == 0) {
        Tdb = T_drybulb_calc(h, W);
    }

    if (strcmp(outType, "RH") == 0 || strcmp(outType, "Twb") == 0) {  // Find RH
        if (strcmp(in1Type, "Twb") == 0) {                            // given Twb
            RH = Rel_hum(Tdb, Twb, P);
        } else if (strcmp(in1Type, "DP") == 0) {  // given Dew
            RH = Sat_press(Dew) / Sat_press(Tdb);
        } else if (strcmp(in1Type, "W") == 0) {  // given W
            RH = Part_press(P, W) / Sat_press(Tdb);
        } else if (strcmp(in1Type, "h") == 0) {
            W = (1.006 * Tdb - h) / (-(2501 + 1.86 * Tdb));
            RH = Part_press(P, W) / Sat_press(Tdb);
        }
        // else if (in1Type == "RH") {  // given RH
        // } RH already Set
    } else {
        if (strcmp(in0Type, "W") != 0) {        // Find W
            if (strcmp(in1Type, "Twb") == 0) {  // Given Twb
                W = Hum_rat(Tdb, Twb, P);
            } else if (strcmp(in1Type, "DP") == 0) {  // Given Dew
                W = 0.621945 * Sat_press(Dew) / (P - Sat_press(Dew));
            }
            // Equation taken from eq 20 of 2009 Fundemental chapter 1
            else if (strcmp(in1Type, "RH") == 0) {  // Given RH
                W = Hum_rat2(Tdb, RH, P);
            } else if (strcmp(in1Type, "h") == 0) {  // Given h
                W = (1.006 * Tdb - h) / (-(2501 + 1.86 * Tdb));
                // Algebra from 2005 ASHRAE Handbook - Fundamentals - SI P6.9 eqn 32
            }
            // else if (in1Type == "W") {  // Given W
            // } W already known
        } else {
            printf("invalid input varilables\n");
        }
    }

    // P, Tdb, and W are now availible

    if (strcmp(outType, "Tdb") == 0) {
        outVal = Tdb;
    } else if (strcmp(outType, "Twb") == 0) {  // Request Twb
        outVal = Wet_bulb(Tdb, RH, P);
    } else if (strcmp(outType, "DP") == 0) {  // Request Dew
        outVal = Dew_point(P, W);
    } else if (strcmp(outType, "RH") == 0) {  // Request RH
        outVal = RH;
    } else if (strcmp(outType, "W") == 0) {  // Request W
        outVal = W;
    } else if (strcmp(outType, "WVP") == 0) {  // Request Pw
        outVal = Part_press(P, W) * 1000;
    } else if (strcmp(outType, "DSat") == 0) {  // Request deg of sat
        outVal = W / Hum_rat2(Tdb, 1, P);
    }
    // the middle arg of Hum_rat2 is suppose to be RH.  RH is suppose to be 100%
    else if (strcmp(outType, "h") == 0) {  // Request enthalpy
        outVal = Enthalpy_Air_H2O(Tdb, W);
    } else if (strcmp(outType, "s") == 0) {  // Request entropy
        outVal = NAN;
    }
    // don\'t have equation for Entropy, so I divided by zero to induce an error.
    else if (strcmp(outType, "SV") == 0) {  // Request specific volume
        outVal = 1 / (Dry_Air_Density(P, Tdb, W));
    } else if (strcmp(outType, "MAD") == 0) {  // Request density
        outVal = Dry_Air_Density(P, Tdb, W) * (1 + W);
    }

    if (strcmp(unitType, "Imp")) {  // Convert to Imperial units
        if (strcmp(outType, "Tdb") == 0 || strcmp(outType, "Twb") == 0 ||
            strcmp(outType, "DP") == 0) {
            outVal = 1.8 * outVal + 32;
        } else if (strcmp(outType, "WVP") == 0) {
            outVal = outVal * pow(0.0254, 2) / 4.448230531;
        } else if (strcmp(outType, "h") == 0) {
            outVal = (outVal + 17.88444444444) * 0.45359237 / 1.055056;
        } else if (strcmp(outType, "SV") == 0) {
            outVal = outVal * 0.45359265 / (pow(12 * 0.0254, 3));
        } else if (strcmp(outType, "MAD") == 0) {
            outVal = outVal * pow(12 * 0.0254, 3) / 0.45359265;
        }
    }
    return outVal;
}
