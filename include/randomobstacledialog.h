#ifndef RANDOMOBSTACLEDIALOG_H
#define RANDOMOBSTACLEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>

class RandomObstacleDialog : public QDialog
{
    Q_OBJECT

public:
    enum ConnectivityType {
        NoPath = 0,        // 无可通行通路
        OnePath = 1,       // 一条可通行通路
        MultiplePaths = 2  // 多条可通行通路
    };

    explicit RandomObstacleDialog(QWidget *parent = nullptr);

    // 获取设置的参数
    double getObstacleDensity() const;
    ConnectivityType getConnectivityType() const;
    int getPathCount() const;
    bool isUseSeed() const;
    int getSeed() const;

private slots:
    void onConnectivityChanged();
    void onUseSeedChanged();

private:
    void setupUI();

private:
    // 障碍物密度
    QDoubleSpinBox *densitySpinBox;
    
    // 连通性设置
    QButtonGroup *connectivityGroup;
    QRadioButton *noPathRadio;
    QRadioButton *onePathRadio;
    QRadioButton *multiplePathsRadio;
    QSpinBox *pathCountSpinBox;
    
    // 随机种子
    QCheckBox *useSeedCheckBox;
    QSpinBox *seedSpinBox;
    
    // 按钮
    QPushButton *generateButton;
    QPushButton *cancelButton;
};

#endif // RANDOMOBSTACLEDIALOG_H 