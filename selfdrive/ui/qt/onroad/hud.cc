#include "selfdrive/ui/qt/onroad/hud.h"

#include <cmath>

#include "selfdrive/ui/qt/util.h"

constexpr int SET_SPEED_NA = 255;

HudRenderer::HudRenderer() {}

void HudRenderer::updateState(const UIState &s) {
  is_metric = s.scene.is_metric;
  status = s.status;

  const SubMaster &sm = *(s.sm);
  if (sm.rcv_frame("carState") < s.scene.started_frame) {
    is_cruise_set = false;
    set_speed = SET_SPEED_NA;
    speed = 0.0;
    return;
  }

  const auto &controls_state = sm["controlsState"].getControlsState();
  const auto &car_state = sm["carState"].getCarState();
  const auto &ioniq_data = sm["ioniq"].getIoniq();

  // Handle older routes where vCruiseCluster is not set
  set_speed = car_state.getVCruiseCluster() == 0.0 ? controls_state.getVCruiseDEPRECATED() : car_state.getVCruiseCluster();
  is_cruise_set = set_speed > 0 && set_speed != SET_SPEED_NA;
  is_cruise_available = set_speed != -1;

  if (is_cruise_set && !is_metric) {
    set_speed *= KM_TO_MILE;
  }

  // Handle older routes where vEgoCluster is not set
  v_ego_cluster_seen = v_ego_cluster_seen || car_state.getVEgoCluster() != 0.0;
  float v_ego = v_ego_cluster_seen ? car_state.getVEgoCluster() : car_state.getVEgo();
  speed = std::max<float>(0.0f, v_ego * (is_metric ? MS_TO_KPH : MS_TO_MPH));

  altitude = ioniq_data.getAltitudeMsl();

  power = ioniq_data.getVoltage() * ioniq_data.getCurrent() / 1000.0;
  current = ioniq_data.getCurrent();

  energySinceIgnition = ioniq_data.getEnergySinceIgnition() / 1000.0;
  energySinceCharging = ioniq_data.getEnergySinceCharging() / 1000.0;

  maxChargePower = ioniq_data.getAvailableChargePower();
  maxDischargePower = ioniq_data.getAvailableDischargePower();
  maxRequestedChargeCurrent = ioniq_data.getMaximumChargeCurrent();
  maxRequestedChargePower = ioniq_data.getMaximumChargePower();
  minBatteryTemp = ioniq_data.getMinBatteryTemp();
  maxBatteryTemp = ioniq_data.getMaxBatteryTemp();
  batteryInletTemp = ioniq_data.getBatteryInletTemp();
  heaterTemp = ioniq_data.getHeaterTemp();
  sunrise = ioniq_data.getSunrise();
  sunset = ioniq_data.getSunset();
}

void HudRenderer::draw(QPainter &p, const QRect &surface_rect) {
  p.save();

  // Draw header gradient
  QLinearGradient bg(0, UI_HEADER_HEIGHT - (UI_HEADER_HEIGHT / 2.5), 0, UI_HEADER_HEIGHT);
  bg.setColorAt(0, QColor::fromRgbF(0, 0, 0, 0.45));
  bg.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));
  p.fillRect(0, 0, surface_rect.width(), UI_HEADER_HEIGHT, bg);

  if (fullScreenWidth == 0) {
    fullScreenWidth = surface_rect.width();
  }

  if (is_cruise_available) {
    drawSetSpeed(p, surface_rect);
  }
  drawCurrentSpeed(p, surface_rect);

  if (surface_rect.width() == fullScreenWidth) {
    drawAltitude(p, surface_rect);
    drawPower(p, surface_rect);
    drawEnergy(p, surface_rect);
    if (maxRequestedChargeCurrent > 0.0) {
      drawRequestedCurrent(p, surface_rect);
      drawRequestedPower(p, surface_rect);
    }
  }
  else {
    drawChargePower(p, surface_rect);
    drawDischargePower(p, surface_rect);
    drawBatteryTemps(p, surface_rect);
    drawOtherTemps(p, surface_rect);
    drawSinceChargeEnergy(p, surface_rect);
    drawSuntimes(p, surface_rect);
  }

  p.restore();
}

void HudRenderer::drawSetSpeed(QPainter &p, const QRect &surface_rect) {
  // Draw outer box + border to contain set speed
  const QSize default_size = {172, 204};
  QSize set_speed_size = is_metric ? QSize(200, 204) : default_size;
  QRect set_speed_rect(QPoint(60 + (default_size.width() - set_speed_size.width()) / 2, 45), set_speed_size);

  // Draw set speed box
  p.setPen(QPen(QColor(255, 255, 255, 75), 6));
  p.setBrush(QColor(0, 0, 0, 166));
  p.drawRoundedRect(set_speed_rect, 32, 32);

  // Colors based on status
  QColor max_color = QColor(0xa6, 0xa6, 0xa6, 0xff);
  QColor set_speed_color = QColor(0x72, 0x72, 0x72, 0xff);
  if (is_cruise_set) {
    set_speed_color = QColor(255, 255, 255);
    if (status == STATUS_DISENGAGED) {
      max_color = QColor(255, 255, 255);
    } else if (status == STATUS_OVERRIDE) {
      max_color = QColor(0x91, 0x9b, 0x95, 0xff);
    } else {
      max_color = QColor(0x80, 0xd8, 0xa6, 0xff);
    }
  }

  // Draw "MAX" text
  p.setFont(InterFont(40, QFont::DemiBold));
  p.setPen(max_color);
  p.drawText(set_speed_rect.adjusted(0, 27, 0, 0), Qt::AlignTop | Qt::AlignHCenter, tr("MAX"));

  // Draw set speed
  QString setSpeedStr = is_cruise_set ? QString::number(std::nearbyint(set_speed)) : "–";
  p.setFont(InterFont(90, QFont::Bold));
  p.setPen(set_speed_color);
  p.drawText(set_speed_rect.adjusted(0, 77, 0, 0), Qt::AlignTop | Qt::AlignHCenter, setSpeedStr);
}

void HudRenderer::drawCurrentSpeed(QPainter &p, const QRect &surface_rect) {
  QString speedStr = QString::number(std::nearbyint(speed));

  p.setFont(InterFont(176, QFont::Bold));
  drawText(p, surface_rect.center().x(), 210, speedStr);

  p.setFont(InterFont(66));
  drawText(p, surface_rect.center().x(), 290, is_metric ? tr("km/h") : tr("mph"), 200);
}

void HudRenderer::drawPower(QPainter &p, const QRect &surface_rect) {
  QString powerStr = QString::number(power, 'f', 1);
  powerStr.append(" kW");

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 445, "Power", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 370, powerStr);
}
void HudRenderer::drawEnergy(QPainter &p, const QRect &surface_rect) {
  QString powerStr = QString::number(energySinceIgnition, 'f', 1);
  powerStr.append(" kWh");

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 285, "Energy", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 210, powerStr);
}
void HudRenderer::drawRequestedPower(QPainter &p, const QRect &surface_rect) {
  QString powerStr = QString::number(maxRequestedChargePower, 'f', 1);
  powerStr.append(" kW");

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 650, surface_rect.bottomLeft().y() - 445, "Requested Power", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 650, surface_rect.bottomLeft().y() - 370, powerStr);
}
void HudRenderer::drawRequestedCurrent(QPainter &p, const QRect &surface_rect) {
  QString str = "%1/%2 A";
  str = str.arg(QString::number(current * -1, 'f', 1)).arg(QString::number(maxRequestedChargeCurrent, 'f', 1));

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 650, surface_rect.bottomLeft().y() - 285, "Amps", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 650, surface_rect.bottomLeft().y() - 210, str);
}
void HudRenderer::drawAltitude(QPainter &p, const QRect &surface_rect) {
  QString altitudeStr = QString::number(altitude * METER_TO_FOOT, 'f', 0);
  altitudeStr.append(" ft");

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 125, "Altitude", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 50, altitudeStr);
}

void HudRenderer::drawChargePower(QPainter &p, const QRect &surface_rect) {
  QString str = "%1 kW";
  str = str.arg(QString::number(maxChargePower, 'f', 0));
  QString str2 = "%1%";
  str2 = str2.arg(QString::number(maxChargePower / 277.0 * 100.0, 'f', 0));

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 700, "Charge", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 625, str);

  p.setFont(InterFont(60, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 565, str2);
}
void HudRenderer::drawDischargePower(QPainter &p, const QRect &surface_rect) {
  QString str = "%1 kW";
  str = str.arg(QString::number(maxDischargePower, 'f', 0));
  QString str2 = "%1%";
  str2 = str2.arg(QString::number(maxDischargePower / 277.0 * 100.0, 'f', 0));

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 490, "Discharge", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 415, str);

  p.setFont(InterFont(60, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 355, str2);
}
void HudRenderer::drawBatteryTemps(QPainter &p, const QRect &surface_rect) {
  QString str = "%1 - %2 °C";
  str = str.arg(minBatteryTemp).arg(maxBatteryTemp);

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 275, "Battery", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 200, str);
}
void HudRenderer::drawOtherTemps(QPainter &p, const QRect &surface_rect) {
  QString str = "%1 / %2 °C";
  str = str.arg(heaterTemp).arg(batteryInletTemp);

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 125, "Heater/Inlet", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 175, surface_rect.bottomLeft().y() - 50, str);
}
void HudRenderer::drawSinceChargeEnergy(QPainter &p, const QRect &surface_rect) {
  QString str = QString::number(energySinceCharging, 'f', 1);
  str.append(" kWh");

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 660, surface_rect.bottomLeft().y() - 275, "Since Last Charge", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 660, surface_rect.bottomLeft().y() - 200, str);
}
void HudRenderer::drawSuntimes(QPainter &p, const QRect &surface_rect) {
  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 525, surface_rect.bottomLeft().y() - 125, "Sunrise", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 525, surface_rect.bottomLeft().y() - 50, QString::fromStdString(sunrise));

  p.setFont(InterFont(60));
  drawText(p, surface_rect.bottomLeft().x() + 800, surface_rect.bottomLeft().y() - 125, "Sunset", 200);

  p.setFont(InterFont(70, QFont::Bold));
  drawText(p, surface_rect.bottomLeft().x() + 800, surface_rect.bottomLeft().y() - 50, QString::fromStdString(sunset));
}

void HudRenderer::drawText(QPainter &p, int x, int y, const QString &text, int alpha) {
  QRect real_rect = p.fontMetrics().boundingRect(text);
  real_rect.moveCenter({x, y - real_rect.height() / 2});

  p.setPen(QColor(0xff, 0xff, 0xff, alpha));
  p.drawText(real_rect.x(), real_rect.bottom(), text);
}
