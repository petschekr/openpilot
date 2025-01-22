#include "selfdrive/ui/qt/offroad/ioniq.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

IoniqWidget::IoniqWidget(QWidget* parent) : QFrame(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(80, 50, 80, 50);
  main_layout->setSpacing(0);

  title = new QLabel();
  title->setStyleSheet("font-size: 75px;");
  main_layout->addWidget(title, 0, Qt::AlignTop);
  main_layout->addSpacing(50);

  primaryDetails = new QLabel();
  primaryDetails->setStyleSheet("font-size: 60px; font-weight: light; color: white;");
  primaryDetails->setWordWrap(true);
  main_layout->addWidget(primaryDetails, 0, Qt::AlignTop);
  main_layout->addSpacing(30);

  secondaryDetails = new QLabel();
  secondaryDetails->setStyleSheet("font-size: 42px; font-weight: light; color: white;");
  secondaryDetails->setWordWrap(true);
  main_layout->addWidget(secondaryDetails, 0, Qt::AlignTop);

  main_layout->addStretch();

  sunTimes = new QLabel();
  sunTimes->setStyleSheet("font-size: 56px; font-weight: light; color: white;");
  main_layout->addWidget(sunTimes, 0, Qt::AlignBottom);

  setStyleSheet(R"(
    IoniqWidget {
      border-radius: 10px;
      background-color: #333333;
    }
  )");
}

void IoniqWidget::updateState(const UIState& s) {
  const SubMaster& sm = *(s.sm);

  const auto& ioniq_data = sm["ioniq"].getIoniq();
  const auto chargingType = ioniq_data.getChargingType();

  QString titleString = QString("<b><i>Ioniq 5</i></b> - %1%");
  titleString = titleString.arg(QString::number(ioniq_data.getSocDisplay(), 'f', 1));
  title->setText(titleString);

  QString primaryDetailsStr = QString("%1");
  if (chargingType == cereal::Ioniq::ChargingType::NOT_CHARGING) {
    primaryDetailsStr = primaryDetailsStr.arg("Last trip: <b>%1 kWh</b><br />Since charge: <b>%2 kWh</b>");
  }
  else if (chargingType == cereal::Ioniq::ChargingType::AC) {
    primaryDetailsStr = primaryDetailsStr.arg("AC slow charging<br /><b>%1 kW</b> - %2 kWh");
  }
  else if (chargingType == cereal::Ioniq::ChargingType::DC) {
    primaryDetailsStr = primaryDetailsStr.arg("DC fast charging<br /><b>%1 / %3 kW</b> - %2 kWh<br />%4 / %5 A");
  }
  else {
    primaryDetailsStr = primaryDetailsStr.arg("Other charging<br /><b>%1 kW</b> - %2 kWh");
  }

  if (chargingType == cereal::Ioniq::ChargingType::NOT_CHARGING) {
    primaryDetailsStr = primaryDetailsStr.arg(QString::number(ioniq_data.getEnergySinceIgnition() / 1000.0, 'f', 1));
    primaryDetailsStr = primaryDetailsStr.arg(QString::number(ioniq_data.getEnergySinceCharging() / 1000.0, 'f', 1));
  }
  else {
    primaryDetailsStr = primaryDetailsStr.arg(QString::number(ioniq_data.getVoltage() * ioniq_data.getCurrent() / -1000.0, 'f', 1));
    primaryDetailsStr = primaryDetailsStr.arg(QString::number(ioniq_data.getEnergySinceCharging() / -1000.0, 'f', 1));
  }
  if (chargingType == cereal::Ioniq::ChargingType::DC) {
    primaryDetailsStr = primaryDetailsStr.arg(QString::number(ioniq_data.getMaximumChargePower(), 'f', 1));
    primaryDetailsStr = primaryDetailsStr.arg(QString::number(ioniq_data.getCurrent() * -1.0, 'f', 1));
    primaryDetailsStr = primaryDetailsStr.arg(QString::number(ioniq_data.getMaximumChargeCurrent(), 'f', 1));
  }
  primaryDetails->setText(primaryDetailsStr);

  QString secondaryDetailsStr = QString("")
    .append("Charge / Discharge: <b>%1 kW</b> / <b>%2 kW</b><br />")
    .append("Battery Temp: <b>%3</b> - <b>%4 °C</b><br />")
    .append("Heater Temp: <b>%5 °C</b> / Inlet Temp: <b>%6 °C</b><br />")
    .append("AC Inlet: <b>%7 °C</b> / DC Inlet: <b>%8</b> - <b>%9 °C</b>");
  secondaryDetailsStr = secondaryDetailsStr.arg(QString::number(ioniq_data.getAvailableChargePower(), 'f', 0));
  secondaryDetailsStr = secondaryDetailsStr.arg(QString::number(ioniq_data.getAvailableDischargePower(), 'f', 0));
  secondaryDetailsStr = secondaryDetailsStr.arg(ioniq_data.getMinBatteryTemp()).arg(ioniq_data.getMaxBatteryTemp());
  secondaryDetailsStr = secondaryDetailsStr.arg(ioniq_data.getHeaterTemp()).arg(ioniq_data.getBatteryInletTemp());
  secondaryDetailsStr = secondaryDetailsStr.arg(ioniq_data.getAcInletTemp()).arg(ioniq_data.getDcInlet1Temp()).arg(ioniq_data.getDcInlet2Temp());
  secondaryDetails->setText(secondaryDetailsStr);

  QString sunTimesString = QString("Sunrise: <b>%1</b><br />Sunset: <b>%2</b>");
  sunTimes->setText(
    sunTimesString
      .arg(ioniq_data.getSunrise().cStr())
      .arg(ioniq_data.getSunset().cStr())
  );

  update();
}
