#include "../header/traceroute.h"

/*
 * Функция подсчета контрольной суммы ICMP пакета
 *
 * @param   packet  Указатель на начало пакета
 * @retval  Конрольная сумма
 */
u_short icmp_checksum(u_char *packet)
{
    int     length; // Длина ICMP пакета
    u_short fbyte;  // Первый байт 16ти битного блока
    u_short sbyte;  // Второй байт 16ти битного блока
    int     chsum;  // Контрольная сумма

    length = sizeof(struct icmp_echo);
    chsum = 0;
    /*
    * Считаем контрольную сумму, разворачивая по 2 байта. Обязательно до 
    * этого поле контрольной суммы в заголовке должно быть обнулено
    */
    for(int i = 0; i < length; i += 2){
        fbyte = packet[i];
        sbyte = packet[i + 1];
        chsum += (fbyte<<8)|sbyte;
    }

    // Приводим к необходимому виду
    chsum = chsum + (chsum>>16);
    return (u_short)~chsum;
}


/*
 * Формирует ICMP пакет для эхо запроса / ответа
 *
 * @param packet    Указатель на пакет, в который будет сформирован
 * @param type      Тип пакета
 * @param code      Код
 */
void icmp_build(struct icmp_echo *packet, u_char type, u_char code)
{
    packet->type = type;
    packet->code = code;
    packet->id = htons(rand()%256);
    packet->nseq = htons(rand()%256);
    gettimeofday(&(packet->time), NULL);
    // Необходимо обнулить контрольную сумму для ее верного подсчета
    packet->chsum = 0;
    packet->chsum = htons(icmp_checksum((u_char *)packet));
}


/*
 * Принимает ICMP пакет
 *
 * @param sock  Сокет, на котором принимаем
 * @param ip    Будет записан адрес промежуточного узла от которго пришел пакет
 *
 * @retval -1   Истекло время ожидания ответа
 * @retval 0    Пришел ответ от конечного узла 
 * @retval 1    Пришел ответ от промежуточного (закончился TTL) 
*/
int icmp_recv(int sock, char *ipto, char *ipfrom)
{
    u_char              packet[MAX_SPACK];  // Перехваченный пакет
    int                 length;             // Длина перехваченной части
    struct header_ipv4  *hdr_ip;            // Заголовок IP
    struct icmp_echo    *hdr_icmpe;         // ICMP reply от конечного узла
    struct icmp_timeexp *hdr_icmpt;         // ICMP, от промежуточного
    struct in_addr      tmp;                // Для коневертации IP в строку

    /*
     * Принимаем пакеты, пока не попадется нужный или не сработает таймаут.
     * Если поймается ненужный пакет, то повторяем попытку в цикле
     */
    while (1){
        length = recvfrom(sock, &packet, MAX_SPACK, 0, NULL, NULL);
        // Таймаут или ошибка
        if (length == -1)
            return -1;

        hdr_ip = (struct header_ipv4 *)&packet;
        // Проверяем является ли это ответом конечного узла
        if (hdr_ip->sip == inet_addr(ipto))
            return 0;
        /*
         * Проверяем является ли это сообщением о истечении ttl для меня, 
         * сместившись к полю типа на размер заголовка IP
         */
        if (packet[sizeof(struct header_ipv4)] == 11){
            hdr_icmpt = (struct icmp_timeexp *)(packet + 
                         sizeof(struct header_ipv4));
            // Проверяем, что это ответ на мой пакет
            if (hdr_icmpt->hip.dip == inet_addr(ipto)){
                tmp.s_addr = hdr_ip->sip;
                strcpy(ipfrom, inet_ntoa(tmp));
                return 1;
            }
        }
    }
    return -1;
}