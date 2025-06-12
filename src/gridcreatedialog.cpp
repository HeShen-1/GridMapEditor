#include "../include/gridcreatedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>

GridCreateDialog::GridCreateDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("RasterMapEditor"));
    setFixedSize(300, 150);

    // 创建控件
    rowsLabel = new QLabel(tr("行数:"), this);
    colsLabel = new QLabel(tr("列数:"), this);
    
    rowsSpinBox = new QSpinBox(this);
    rowsSpinBox->setRange(1, 100);
    rowsSpinBox->setValue(10);
    
    colsSpinBox = new QSpinBox(this);
    colsSpinBox->setRange(1, 100);
    colsSpinBox->setValue(10);

    okButton = new QPushButton(tr("OK"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);

    // 创建布局
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(rowsLabel, 0, 0);
    gridLayout->addWidget(rowsSpinBox, 0, 1);
    gridLayout->addWidget(colsLabel, 1, 0);
    gridLayout->addWidget(colsSpinBox, 1, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // 连接信号和槽
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // 设置样式
    setStyleSheet(R"(
        QDialog {
            background-color: white;
        }
        QLabel {
            font-size: 12px;
        }
        QSpinBox {
            min-width: 80px;
            padding: 2px;
        }
        QPushButton {
            min-width: 80px;
            padding: 5px;
        }
    )");
} 