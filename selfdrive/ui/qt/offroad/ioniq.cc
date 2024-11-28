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

  QLabel *upgrade = new QLabel(tr("Ioniq 5"));
  upgrade->setStyleSheet("font-size: 75px; font-weight: bold;");
  main_layout->addWidget(upgrade, 0, Qt::AlignTop);
  main_layout->addSpacing(50);

  description = new QLabel(tr("Become a comma prime member at connect.comma.ai"));
  description->setStyleSheet("font-size: 56px; font-weight: light; color: white;");
  description->setWordWrap(true);
  main_layout->addWidget(description, 0, Qt::AlignTop);

  main_layout->addStretch();

  QLabel *features = new QLabel(tr("PRIME FEATURES:"));
  features->setStyleSheet("font-size: 41px; font-weight: bold; color: #E5E5E5;");
  main_layout->addWidget(features, 0, Qt::AlignBottom);
  main_layout->addSpacing(30);

  QVector<QString> bullets = {tr("Remote access"), tr("24/7 LTE connectivity"), tr("1 year of drive storage"), tr("Remote snapshots")};
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
  descriptionStr.append("%");
  description->setText(descriptionStr);
}
