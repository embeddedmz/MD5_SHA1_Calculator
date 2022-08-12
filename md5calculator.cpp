#include <string>
#include "md5calculator.h"
#include "ui_md5calculator.h"
#include "md5.h"
#include "SHA1.h"
#ifndef _WINDOWS
#include <unistd.h>
#endif

Md5Calculator::Md5Calculator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Md5Calculator),
    m_pWorkThread(nullptr)
{
    // ************** Miscellaneous
    // **************

    ui->setupUi(this);
    setFixedSize(geometry().width(),geometry().height());
    qRegisterMetaType<AppPopupDialog>("AppPopupDialog");

    connect(ui->uiBrowseBtn, SIGNAL(clicked()), this, SLOT(OpenAndHash()));
    connect(ui->uiCopyMD5Btn, SIGNAL(clicked()), this, SLOT(CopyMD5HashToClipboard()));
    connect(ui->uiCopySHA1Btn, SIGNAL(clicked()), this, SLOT(CopySHA1HashToClipboard()));
    connect(ui->uiPasteBtn, SIGNAL(clicked()), this, SLOT(PasteFromClipboard()));
    connect(ui->uiVerifyBtn, SIGNAL(clicked()), this, SLOT(VerifyHash()));
    connect(ui->uiCancelBtn, &QPushButton::clicked, this, &Md5Calculator::CancelHashGeneration);
}

void Md5Calculator::OpenAndHash()
{
    ui->uiFile->setText(QFileDialog::getOpenFileName(this, tr("Open a file"), QString(), "*.*"));
    if( !(ui->uiFile->text().isEmpty()) )
    {
        Q_ASSERT(m_pWorkThread == nullptr);

        // GUI ops before hash
        ui->uiBrowseBtn->setEnabled(false);
        ui->uiCopyMD5Btn->setEnabled(false);
        ui->uiCopySHA1Btn->setEnabled(false);
        ui->uiVerifyBtn->setEnabled(false);
        ui->uiPasteBtn->setEnabled(false);
        ui->uiCancelBtn->setEnabled(true);
        ui->uiPB->setValue(0);

        m_pWorkThread = new WorkThread(this, ui->uiFile->text());
        connect(m_pWorkThread, SIGNAL(HashProgressChanged(double)), this, SLOT(onHashProgressChanged(double)));
        connect(m_pWorkThread, SIGNAL(started()), this, SLOT(onThreadStarted()));
        connect(m_pWorkThread, SIGNAL(finished()), this, SLOT(onThreadEnded()));
        connect(m_pWorkThread, SIGNAL(MD5Hash(QString)), this, SLOT(onMD5HashReceived(QString)));
        connect(m_pWorkThread, SIGNAL(SHA1Hash(QString)), this, SLOT(onSHA1HashReceived(QString)));
        connect(m_pWorkThread, SIGNAL(dialogPopup(AppPopupDialog)), this, SLOT(onDialogPopup(AppPopupDialog)));
        m_pWorkThread->start();
    }
}

void Md5Calculator::CopyMD5HashToClipboard()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(ui->uiMD5->text());
    QMessageBox::information(this, tr("Information"), tr("MD5 Checksum has been copied to clipboard"));
}

void Md5Calculator::CopySHA1HashToClipboard()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(ui->uiSHA1->text());
    QMessageBox::information(this, tr("Information"), tr("SHA1 Checksum has been copied to clipboard"));
}

void Md5Calculator::PasteFromClipboard()
{
    QClipboard* clipboard = QApplication::clipboard();
    ui->uiHash->setText(clipboard->text());
}

void Md5Calculator::VerifyHash()
{
    if (ui->uiHash->text().toUpper() == ui->uiMD5->text())
        QMessageBox::information(this, tr("Matched"), tr("MD5 Hash Matched."));
    else if (ui->uiHash->text().toUpper() == ui->uiSHA1->text())
        QMessageBox::information(this, tr("Matched"), tr("SHA1 Hash Matched."));
    else
        QMessageBox::warning(this, tr("Not match"), tr("Hash does not match !"));
}

void Md5Calculator::CancelHashGeneration()
{
    if (m_pWorkThread)
    {
        m_pWorkThread->requestInterruption();
    }
}

void Md5Calculator::onHashProgressChanged(double dProgression)
{
    ui->uiPB->setValue(dProgression*100);
}

void Md5Calculator::onThreadStarted()
{
    // instructions here will not necessarily be
    // executed before WorkThread::run()
}

void Md5Calculator::onThreadEnded()
{
    m_pWorkThread->wait();

    if (m_pWorkThread->isFinished())
    {
        delete m_pWorkThread; m_pWorkThread = nullptr;
    }

    ui->uiBrowseBtn->setEnabled(true);
    ui->uiCopyMD5Btn->setEnabled(true);
    ui->uiCopySHA1Btn->setEnabled(true);
    ui->uiVerifyBtn->setEnabled(true);
    ui->uiCancelBtn->setEnabled(false);
    ui->uiPasteBtn->setEnabled(true);
    ui->uiPB->setValue(0);
}

void Md5Calculator::onDialogPopup(AppPopupDialog eDialog)
{
    switch (eDialog)
    {
        case ERROR_FILE:
            QMessageBox::critical(this, tr("Error"), tr("Could not open the file for reading."));
            break;
        default:
            QMessageBox::critical(this, tr("Unknown event"), tr("Something strange happened !"));
            break;
    }
}

void Md5Calculator::onMD5HashReceived(QString qstrHash)
{
    ui->uiMD5->setText(qstrHash);
}

void Md5Calculator::onSHA1HashReceived(QString qstrHash)
{
    ui->uiSHA1->setText(qstrHash);
}

Md5Calculator::~Md5Calculator()
{
    delete ui;

    if (m_pWorkThread != nullptr)
        m_pWorkThread->exit();
}

//**************************************************//
//******************* WORK THREAD ******************//
//**************************************************//

WorkThread::WorkThread(QObject* pParent, const QString& qstrFile) :
    QThread(pParent),
    m_qstrFile(qstrFile)
{
    m_pOwner = static_cast<Md5Calculator*>(pParent);
}

void WorkThread::run()
{
    QFile file(m_qstrFile);

    if(!file.open(QFile::ReadOnly))
    {
        emit dialogPopup(ERROR_FILE);
        return;
    }

    // Generate MD5 and SHA1 hash
    MD5 MD5Object;
    CSHA1 sha1;

    // old method to generate MD5 hash : not suitable for big files...
    //QByteArray packet = file.readAll();
    //MD5Object.update(packet.data(),packet.size());
    //MD5Object.finalize();

    // to avoid loading a huge file in the RAM in one shot
    char* pFileBuffer = new char[MD5_MAX_FILE_BUFFER];
    if (!pFileBuffer)
        return;

    const size_t uFileSize = file.size();
    size_t uBytesProcessed = 0;
    while (!QThread::currentThread()->isInterruptionRequested())
    {
        const size_t uRead = file.read(pFileBuffer, MD5_MAX_FILE_BUFFER);

        if(uRead > 0)
        {
            MD5Object.update(pFileBuffer, uRead);
            sha1.Update(reinterpret_cast<const unsigned char*>(pFileBuffer), uRead);
        }

        // Update progress bar...
        uBytesProcessed += uRead;
        emit HashProgressChanged(static_cast<double>(uBytesProcessed)/uFileSize);

        if(uRead == 0)
            break;
    }
    delete[] pFileBuffer;

    if (!QThread::currentThread()->isInterruptionRequested())
    {
        // Finalize both hashes
        MD5Object.finalize();
        sha1.Final();

        // Emit to the GUI computed MD5 hash
        std::string strHash = MD5Object.hexdigest();
        emit MD5Hash(QString::fromStdString(strHash).toUpper());

        // Emit to the GUI computed SHA1 hash
        strHash.clear();
        sha1.ReportHashStl(strHash, CSHA1::REPORT_HEX_SHORT);
        emit SHA1Hash(QString::fromLocal8Bit(strHash.c_str()).toUpper());
    }

    file.close();
}
