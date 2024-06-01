#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loginmanager.h"
#include <iostream>

using namespace std;

string username;
string password;
QStringList waylandSessions;
QStringList xorgSessions;
QStringList sessions;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    waylandSessions = get_sessions("/usr/share/wayland-sessions");
    ui->SessionSelector->addItems(waylandSessions);
    xorgSessions = get_sessions("/usr/share/xsessions");
    ui->SessionSelector->addItems(xorgSessions);
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
    const string sessionExec = get_session_exec(session.toStdString(), "/usr/share/wayland-sessions");
    prepareuser(username, sessionExec);
}

