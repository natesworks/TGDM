#ifndef LOGIN_MANAGER_H
#define LOGIN_MANAGER_H

#include <string>
#include <QStringList>

using namespace std;

extern const char* input;

QStringList get_sessions(std::string xdgSessionDir);
string get_session_exec(const string session, const string xdgSessionDir);

int pam_conv_func(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr);

bool pam_authenticate(const char *username);

void login_user_shell(const std::string& username);

bool executeCommand(string file);

#endif // LOGIN_MANAGER_H
