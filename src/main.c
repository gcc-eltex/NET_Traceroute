#include "../header/traceroute.h"

#define MAX_HOPE    32          // Максимальное количество узлов

int main(int argc, void *argv[])
{
    int                 sock;       // RAW Сокет
    struct sockaddr_in  addr;       // Данные назначения
    struct icmp_echo    packet;     // Сформированный для отправки ICMP пакет
    u_short             ttl;        // Текущий TTL
    struct timeval      time;       // Время ожидания прихода ответа
    char                ipfrom[16]; // IP промежуточного узла
    int                 reply;      // Тип ответа на посланый запрос
    
    // Проверяем параметры
    if (argc != 2){
        printf("Неверно указаны параметры\n");
        exit(-1);
    }
    if(inet_addr(argv[1]) == INADDR_NONE){
        printf("Некорректно задан адрес\n");
        exit(-1);
    }

    // Создаем RAW сокет
    sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1){
        perror("socket");
        exit(-1);
    }
    /*
     * Заполняем IP назначения, поскольку система сама добавит на наш ICMP 
     * необходимые заголовки. И задаем таймаут для получения
     */
    time.tv_sec = 2;
    time.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&time,sizeof(time)) == -1){
        perror("Не удалось настроить таймер");
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    /*
     * Отправляем эхо запросы, постепенно увеличивая TTL, и ждем ответ на мой 
     * пакет
     */
    for(int ttl = 1; ttl < MAX_HOPE; ttl++){
        setsockopt(sock, SOL_IP, IP_TTL, &ttl, sizeof(ttl));
        icmp_build(&packet, 8, 0);
        sendto(sock, (void *)&packet, sizeof(packet), 0, (struct sockaddr *)
               &addr, sizeof(addr));
        reply = icmp_recv(sock, (char *)argv[1], ipfrom);
        switch(reply){
            // Нет ответа
            case -1:
                printf("%2d  %s\n", ttl, "* * *");
            break;
            // Ответ от конечного узла. Изменим TTL чтобы выйти из цикла
            case 0:
                printf("%2d  %s\n", ttl, (char *)argv[1]);
                ttl = MAX_HOPE;
            break;
            // Ответ от промежуточного узла
            case 1:
                printf("%2d  %s\n", ttl, ipfrom);
            break;
        }
    }

    close(sock);
}