#ifndef EXAMPLECODEDIALOG_H
#define EXAMPLECODEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>

class ExampleCodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExampleCodeDialog(QWidget *parent = nullptr);
    QString getSelectedCode() const;
    QString getSelectedAlgorithm() const;
    QString getSelectedLanguage() const;

private slots:
    void onAlgorithmChanged();
    void onLanguageChanged();
    void onCodeSelected();

private:
    void setupUI();
    void loadExampleCodes();

private:
    QComboBox *algorithmComboBox;
    QComboBox *languageComboBox;
    QTextEdit *codePreview;
    QPushButton *selectButton;
    QPushButton *cancelButton;
    
    // 存储示例代码的映射表
    // key: "算法名称-语言", value: 代码内容
    QMap<QString, QString> exampleCodes;
    
    QString selectedCode;
};

#endif // EXAMPLECODEDIALOG_H 