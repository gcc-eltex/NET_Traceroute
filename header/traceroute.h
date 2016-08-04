#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_SPACK   4096    // Максимальный размер пакета

/*
 * Структура, описывающая заголовок IP пакета.
 * При отправке решено не добавлять опций, поэтому они отсутствуют в 
 * описанной структуре, благодаря чему имеем фиксированный размер заголовка
 */ 
struct header_ipv4
{
    u_char  ver_len;    // 4 бита - версия, 4 - длина заголовка в октетах
    u_char  dscp_ecn;   // 6 бит - тип обслуживания , 2 - перегрузка
    short   length;     // Длина пакета в байтах (заголовок + данные)
    short   ident;      // Идентификатор фрагмента
    short   flag_offset;// 3 бита флаги, остальное - смещение фрагмента
    u_char  ttl;        // Время жизни пакета
    u_char  proto;      // Тип инкапсулированного протокола
    short   chsum;      // Контрольная сумма заголовка
    int     sip;        // IP адрес источника
    int     dip;        // IP адрес назначения
};

/*
 * Структура, описывающая формат ICMP пакета для посылки и приема echo reply и 
 * echo recive
 */
struct icmp_echo
{
    u_char          type;   // Тип сообщения
    u_char          code;   // Код сообщения
    u_short         chsum;  // Контрольная сумма
    u_short         id;     // Идентификатор
    u_short         nseq;   // Номер последовательности
    struct timeval  time;   // Временная метка
};

/*
 * Структура, описывающая формат ICMP пакета, получаемого от узла при
 * истечении TTL
 */
struct icmp_timeexp
{
    u_char              type;   // Тип сообщения
    u_char              code;   // Код сообщения
    u_short             chsum;  // Контрольная сумма
    u_short             zero;   // ХЗ, без этого работать не будет
    struct header_ipv4  hip;    // Заголовок пакета, TTL которого закончился
};

/*
 * Модуль icmp.c
 *
 * Предназначен для манипуляций с ICMP
 */
int icmp_recv(int sock, char *ipto, char *ipfrom);
u_short icmp_checksum(u_char *packet);
void icmp_build(struct icmp_echo *header, u_char type, u_char code);