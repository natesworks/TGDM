#include "loginmanager.h"
#include <iostream>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <vector>
#include <filesystem>
#include <fstream>
#include <pwd.h>
#include <QStringList>
#include <wait.h>

using namespace std;
const char* input;

struct DesktopEnvironment
{
    string name;
    string exec;
};

QStringList get_sessions(string xdgSessionDir) {
    QStringList sessions;
    for (const auto& entry : filesystem::directory_iterator(xdgSessionDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".desktop") {
            ifstream desktopFile(entry.path());
            string line;
            string desktopEnvironment;
            bool found = false;

            while (getline(desktopFile, line)) {
                if (line.find("Name=") != string::npos) {
                    desktopEnvironment = line.substr(line.find("=") + 1);
                    found = true;
                    break;
                }
            }

            if (found) {
                sessions.append(QString::fromStdString(desktopEnvironment));
            }

            desktopFile.close();
        }
    }
    return sessions;
}


string get_session_exec(const string session, const string xdgSessionDir) {
    string execLine;
    bool sessionFound = false;
    for (const auto& entry : filesystem::directory_iterator(xdgSessionDir)) {
        while(sessionFound == false)
        {
            if (entry.is_regular_file() && entry.path().extension() == ".desktop") {
                ifstream desktopFile(entry.path());
                string line;
                string desktopEnvironment;

                while (getline(desktopFile, line)) {
                    if (line.find("Name=") != string::npos) {
                        desktopEnvironment = line.substr(line.find("=") + 1);
                        if (desktopEnvironment == session) {
                            sessionFound = true;
                        }
                    }
                }
                desktopFile.close();

                if (!execLine.empty()) {
                    break;
                }
            }
        }
    }
    for (const auto& entry : filesystem::directory_iterator(xdgSessionDir)) {
        {
            if (entry.is_regular_file() && entry.path().extension() == ".desktop") {
                ifstream desktopFile(entry.path());
                string line;
                string desktopEnvironment;

                while (getline(desktopFile, line)) {
                        if (line.find("Exec=") != string::npos) {
                            execLine = line.substr(line.find("=") + 1);
                            break;
                    }
                }
                desktopFile.close();

                if (!execLine.empty()) {
                    break;
                }
            }
        }
    }
    return execLine;
}


int pam_conv_func(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr) {
    *resp = (struct pam_response *)malloc(num_msg * sizeof(struct pam_response));
    if (!*resp) {
        return PAM_BUF_ERR;
    }

    (*resp)[0].resp = strdup(input);
    if (!(*resp)[0].resp) {
        free(*resp);
        return PAM_BUF_ERR;
    }

    return PAM_SUCCESS;
}

bool pam_authenticate(const char *username) {
    pam_handle_t *pamh;
    struct pam_conv conv = { pam_conv_func, NULL };
    int retval;

    retval = pam_start("login", username, &conv, &pamh);

    if (retval != PAM_SUCCESS) {
        return false;
    }

    retval = pam_authenticate(pamh, 0);
    if (retval == PAM_SUCCESS) {
        retval = pam_acct_mgmt(pamh, 0);
        if (retval == PAM_SUCCESS) {
            pam_end(pamh, retval);
            return true;
        }
    }

    pam_end(pamh, retval);
    return false;
}

int prepareuser(const string& username, string session)
{
    struct passwd *pw = getpwnam(username.c_str());
    if (!pw) {
        cerr << "User not found." << endl;
        return 1;
    }

    setuid(pw->pw_uid);

    chdir(pw->pw_dir);

    setenv("HOME", pw->pw_dir, 1);
    setenv("USER", pw->pw_name, 1);
    setenv("SHELL", pw->pw_shell, 1);

    executeCommand(session);

    return 0;
}

bool executeCommand(string file)
{
    istringstream iss(file);
    vector<char *> args;
    string arg;
    while (iss >> arg)
    {
        args.push_back(strdup(arg.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0)
    {
        setenv("DISPLAY", ":1", 1);

        if (execvp(args[0], const_cast<char *const *>(args.data())) == -1)
        {
            if (errno == ENOENT)
            {
                cerr << "Error: Command '" << file << "' not found." << endl;
                exit(1);
            }
        }
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        cout << "Failed to fork process." << endl;
    }
    for (auto &arg : args)
    {
        free(arg);
    }
    return 0;
}
