#include "../include/randomobstacledialog.h"
#include <QMessageBox>

RandomObstacleDialog::RandomObstacleDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("随机生成障碍物"));
    setModal(true);
    setFixedSize(350, 400);
    
    setupUI();
    
    // 连接信号和槽
    connect(generateButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(connectivityGroup, SIGNAL(buttonClicked(int)), this, SLOT(onConnectivityChanged()));
    connect(useSeedCheckBox, &QCheckBox::toggled, this, &RandomObstacleDialog::onUseSeedChanged);
    
    // 初始化状态
    onConnectivityChanged();
    onUseSeedChanged();
}

void RandomObstacleDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 障碍物密度设置
    QGroupBox *densityGroup = new QGroupBox(tr("障碍物密度"), this);
    QFormLayout *densityLayout = new QFormLayout(densityGroup);
    
    densitySpinBox = new QDoubleSpinBox(this);
    densitySpinBox->setRange(0.0, 1.0);
    densitySpinBox->setSingleStep(0.05);
    densitySpinBox->setDecimals(2);
    densitySpinBox->setValue(0.30);
    densitySpinBox->setSuffix(" (0-1)");
    
    densityLayout->addRow(tr("密度比例:"), densitySpinBox);
    
    QLabel *densityTip = new QLabel(tr("提示：密度越高，障碍物越多"), this);
    densityTip->setStyleSheet("color: gray; font-size: 11px;");
    densityLayout->addRow(densityTip);
    
    // 连通性设置
    QGroupBox *connectivityGroup = new QGroupBox(tr("连通性设置"), this);
    QVBoxLayout *connectivityLayout = new QVBoxLayout(connectivityGroup);
    
    this->connectivityGroup = new QButtonGroup(this);
    
    noPathRadio = new QRadioButton(tr("无可通行通路"), this);
    onePathRadio = new QRadioButton(tr("一条可通行通路"), this);
    multiplePathsRadio = new QRadioButton(tr("多条可通行通路"), this);
    
    this->connectivityGroup->addButton(noPathRadio, NoPath);
    this->connectivityGroup->addButton(onePathRadio, OnePath);
    this->connectivityGroup->addButton(multiplePathsRadio, MultiplePaths);
    
    // 默认选择一条可通行通路
    onePathRadio->setChecked(true);
    
    connectivityLayout->addWidget(noPathRadio);
    connectivityLayout->addWidget(onePathRadio);
    
    // 多条通路的数量设置
    QHBoxLayout *multipleLayout = new QHBoxLayout();
    multipleLayout->addWidget(multiplePathsRadio);
    
    pathCountSpinBox = new QSpinBox(this);
    pathCountSpinBox->setRange(2, 10);
    pathCountSpinBox->setValue(3);
    pathCountSpinBox->setSuffix(tr(" 条"));
    pathCountSpinBox->setMaximumWidth(80);
    
    multipleLayout->addWidget(pathCountSpinBox);
    multipleLayout->addStretch();
    
    connectivityLayout->addLayout(multipleLayout);
    
    QLabel *connectivityTip = new QLabel(tr("提示：无通路可用于测试算法的错误处理"), this);
    connectivityTip->setStyleSheet("color: gray; font-size: 11px;");
    connectivityLayout->addWidget(connectivityTip);
    
    // 随机种子设置
    QGroupBox *seedGroup = new QGroupBox(tr("随机种子"), this);
    QVBoxLayout *seedLayout = new QVBoxLayout(seedGroup);
    
    useSeedCheckBox = new QCheckBox(tr("使用固定种子（可复现结果）"), this);
    seedLayout->addWidget(useSeedCheckBox);
    
    QHBoxLayout *seedValueLayout = new QHBoxLayout();
    QLabel *seedLabel = new QLabel(tr("种子值:"), this);
    
    seedSpinBox = new QSpinBox(this);
    seedSpinBox->setRange(0, 999999);
    seedSpinBox->setValue(12345);
    seedSpinBox->setEnabled(false);
    
    seedValueLayout->addWidget(seedLabel);
    seedValueLayout->addWidget(seedSpinBox);
    seedValueLayout->addStretch();
    
    seedLayout->addLayout(seedValueLayout);
    
    QLabel *seedTip = new QLabel(tr("提示：相同种子会生成相同的障碍物分布"), this);
    seedTip->setStyleSheet("color: gray; font-size: 11px;");
    seedLayout->addWidget(seedTip);
    
    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    generateButton = new QPushButton(tr("生成障碍物"), this);
    generateButton->setMinimumWidth(100);
    generateButton->setStyleSheet("QPushButton { font-weight: bold; }");
    
    cancelButton = new QPushButton(tr("取消"), this);
    cancelButton->setMinimumWidth(80);
    
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(generateButton);
    
    // 添加到主布局
    mainLayout->addWidget(densityGroup);
    mainLayout->addWidget(connectivityGroup);
    mainLayout->addWidget(seedGroup);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void RandomObstacleDialog::onConnectivityChanged()
{
    int checkedId = connectivityGroup->checkedId();
    pathCountSpinBox->setEnabled(checkedId == MultiplePaths);
}

void RandomObstacleDialog::onUseSeedChanged()
{
    bool useSeed = useSeedCheckBox->isChecked();
    seedSpinBox->setEnabled(useSeed);
}

double RandomObstacleDialog::getObstacleDensity() const
{
    return densitySpinBox->value();
}

RandomObstacleDialog::ConnectivityType RandomObstacleDialog::getConnectivityType() const
{
    return static_cast<ConnectivityType>(connectivityGroup->checkedId());
}

int RandomObstacleDialog::getPathCount() const
{
    return pathCountSpinBox->value();
}

bool RandomObstacleDialog::isUseSeed() const
{
    return useSeedCheckBox->isChecked();
}

int RandomObstacleDialog::getSeed() const
{
    return seedSpinBox->value();
} 