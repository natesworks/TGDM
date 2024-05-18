#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loginmanager.h"
#include <iostream>

using namespace std;

string username;
string password;
QStringList sessions;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sessions = get_sessions("/usr/share/wayland-sessions");
    ui->SessionSelector->addItems(sessions);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Login_clicked()
{
    username = ui->Username->toPlainText().toStdString();
    password = ui->Password->toPlainText().toStdString();
    input = password.c_str();
    if (pam_authenticate(username.c_str())) {
        cout << "Authentication successful!" << endl;
    } else {
        cout << "Authentication failed!" << endl;
    }
    const QString session = ui->SessionSelector->currentText();
    const string sessionStr = session.toStdString();
    const string sessionExec = get_session_exec(sessionStr, "/usr/share/wayland-sessions");
    cout << sessionStr;
    executeCommand(sessionExec);
}

