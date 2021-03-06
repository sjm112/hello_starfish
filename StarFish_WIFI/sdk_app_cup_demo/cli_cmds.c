#include <qcom/qcom_common.h>
#include <qcom/socket_api.h>
#include <qcom/select_api.h>
#include "threadx/tx_api.h"
#include "main.h"
#include "libcli.h"
#include "bb_httpclient.h"

#ifdef __GNUC__
#define UNUSED(d) d __attribute__ ((unused))
#else
#define UNUSED(d) d
#endif

#define TELNET_LISTEN_PORT (23)
#define TELNET_POOL_SIZE   (3*1024)
#define TELNET_STACK_SIZE  (2*1024)

extern SYS_CONFIG_t sys_config;
extern A_UINT32 mem_heap_get_free_size(void);
extern A_CHAR * _inet_ntoa(A_UINT32 ip);
extern A_UINT32 _inet_addr(A_CHAR *str);
extern A_INT32  CLIOTA_FWUpgrade(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc);
extern A_STATUS SMARTCONFIG_SetPromiscuousEnable(A_UINT8 enabled);

static int telnet_deamon_stop     = 0;
static int telnet_daemon_port     = 0;
static int telnet_firsttime_start = 1;

static TX_THREAD     telnet_thread;
static TX_BYTE_POOL  telnet_pool;
static CHAR         *telnet_thread_stack_pointer;

/* telnet listening socket */
static int socket_telnet;

static A_INT32 CLICMDS_Reset(struct cli_def *cli, UNUSED(const char *command), A_CHAR *argv[], A_INT32 argc)
{
    qcom_sys_reset();

    return CLI_OK;
}

static A_INT32 CLIOTA_PromiscEnable(struct cli_def *cli, const A_CHAR *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT8 enable;

    if (argc < 1) {
        cli_print(cli, "Usage: %s <1/0>", command);
        return -1;
    }

    enable = atoi(argv[0]);

    if(SMARTCONFIG_SetPromiscuousEnable(enable) != A_OK) {
        return -1;
    }
    else {
        return 0;
    }
}

static A_INT32 CLICMDS_Ping(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT32 hostaddr;
    A_UINT32 count, size, interval;
    A_UINT32 i;

    if (argc < 1) {
        CLI_PRINTF("Usage: %s <host_ip> [ -c <count> -s <size> -i <interval> ]", command);
        return -1;
    }

    hostaddr = _inet_addr(argv[0]);

    count    = 1;
    size     = 64;
    interval = 0;

    if (argc >= 1 && argc <= 7) {

        for (i = 1; i < argc; i += 2) {

            if (!A_STRCMP(argv[i], "-c")) {

                if ((i + 1) == argc) {
                    CLI_PRINTF("Missing parameter");
                    return -1;
                }

                A_SSCANF(argv[i + 1], "%u", &count);
            } 
            else if (!A_STRCMP(argv[i], "-s")) {

                if ((i + 1) == argc) {
                    CLI_PRINTF("Missing parameter");
                    return -1;
                }

                A_SSCANF(argv[i + 1], "%u", &size);
            } 
            else if (!A_STRCMP(argv[i], "-i")) {

                if ((i + 1) == argc) {
                    CLI_PRINTF("Missing parameter");
                    return -1;
                }

                A_SSCANF(argv[i + 1], "%u", &interval);
            }
        }
    } 
    else {
        CLI_PRINTF("Usage: %s <host> [ -c <count> -s <size> -i <interval> ]", command);
        return -1;
    }

    if (size > 7000) {          /*CFG_PACKET_SIZE_MAX_TX */
        CLI_PRINTF("Error: Invalid Parameter %s", argv[4]);

        return -1;
    }

    CLI_PRINTF("Pinging %s with %u bytes of data, count %u, interval %u", 
               argv[0], size, count, interval);

    if(qcom_ip_ping(hostaddr, size, count, interval) == A_OK) {
        CLI_PRINTF("ping OK");
    }
    else {
        CLI_PRINTF("ping Error");
    }

    return 0;
}

static A_INT32 CLICMDS_Version(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    CLI_PRINTF("Host version      : Hostless");
    CLI_PRINTF("Target version    : QCM");
    CLI_PRINTF("Firmware version  : %s", SW_VERSION);
    CLI_PRINTF("Interface version : EBS");
    CLI_PRINTF("Built on %s %s", __DATE__, __TIME__);

    return 0;
}

static A_INT32 CLICMDS_PowerMode(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    A_INT32 data;
    A_UINT32 wifiMode;

    if (argc < 1) {
        CLI_PRINTF("Usage: %s <0/1>", command);
        return -1;
    }

    A_SSCANF(argv[1], "%d", &data);

    if (data == 0) {
        qcom_power_set_mode(2);   //MAX_PERF_POWER
    } 
    else if (data == 1) {

        qcom_op_get_mode(&wifiMode);

        if (wifiMode == 1) {
            CLI_PRINTF("Setting REC Power is not allowed MODE_AP");
            return -1;
        }

        qcom_power_set_mode(1);   //REC_POWER
    } 
    else {
        CLI_PRINTF("Unknown power mode");
        return -1;
    }

    return 0; 
}

static A_INT32 CLICMDS_TxPowerMode(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{    
    A_INT32 data;

    if (argc != 1) {
        CLI_PRINTF("Usage: %s <1 - 17>", command);
        return -1;
    }

    A_SSCANF(argv[0], "%d", &data);

    qcom_set_tx_power((A_UINT32)data);

    return 0; 
}

static A_INT32 CLICMDS_SetChannel(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{    
    A_INT32 data;

    if (argc != 1) {
        CLI_PRINTF("Usage: %s <1 - 14>", command);
        return -1;
    }

    A_SSCANF(argv[0], "%d", &data);

    /* 2.4G */
    if (data < 27) {

        A_CHAR acountry_code[3];

        qcom_get_country_code(acountry_code);

        if (0 == A_STRCMP(acountry_code, "US")) {
            if ((data < 1) || (data > 11)) {
                CLI_PRINTF("wmiconfig --channel <1-11>");
                return -1;
            }
        } 
        else if (0 == A_STRCMP(acountry_code, "JP")) {
            if ((data < 1) || (data > 14)) {
                CLI_PRINTF("wmiconfig --channel <1-14>");
                return -1;
            }
        } 
        else {
            if ((data < 1) || (data > 13)) {
                CLI_PRINTF("wmiconfig --channel <1-13>");
                return -1;
            }
        }
    }
    /* 5G */
    else {
        CLI_PRINTF("Not support 5G channel.");
        return -1;
    }

    qcom_set_channel((A_UINT32)data); 

    return 0; 
}

static A_INT32 CLICMDS_PrintRssi(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT8 rssi;
    A_UINT8 chipState;
    QCOM_WLAN_DEV_MODE devMode;

    qcom_op_get_mode(&devMode);

    if(devMode == QCOM_WLAN_DEV_MODE_STATION){

        qcom_get_state(&chipState);

        if(chipState == 4){
            qcom_sta_get_rssi(&rssi);

            CLI_PRINTF("RSSI indicator = %d dB", rssi);
        }
        else{
            CLI_PRINTF("Not associate");
            return -1;
        }
    }
    else{
        CLI_PRINTF("RSSI not supported in AP mode");
        return -1;
    }

    return 0;
}

static A_INT32 CLICMDS_IpStatic(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT32 address;
    A_UINT32 submask;
    A_UINT32 gateway;

    if (argc < 3) {
        CLI_PRINTF("Usage:%s x.x.x.x(ip) x.x.x.x(msk) x.x.x.x(gw)", command);
        return -1;
    }

    address = _inet_addr(argv[0]);
    submask = _inet_addr(argv[1]);
    gateway = _inet_addr(argv[2]);

    qcom_ip_address_set(address, submask, gateway);

    return 0; 
}

static A_INT32 CLICMDS_IpDhcp(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    /* 1: enbale; 0:disable */
    qcom_dhcpc_enable(1);

    return 0; 
}

static A_INT32 CLICMDS_IpConfig(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT8 macAddr[6] = {0};
    A_UINT32 ipAddress;
    A_UINT32 submask;
    A_UINT32 gateway;
    A_UINT32 dns;

    qcom_mac_get((A_UINT8 *) &macAddr);
    qcom_ip_address_get(&ipAddress, &submask, &gateway);
    qcom_dns_server_address_get(&dns);

    CLI_PRINTF("MAC     : %x:%x:%x:%x:%x:%x",
            macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

    CLI_PRINTF("IP      : %d.%d.%d.%d", 
               (ipAddress) >> 24 & 0xFF, (ipAddress) >> 16 & 0xFF, (ipAddress) >> 8 & 0xFF, (ipAddress) & 0xFF);

    CLI_PRINTF("Mask    : %d.%d.%d.%d",
               (submask) >> 24 & 0xFF, (submask) >> 16 & 0xFF, (submask) >> 8 & 0xFF, (submask) & 0xFF);

    CLI_PRINTF("Gateway : %d.%d.%d.%d", 
               (gateway) >> 24 & 0xFF, (gateway) >> 16 & 0xFF, (gateway) >> 8 & 0xFF, (gateway) & 0xFF);

    CLI_PRINTF("Dns     : %d.%d.%d.%d",
               (dns) >> 24 & 0xFF, (dns) >> 16 & 0xFF, (dns) >> 8 & 0xFF, (dns) & 0xFF);

    return 0; 
}

static A_INT32 CLICMDS_IpGetHostByName(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT32 ipAddress;
    A_CHAR *pName;

    if (argc < 1) {
        CLI_PRINTF("Usage:%s <hostname>", command);
        return -1;
    }

    pName = argv[0];

    if (qcom_dnsc_get_host_by_name(pName, &ipAddress)== A_OK) {
        A_CHAR *ipaddr;

        ipaddr = _inet_ntoa(ipAddress);

        CLI_PRINTF("Get IP address of host %s", pName);
        CLI_PRINTF("ip address is %s", (char *)_inet_ntoa(ipAddress));
    } 
    else {
        CLI_PRINTF("The IP of host %s is not gotten", pName);
    }

    return 0; 
}

static A_INT32 CLICMDS_DnsClient(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    int flag;

    if (argc < 1) {
        CLI_PRINTF("Usage:%s start/stop", command);
        return -1;
    }

    if (!strcmp(argv[0], "start"))
        flag = 1;
    else if (!strcmp(argv[0], "stop"))
        flag = 0;
    else {
        CLI_PRINTF("Input paramenter should be start or stop !");
        return -1;
    }

    /* 1: enable; 0:diable; */
    qcom_dnsc_enable(flag);

    return 0; 
}

static A_INT32 CLICMDS_IpDnsServerAddr(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT32 ip = 0;

    if (argc < 1) {
        CLI_PRINTF("Usage:%s <xx.xx.xx.xx>", command);
        return -1;
    }

    ip = _inet_addr(argv[0]);

    if (0 == ip) {
        CLI_PRINTF("Input ip addr is not valid!");
        return -1;
    }

    qcom_dnsc_add_server_address(ip);

    return 0; 
}

static A_INT32 CLICMDS_IpDelDnsServerAddr(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT32 ip = 0;

    if (argc < 1) {
        CLI_PRINTF("Usage:%s <xx.xx.xx.xx>", command);
        return -1;
    }

    ip = _inet_addr(argv[0]);

    if (0 == ip) {
        CLI_PRINTF("Input ip addr is not valid!");
        return -1;
    }

    qcom_dnsc_del_server_address(ip);

    return 0; 
}

static A_INT32 CLICMDS_IpDhcpPool(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    A_UINT32 leasetime = 0;
    A_UINT32 startaddr, endaddr;

    if (argc < 3) {
        CLI_PRINTF("Usage:%s <Start ipaddr> <End ipaddr> <Lease time>", command);
        return -1;
    }

    startaddr = _inet_addr(argv[0]);
    endaddr   = _inet_addr(argv[1]);
    leasetime = atoi(argv[2]);

    qcom_dhcps_set_pool(startaddr, endaddr, leasetime);

    return 0; 
}

static A_INT32 CLICMDS_MemFree(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    CLI_PRINTF("MEM FREE : %d", mem_heap_get_free_size());

    return 0; 
}

void CLICMDS_STAConnect2(A_CHAR *pSSID, A_CHAR *pKey)
{
    strcpy(sys_config.staSSID, pSSID);
    strcpy(sys_config.staKey,  pKey);

    sys_config.mode  = eStation;
    sys_config.crc16 = MAIN_CalcCRC16((A_UINT8 *)&sys_config,
                                     sizeof(SYS_CONFIG_t) - sizeof(A_UINT16));

    if(NVRAM_SaveSettings(&sys_config) == A_OK) {
        qcom_sys_reset();
    }
}

static A_INT32 CLICMDS_STAConnect(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    if (argc < 2) {
        CLI_PRINTF("Usage:%s <SSID> <Password>", command);
        return -1;
    }

    CLICMDS_STAConnect2(argv[0], argv[1]);

    return 0; 
}

A_INT32 CLICMDS_HttpGet(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    if (argc < 1) {
        CLI_PRINTF("Usage:%s <url>", command);
        return -1;
    }

    BB_HTTP_RESPONSE *hr = http_get(argv[0], 0);

    if (hr != NULL) {
        http_response_free(hr); 
    }

    return 0; 
}

A_INT32 CLICMDS_HttpPost(struct cli_def *cli, const char *command, A_CHAR *argv[], A_INT32 argc)
{
    if (argc < 1) {
        CLI_PRINTF("Usage:%s <url>", command);
        return -1;
    }

    BB_HTTP_RESPONSE *hr = http_post(argv[0], 0,argv[1]);

    if (hr != NULL) {
        http_response_free(hr); 
    }

    return 0; 
}

static void CLICMDS_RegisterCommands(struct cli_def *cli)
{  
    struct cli_command *pCmd;

    cli_register_command(cli, NULL, "reset",            CLICMDS_Reset,              PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Reset the board");
    cli_register_command(cli, NULL, "ota_fw_upgrade",   CLIOTA_FWUpgrade,           PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "ota_fw_upgrade <OTA-server-ip> <image-name>");
    cli_register_command(cli, NULL, "ping",             CLICMDS_Ping,               PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "ping");
    cli_register_command(cli, NULL, "promisc",          CLIOTA_PromiscEnable,       PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Enable/Disable wlan promiscurous");
    cli_register_command(cli, NULL, "version",          CLICMDS_Version,            PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Displays versions");
    cli_register_command(cli, NULL, "rssi",             CLICMDS_PrintRssi,          PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Prints Link Quality (SNR)");
    cli_register_command(cli, NULL, "channel",          CLICMDS_SetChannel,         PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Set channel hint 1-13");
    cli_register_command(cli, NULL, "connect",          CLICMDS_STAConnect,         PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Connect to the AP");
    cli_register_command(cli, NULL, "http_get",         CLICMDS_HttpGet,            PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Http Gets url");
	cli_register_command(cli, NULL, "http_post",         CLICMDS_HttpPost,            PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Http Post url");
    pCmd = cli_register_command(cli, NULL, "memory",    NULL,     PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Memory info");
    cli_register_command(cli, pCmd, "free",             CLICMDS_MemFree,           PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Show the free memory");

    // Power commands
    pCmd = cli_register_command(cli, NULL, "power",     NULL,     PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Power configuration");
    cli_register_command(cli, pCmd, "mode",             CLICMDS_PowerMode,          PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Set power mode 1=Power save, 0= Max Perf");
    cli_register_command(cli, pCmd, "tx",               CLICMDS_TxPowerMode,        PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Set transmit power 1-17 dbM");

    // IP commands
    pCmd = cli_register_command(cli, NULL, "ip",        NULL,     PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Ip configuration");
    cli_register_command(cli, pCmd, "static",           CLICMDS_IpStatic,           PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Set static IP parameters");
    cli_register_command(cli, pCmd, "dhcp",             CLICMDS_IpDhcp,             PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Run DHCP client");
    cli_register_command(cli, pCmd, "config",           CLICMDS_IpConfig,           PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Show IP parameters");
    cli_register_command(cli, pCmd, "resolve_hostname", CLICMDS_IpGetHostByName,    PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Resolve hostname for domain_type");
    cli_register_command(cli, pCmd, "dns_client",       CLICMDS_DnsClient,          PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Start/Stop the DNS Client");
    cli_register_command(cli, pCmd, "dns_server",       CLICMDS_IpDnsServerAddr,    PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Address of the DNS Server");
    cli_register_command(cli, pCmd, "del_dns_server",   CLICMDS_IpDelDnsServerAddr, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Address of the DNS Server to be deleted");
    cli_register_command(cli, pCmd, "dhcp_pool",        CLICMDS_IpDhcpPool,         PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Set ip dhcp pool");
}

void CLICMDS_TelnetRequestDaemon(unsigned long which_threa)
{
    int ret = 0;
    struct sockaddr_in srv_addr;
    int socket_client = -1;
    struct sockaddr_in client_addr;

    int len = 0;
    q_fd_set fd_sockSet;
    int client_ip_addr;
    int client_l4_port;

    struct cli_def *cli;

    A_PRINTF("Telent deamon task start \n");

    // Init the cli
    cli = cli_init();

    cli_set_hostname(cli, "CMD");
    cli_regular_interval(cli, 5);

    CLICMDS_RegisterCommands(cli);

    /* setup socket */
    socket_telnet = qcom_socket(AF_INET, SOCK_STREAM, 0);
    if (socket_telnet < 0) {
        return;
    }

    telnet_daemon_port = TELNET_LISTEN_PORT;

    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port        = htons(telnet_daemon_port);
    srv_addr.sin_family      = AF_INET;

    /* bind the socket */
    ret = qcom_bind(socket_telnet, (struct sockaddr *) &srv_addr, sizeof (struct sockaddr_in));
    if (ret < 0) {
        A_PRINTF("qcom_bind fail, ret = %d!\n", ret);
        goto EXIT;
    }

    /* start listening */
    ret = qcom_listen(socket_telnet, 10);
    if (ret < 0) {
        A_PRINTF("qcom_listen fail, ret = %d!\n", ret);
        goto EXIT;
    }

    A_PRINTF("Telnet request deamon is listenning on port: %d ...\n", telnet_daemon_port);

    // wait for somebody to connect me
    for (;;) {

        socket_client = qcom_accept(socket_telnet, (struct sockaddr *) &client_addr, &len);

        if (socket_client < 0) {
            if (telnet_deamon_stop) {
                goto EXIT;
            }

            else {
                A_PRINTF("qcom_accept fail, ret = %d!\n", socket_client);
                continue;
            }
        }

        client_ip_addr = ntohl(client_addr.sin_addr.s_addr);
        client_l4_port = ntohs(client_addr.sin_port);

        A_PRINTF("Accept connection from %d.%d.%d.%d:%d\n",
                 (client_ip_addr) >> 24 & 0xFF,
                 (client_ip_addr) >> 16 & 0xFF,
                 (client_ip_addr) >> 8 & 0xFF,
                 (client_ip_addr)&0xFF,
                 client_l4_port); 

        cli_loop(cli, socket_client);

        A_PRINTF("telnet session end\n");
    }

EXIT:
    A_PRINTF("telnet server is turning down\n");
    FD_ZERO(&fd_sockSet); 
    tx_thread_terminate(&telnet_thread);

    return;
}

void CLICMDS_StartTelnetDaemon(void)
{
	A_INT32 i;

	if(telnet_firsttime_start)
	{
        tx_byte_pool_create(&telnet_pool,
                            "telnet pool",
                            TX_POOL_CREATE_DYNAMIC,
                            TELNET_POOL_SIZE);

		tx_byte_allocate(&telnet_pool, 
                         (VOID **)&telnet_thread_stack_pointer, 
                         TELNET_STACK_SIZE, 
                         TX_NO_WAIT);

		telnet_firsttime_start = 0;
	}
	else
	{
		
        tx_thread_delete(&telnet_thread);
	}

    tx_thread_create(&telnet_thread, 
                     "telnet thread", 
                     CLICMDS_TelnetRequestDaemon,
                     i, 
                     telnet_thread_stack_pointer, 
                     TELNET_STACK_SIZE, 
                     16, 
                     16, 
                     4, 
                     TX_AUTO_START);
}

A_INT32 CLICMDS_StopTelnetDaemon(void)
{
    telnet_deamon_stop = 1;
    qcom_close(socket_telnet);

    return 0;
}
