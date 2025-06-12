#ifndef GRIDCREATEDIALOG_H
#define GRIDCREATEDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

class GridCreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GridCreateDialog(QWidget *parent = nullptr);
    int getRows() const { return rowsSpinBox->value(); }
    int getCols() const { return colsSpinBox->value(); }

private:
    QSpinBox *rowsSpinBox;
    QSpinBox *colsSpinBox;
    QLabel *rowsLabel;
    QLabel *colsLabel;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // GRIDCREATEDIALOG_H 