#ifndef XMOON__INCLUDE_COMM_XMN_SOCKET_LOGIC_COMM_H_
#define XMOON__INCLUDE_COMM_XMN_SOCKET_LOGIC_COMM_H_

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