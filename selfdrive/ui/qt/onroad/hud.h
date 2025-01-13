#pragma once

#include <QPainter>
#include "selfdrive/ui/ui.h"

class HudRenderer : public QObject {
  Q_OBJECT

public:
  HudRenderer();
  void updateState(const UIState &s);
  void draw(QPainter &p, const QRect &surface_rect);

private:
  void drawSetSpeed(QPainter &p, const QRect &surface_rect);
  void drawCurrentSpeed(QPainter &p, const QRect &surface_rect);
  void drawAltitude(QPainter &p, const QRect &surface_rect);
  void drawPower(QPainter &p, const QRect &surface_rect);
  void drawEnergy(QPainter& p, const QRect& surface_rect);
  void drawRequestedCurrent(QPainter& p, const QRect& surface_rect);
  void drawRequestedPower(QPainter& p, const QRect& surface_rect);

  void drawChargePower(QPainter& p, const QRect& surface_rect);
  void drawDischargePower(QPainter& p, const QRect& surface_rect);
  void drawBatteryTemps(QPainter& p, const QRect& surface_rect);
  void drawOtherTemps(QPainter& p, const QRect& surface_rect);
  void drawSinceChargeEnergy(QPainter& p, const QRect& surface_rect);
  void drawSuntimes(QPainter& p, const QRect& surface_rect);

  void drawText(QPainter &p, int x, int y, const QString &text, int alpha = 255);

  float speed = 0;
  float set_speed = 0;

  int fullScreenWidth = 0;

  bool shouldEnergyReset = true;
  cereal::Ioniq::ChargingType lastChargingType = cereal::Ioniq::ChargingType::NOT_CHARGING;
  double altitude = 0;
  float power = 0;
  float current = 0;
  float energySinceIgnition = 0;
  float energySinceCharging = 0;

  float maxChargePower = 0;
  float maxDischargePower = 0;
  float maxRequestedChargePower = 0;
  float maxRequestedChargeCurrent = 0;
  int8_t minBatteryTemp = 0;
  int8_t maxBatteryTemp = 0;
  int8_t batteryInletTemp = 0;
  int8_t heaterTemp = 0;
  std::string sunrise = "--:--";
  std::string sunset = "--:--";

  bool is_cruise_set = false;
  bool is_cruise_available = true;
  bool is_metric = false;
  bool v_ego_cluster_seen = false;
  int status = STATUS_DISENGAGED;
};
