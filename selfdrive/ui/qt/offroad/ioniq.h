#pragma once

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "selfdrive/ui/ui.h"

class IoniqWidget : public QFrame {
  Q_OBJECT
public:
  explicit IoniqWidget(QWidget* parent = 0);
  void updateState(const UIState& s);
private:
  QLabel* title;
  QLabel* primaryDetails;
  QLabel* secondaryDetails;
  QLabel* sunTimes;
};
