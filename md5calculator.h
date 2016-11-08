#ifndef MD5CALCULATOR_H
#define MD5CALCULATOR_H

#include <QtGui>
#include <QtWidgets>
#include <QThread>
#include <string>

// the same as the one used by the SHA1 class
#define MD5_MAX_FILE_BUFFER (32 * 20 * 820)

namespace Ui {
class Md5Calculator;
}

typedef enum AppPopupDialog
{
    ERROR_FILE
} AppPopupDialog;

class WorkThread;

class Md5Calculator : public QWidget
{
    Q_OBJECT

public:
    explicit Md5Calculator(QWidget* parent = 0);
    ~Md5Calculator();

private slots:
    void OpenAndHash();
    void CopyMD5HashToClipboard();
    void CopySHA1HashToClipboard();
    void PasteFromClipboard();
    void VerifyHash();

    void onThreadStarted();
    void onThreadEnded();
    void onHashProgressChanged(double);
    void onMD5HashReceived(QString);
    void onSHA1HashReceived(QString);
    void onDialogPopup(AppPopupDialog);

private:
    Ui::Md5Calculator* ui;
    WorkThread* m_pWorkThread;

};

class WorkThread : public QThread
{
    Q_OBJECT

    //friend class MD5Calculator;

public:
    explicit WorkThread(QObject* pParent, const QString& qstrFile);
    void run();

signals:
    // To communicate with Gui Thread
    // we need to emit a signal
    void HashProgressChanged(double);
    void MD5Hash(QString);
    void SHA1Hash(QString);
    void dialogPopup(AppPopupDialog);

private:
    Md5Calculator* m_pOwner;
    const QString m_qstrFile;

};

#endif // MD5CALCULATOR_H
