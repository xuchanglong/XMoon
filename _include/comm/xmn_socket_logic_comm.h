#ifndef XMOON__INCLUDE_COMM_XMN_SOCKET_LOGIC_COMM_H_
#define XMOON__INCLUDE_COMM_XMN_SOCKET_LOGIC_COMM_H_

#define CMD_LOGIC_START 0
/**
 * 心跳包。
*/
#define CMD_LOGIC_PING (CMD_LOGIC_START + 0)
#define CMD_LOGIC_REGISTER (CMD_LOGIC_START + 5)
#define CMD_LOGIC_LOGIN (CMD_LOGIC_START + 6)

struct RegisterInfo
{
    int type;
    char username[56];
    char password[40];
} __attribute__((packed));

struct Logininfo
{
    char username[56];
    char password[40];
} __attribute__((packed));

#endif