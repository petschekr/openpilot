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
  main_layout->setContentsMargins(80, 90, 80, 60);
  main_layout->setSpacing(0);

  QLabel *upgrade = new QLabel("Ioniq 5");
  upgrade->setStyleSheet("font-size: 75px; font-weight: bold;");
  main_layout->addWidget(upgrade, 0, Qt::AlignTop);
  main_layout->addSpacing(50);

  description = new QLabel();
  description->setStyleSheet("font-size: 56px; font-weight: light; color: white;");
  description->setWordWrap(true);
  main_layout->addWidget(description, 0, Qt::AlignTop);

  main_layout->addStretch();

  QLabel *features = new QLabel("PRIME FEATURES:");
  features->setStyleSheet("font-size: 41px; font-weight: bold; color: #E5E5E5;");
  main_layout->addWidget(features, 0, Qt::AlignBottom);
  main_layout->addSpacing(30);

  QVector<QString> bullets = {"Remote access", "24/7 LTE connectivity", "1 year of drive storage", "Remote snapshots"};
  for (auto &b : bullets) {
    const QString check = "<b><font color='#465BEA'>✓</font></b> ";
    QLabel *l = new QLabel(check + b);
    l->setAlignment(Qt::AlignLeft);
    l->setStyleSheet("font-size: 50px; margin-bottom: 15px;");
    main_layout->addWidget(l, 0, Qt::AlignBottom);
  }

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

  QString descriptionStr = QString::number(ioniq_data.getSocDisplay(), 'f', 1);
  descriptionStr.append("%\n");
  if (ioniq_data.getChargingType() == cereal::Ioniq::ChargingType::NOT_CHARGING) {
    descriptionStr.append("Not charging");
  }
  else if (ioniq_data.getChargingType() == cereal::Ioniq::ChargingType::AC) {
    descriptionStr.append("AC slow charging");
  }
  else if (ioniq_data.getChargingType() == cereal::Ioniq::ChargingType::DC) {
    descriptionStr.append("DC fast charging");
  }
  else {
    descriptionStr.append("Other charging type");
  }
  description->setText(descriptionStr);
}
