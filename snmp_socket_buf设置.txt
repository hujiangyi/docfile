1、定义变量
g_socket_buf_rec = 200000
g_socket_buf_rec_len=4

2、设置snmp socketbuf
setsockopt snmpSocket,0xffff,0x1002,&g_socket_buf_rec,g_socket_buf_rec_len

3、查询
g_socket_buf_rec = 0
getsockopt snmpSocket,0xffff,0x1002,&g_socket_buf_rec,&g_socket_buf_rec_len
g_socket_buf_rec