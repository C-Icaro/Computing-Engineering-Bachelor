#ifndef OPERATION_MODE_H
#define OPERATION_MODE_H

#include <Arduino.h>

enum class OperationMode
{
  HIBERNATION,  // Câmera desligada, apenas monitora PIR
  AUTO,         // Liga câmera quando PIR detecta movimento
  MANUAL,       // Aguarda comando remoto para capturar
  VIGILANCE     // Câmera ativa, captura frames em intervalos
};

class OperationModeController
{
public:
  OperationModeController() : currentMode(OperationMode::HIBERNATION) {}

  void setMode(OperationMode mode)
  {
    if (currentMode != mode)
    {
      OperationMode oldMode = currentMode;
      currentMode = mode;
      Serial.printf("[Mode] Modo alterado: %s -> %s\n", 
                    modeToString(oldMode).c_str(), 
                    modeToString(mode).c_str());
    }
  }

  OperationMode getMode() const
  {
    return currentMode;
  }

  String getModeString() const
  {
    return modeToString(currentMode);
  }

  bool isHibernation() const
  {
    return currentMode == OperationMode::HIBERNATION;
  }

  bool isAuto() const
  {
    return currentMode == OperationMode::AUTO;
  }

  bool isManual() const
  {
    return currentMode == OperationMode::MANUAL;
  }

  bool isVigilance() const
  {
    return currentMode == OperationMode::VIGILANCE;
  }

  static OperationMode stringToMode(const String &modeStr)
  {
    String lower = modeStr;
    lower.toLowerCase();
    
    if (lower == "hibernation" || lower == "hibernar")
      return OperationMode::HIBERNATION;
    else if (lower == "auto" || lower == "automatico")
      return OperationMode::AUTO;
    else if (lower == "manual")
      return OperationMode::MANUAL;
    else if (lower == "vigilance" || lower == "vigilancia")
      return OperationMode::VIGILANCE;
    
    return OperationMode::HIBERNATION; // Default
  }

private:
  OperationMode currentMode;

  String modeToString(OperationMode mode) const
  {
    switch (mode)
    {
      case OperationMode::HIBERNATION:
        return "HIBERNATION";
      case OperationMode::AUTO:
        return "AUTO";
      case OperationMode::MANUAL:
        return "MANUAL";
      case OperationMode::VIGILANCE:
        return "VIGILANCE";
      default:
        return "UNKNOWN";
    }
  }
};

#endif // OPERATION_MODE_H




