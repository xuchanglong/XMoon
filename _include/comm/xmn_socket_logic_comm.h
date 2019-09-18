#ifndef XMOON__INCLUDE_COMM_XMN_SOCKET_LOGIC_COMM_H_
#define XMOON__INCLUDE_COMM_XMN_SOCKET_LOGIC_COMM_H_

#define CMD_LOGIC_START 0
#define CMD_LOGIC_REGISTER (CMD_LOGIC_START + 5)
#define CMD_LOGIC_LOGIN (CMD_LOGIC_START + 6)

#pragma pack(1)

struct RegisterInfo
{
    int type;
    char username[56];
    char password[40];
};

struct Logininfo
{
    char username[56];
    char password[40];
};

#pragma pack()

#endif