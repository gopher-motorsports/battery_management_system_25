/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "soc.h"
#include "lookupTable.h"
#include "math.h"


/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define TABLE_LENGTH 101


/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

// State of charge for VTC6 cells
float stateOfCharge[TABLE_LENGTH] = 
{
    0.0000f, 0.0100f, 0.0200f, 0.0300f, 0.0400f, 0.0500f, 0.0600f, 0.0700f, 0.0800f, 0.0900f, 0.1000f, 
    0.1100f, 0.1200f, 0.1300f, 0.1400f, 0.1500f, 0.1600f, 0.1700f, 0.1800f, 0.1900f, 0.2000f, 0.2100f, 
    0.2200f, 0.2300f, 0.2400f, 0.2500f, 0.2600f, 0.2700f, 0.2800f, 0.2900f, 0.3000f, 0.3100f, 0.3200f, 
    0.3300f, 0.3400f, 0.3500f, 0.3600f, 0.3700f, 0.3800f, 0.3900f, 0.4000f, 0.4100f, 0.4200f, 0.4300f, 
    0.4400f, 0.4500f, 0.4600f, 0.4700f, 0.4800f, 0.4900f, 0.5000f, 0.5100f, 0.5200f, 0.5300f, 0.5400f, 
    0.5500f, 0.5600f, 0.5700f, 0.5800f, 0.5900f, 0.6000f, 0.6100f, 0.6200f, 0.6300f, 0.6400f, 0.6500f, 
    0.6600f, 0.6700f, 0.6800f, 0.6900f, 0.7000f, 0.7100f, 0.7200f, 0.7300f, 0.7400f, 0.7500f, 0.7600f, 
    0.7700f, 0.7800f, 0.7900f, 0.8000f, 0.8100f, 0.8200f, 0.8300f, 0.8400f, 0.8500f, 0.8600f, 0.8700f, 
    0.8800f, 0.8900f, 0.9000f, 0.9100f, 0.9200f, 0.9300f, 0.9400f, 0.9500f, 0.9600f, 0.9700f, 0.9800f, 
    0.9900f, 1.0000f
};

// Open cell voltage for VTC6 cells. References SOC table
float openCellVoltage[TABLE_LENGTH] = 
{
    3.002f, 3.135f, 3.223f, 3.293f, 3.343f, 3.383f, 3.405f, 3.42f, 3.427f, 3.435f, 3.442f, 3.449f, 3.457f, 
    3.464f, 3.473f, 3.485f, 3.492f, 3.5f, 3.508f, 3.515f, 3.522f, 3.53f, 3.537f, 3.543f, 3.544f, 3.552f, 3.559f, 
    3.565f, 3.566f, 3.574f, 3.577f, 3.581f, 3.587f, 3.588f, 3.596f, 3.596f, 3.603f, 3.603f, 3.609f, 3.615f, 3.617f, 
    3.624f, 3.625f, 3.631f, 3.632f, 3.639f, 3.646f, 3.647f, 3.654f, 3.661f, 3.666f, 3.673f, 3.676f, 3.683f, 3.691f, 
    3.701f, 3.712f, 3.719f, 3.728f, 3.741f, 3.749f, 3.763f, 3.771f, 3.786f, 3.796f, 3.807f, 3.821f, 3.829f, 3.844f, 
    3.852f, 3.866f, 3.877f, 3.888f, 3.9f, 3.91f, 3.923f, 3.932f, 3.945f, 3.954f, 3.968f, 3.981f, 3.99f, 4.004f, 4.014f,
    4.027f, 4.04f, 4.054f, 4.063f, 4.078f, 4.092f, 4.101f, 4.114f, 4.129f, 4.143f, 4.155f, 4.166f, 4.18f, 4.195f, 4.209f, 
    4.23f, 4.267
};

// State of energy for VTC6 cells. References SOC table
float stateOfEnergy[TABLE_LENGTH] = 
{
    0.00f, 0.01f, 0.02f, 0.03f, 0.03f, 0.04f, 0.05f, 0.06f, 0.07f, 0.08f, 0.09f, 0.10f, 0.11f, 
    0.12f, 0.13f, 0.14f, 0.15f, 0.15f, 0.16f, 0.17f, 0.18f, 0.19f, 0.20f, 0.21f, 0.22f, 0.23f, 
    0.24f, 0.25f, 0.26f, 0.27f, 0.28f, 0.29f, 0.30f, 0.31f, 0.32f, 0.33f, 0.34f, 0.35f, 0.36f, 
    0.37f, 0.37f, 0.38f, 0.39f, 0.40f, 0.41f, 0.42f, 0.43f, 0.44f, 0.45f, 0.46f, 0.47f, 0.48f, 
    0.49f, 0.50f, 0.51f, 0.52f, 0.53f, 0.54f, 0.55f, 0.56f, 0.57f, 0.58f, 0.59f, 0.60f, 0.61f, 
    0.62f, 0.63f, 0.64f, 0.65f, 0.66f, 0.67f, 0.68f, 0.70f, 0.71f, 0.72f, 0.73f, 0.74f, 0.75f, 
    0.76f, 0.77f, 0.78f, 0.79f, 0.80f, 0.81f, 0.82f, 0.83f, 0.84f, 0.86f, 0.87f, 0.88f, 0.89f, 
    0.90f, 0.91f, 0.92f, 0.93f, 0.94f, 0.95f, 0.97f, 0.98f, 0.99f, 1.00
};