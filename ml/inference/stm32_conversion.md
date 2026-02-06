# ML Threshold Values (STM32 Deployment)

This document lists the **reconstruction error statistics** obtained from the trained autoencoder and used directly on the STM32 for machine health classification.

---


## Stored Statistical Parameters

These values are computed offline using the trained autoencoder on healthy machine data and are flashed into the STM32.

| Parameter | Macro Name | Value |
|---------|------------|-------|
| Mean RMS Current | `MEAN_IRMS` | 0.19303803 |
| Mean Vibration Magnitude | `MEAN_VIBMAG` | 0.35111946 |
| Standard Deviation (Current) | `SD_IRMS` | 0.00615908 |
| Standard Deviation (Vibration) | `SD_VIBMAG` | 0.04134532 |

---

## Anomaly Detection Threshold

| Parameter | Macro Name | Value |
|---------|------------|-------|
| Combined Threshold | `THRESHOLD` | 0.2892727262753771 |

---

## STM32 Runtime Decision Logic

| Condition | Machine State |
|---------|---------------|
| Error ≤ 0.2892727262753771 | Healthy |
| Error > 0.2892727262753771 | Fault / Anomaly |

---

## Embedded Usage Notes

- μ, σ, and k are **computed offline (Python)**
- Values are **stored in flash memory** on STM32
- Only simple arithmetic is performed at runtime
- No retraining occurs on the microcontroller

---

## Remarks

These values represent **baseline machine behavior** under normal operating conditions. Any reconstruction error exceeding the threshold indicates a statistically significant deviation from learned patterns.
