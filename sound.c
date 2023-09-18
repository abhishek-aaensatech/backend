#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <unistd.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#define MAX_CONNECTION 10
#define PORT 8080

char *all_possible_ips[MAX_CONNECTION + 1] = {"192.168.5.10", "192.168.5.11", "192.168.5.12", "192.168.5.13", "192.168.5.14", "192.168.5.15", "192.168.5.16", "192.168.5.17", "192.168.5.18", "192.168.5.19", "192.168.5.20"};
char *connected_optimizers_IPs[MAX_CONNECTION];
char *connected_optimizers[MAX_CONNECTION];

char *latest_data[MAX_CONNECTION];
char *database_data[MAX_CONNECTION];
char IP_OptimizerIDs_map[MAX_CONNECTION][20];

char *optimizerID_for_sending_into_getProperperties;
char *properties_for_saving_in_latest_data;
char *current_date;
char *current_time;

char GatewayID[] = "NGCS2023011001";

int cnt = 0;
int is_by_pass_mode = 0;
int current_date_len = 12;
int current_time_len = 10;

pthread_mutex_t mutex;

int request_complete = 0;

/**************************************************METER RELATED CODE START*********************************************************/

unsigned char voltage[] = {0x01, 0x03, 0x00, 0x63, 0x00, 0x02, 0x34, 0x15};
unsigned char current[] = {0x01, 0x03, 0x00, 0x71, 0x00, 0x02, 0x94, 0x10};
// unsigned char frequency[] = {0x01,0x03,0x00,0xAB,0x00,0x02,0xB5,0xEB};
unsigned char powerfactor[] = {0x01, 0x03, 0x00, 0x85, 0x00, 0x02, 0xD5, 0xE2};
unsigned char activepower[] = {0x01, 0x03, 0x00, 0x8D, 0x00, 0x02, 0x54, 0x20};
// unsigned char phasepower[] = {0x01,0x03,0x00,0x95,0x00,0x02,0xD4,0x27};
unsigned char apparentpower[] = {0x01, 0x03, 0x00, 0x9D, 0x00, 0x02, 0x55, 0xE5};

unsigned char voltage2[] = {0x01, 0x03, 0x00, 0x65, 0x00, 0x02, 0xD4, 0x14};
unsigned char current2[] = {0x01, 0x03, 0x00, 0x73, 0x00, 0x02, 0x35, 0xD0};
// unsigned char frequency2[] = {0x01,0x03,0x00,0xAB,0x00,0x02,0xB5,0xEB};
unsigned char powerfactor2[] = {0x01, 0x03, 0x00, 0x87, 0x00, 0x02, 0x74, 0x22};
unsigned char activepower2[] = {0x01, 0x03, 0x00, 0x8F, 0x00, 0x02, 0xF5, 0xE0};
// unsigned char phasepower2[] = {0x01,0x03,0x00,0x97,0x00,0x02,0x75,0xE7};
unsigned char apparentpower2[] = {0x01, 0x03, 0x00, 0x9F, 0x00, 0x02, 0xF4, 0x25};

unsigned char voltage3[] = {0x01, 0x03, 0x00, 0x67, 0x00, 0x02, 0x75, 0xD4};
unsigned char current3[] = {0x01, 0x03, 0x00, 0x75, 0x00, 0x02, 0xD5, 0xD1};
// unsigned char frequency3[] = {0x01,0x03,0x00,0xAB,0x00,0x02,0xB5,0xEB};
unsigned char powerfactor3[] = {0x01, 0x03, 0x00, 0x89, 0x00, 0x02, 0x15, 0xE1};
unsigned char activepower3[] = {0x01, 0x03, 0x00, 0x91, 0x00, 0x02, 0x95, 0xE6};
// unsigned char phasepower3[] = {0x01,0x03,0x00,0x99,0x00,0x02,0x14,0x24};
unsigned char apparentpower3[] = {0x01, 0x03, 0x00, 0xA1, 0x00, 0x02, 0x95, 0xE9};

float volt_value, curr_value, powf_value, act_value, app_value = 0;
float volt_value2, curr_value2, powf_value2, act_value2, app_value2 = 0;
float volt_value3, curr_value3, powf_value3, act_value3, app_value3 = 0;

int serial_port = 0;
int uart_check = 0;

int setup_communication()
{
    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
    system("chmod +777 /dev/ttyHSL1");

    serial_port = open("/dev/ttyHSL1", O_RDWR);

    if (serial_port == -1)
    {
        uart_check = 0;
        return 0;
    }
    uart_check = 1;
    struct termios tty;

    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 0;
    }

    tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
    tty.c_cflag |= CS8;            // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;                                                        // Disable echo
    tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
    tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return -1;
    }
}

float getvalue(char *string)
{
    int num;
    float f;
    sscanf(string, "%x", &num);
    f = *((float *)&num);
    return f;
}

float read_response()
{
    int i = 0;
    unsigned char string[10];
    float value = 0;
    unsigned char read_buf[1024];

    memset(&read_buf, '\0', sizeof(read_buf));

    int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));

    if (num_bytes < 0)
    {
        printf("Error reading: %s\n", strerror(errno));
        return 1;
    }

    sprintf(string, "%02x%02x%02x%02x", read_buf[5], read_buf[6], read_buf[3], read_buf[4]);

    value = getvalue(string);

    return value;
}

void phase1Values(void)
{
    write(serial_port, voltage, sizeof(voltage));
    volt_value = read_response();

    write(serial_port, current, sizeof(current));
    curr_value = read_response();

    // write(serial_port,frequency, sizeof(frequency));
    // freq_value=read_response();

    write(serial_port, activepower, sizeof(activepower));
    act_value = read_response();

    write(serial_port, powerfactor, sizeof(powerfactor));
    powf_value = read_response();

    // write(serial_port,phasepower, sizeof(phasepower));
    // phas_value=read_response();

    write(serial_port, apparentpower, sizeof(apparentpower));
    app_value = read_response();
}

void phase2Values(void)
{
    write(serial_port, voltage2, sizeof(voltage));
    volt_value2 = read_response();

    write(serial_port, current2, sizeof(current));
    curr_value2 = read_response();

    // write(serial_port,frequency2, sizeof(frequency));
    // freq_value2=read_response();

    write(serial_port, activepower2, sizeof(activepower));
    act_value2 = read_response();

    write(serial_port, powerfactor2, sizeof(powerfactor));
    powf_value2 = read_response();

    // write(serial_port,phasepower2, sizeof(phasepower));
    // phas_value2=read_response();

    write(serial_port, apparentpower2, sizeof(apparentpower));
    app_value2 = read_response();
}

void phase3Values(void)
{
    write(serial_port, voltage3, sizeof(voltage));
    volt_value3 = read_response();

    write(serial_port, current3, sizeof(current));
    curr_value3 = read_response();

    // write(serial_port,frequency3, sizeof(frequency));
    // freq_value3=read_response();

    write(serial_port, activepower3, sizeof(activepower));
    act_value3 = read_response();

    write(serial_port, powerfactor3, sizeof(powerfactor));
    powf_value3 = read_response();

    // write(serial_port,phasepower3, sizeof(phasepower));
    // phas_value3=read_response();

    write(serial_port, apparentpower3, sizeof(apparentpower));
    app_value3 = read_response();
}

/**************************************************METER RELATED CODE END*********************************************************/

// Time and Date Function
void getDateTime()
{
    current_date = realloc(current_date, current_date_len);
    current_time = realloc(current_time, current_time_len);
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(current_date, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    sprintf(current_time, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

// Utility function to concate two string

char *findSubstring(char *haystack, char *needle)
{
    char *result = strstr(haystack, needle);
    return result;
}

/**************************************************THREAD-1 RELATED CODE START*********************************************************/

size_t write_callback_for_getOptimizerID(void *contents, size_t size, size_t nmemb, void *userp)
{
    int content_length = strlen((char *)(contents)) + 1;
    optimizerID_for_sending_into_getProperperties = (char *)realloc(optimizerID_for_sending_into_getProperperties, content_length);
    strcpy(optimizerID_for_sending_into_getProperperties, (char *)contents);
    return size * nmemb;
}

void send_post_request_for_getOptimizerID(const char *url, const char *data)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl == NULL)
    {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_for_getOptimizerID);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 2000);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Failed to perform O-POST request:%s %s\n", url, curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
}

size_t write_callback_for_getProperty(void *contents, size_t size, size_t nmemb, void *userp)
{
    int content_length = strlen((char *)(contents)) + 1;
    properties_for_saving_in_latest_data = (char *)realloc(properties_for_saving_in_latest_data, content_length);
    strcpy(properties_for_saving_in_latest_data, (char *)contents);

    printf("%d\n", request_complete++);
    return size * nmemb;
}

void send_post_request_for_getProperty(const char *url, const char *data)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl == NULL)
    {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_for_getProperty);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 2000);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Failed to perform P-POST request:%s %s\n", url, curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
}

void query_function(char *ip_address, int idx)
{
    char url[50] = {0};
    sprintf(url, "%s%s%s%s", "http://", ip_address, ":8080", "/getOptimizerID");

    char post_data[] = "{\"Hello\":\"Optimizer\"}";

    send_post_request_for_getOptimizerID(url, post_data);

    if (optimizerID_for_sending_into_getProperperties != NULL)
    {

        char *position = optimizerID_for_sending_into_getProperperties;

        if ((position = findSubstring(position, "DeviceID")) != NULL)
        {
            char *start = strchr(position, ' ');
            int len = strlen(start);
            for (int i = 2; i < len; i++)
            {
                if (start[i] == '\"')
                {
                    break;
                }
                else
                {
                    IP_OptimizerIDs_map[idx][i - 2] = start[i];
                }
            }
            // printf("%s", IP_OptimizerIDs_map[idx]);
        }

        sprintf(url, "%s%s%s%s", "http://", ip_address, ":8080", "/getProperties");
        send_post_request_for_getProperty(url, optimizerID_for_sending_into_getProperperties);
    }
    if (is_by_pass_mode)
    {
        sprintf(url, "%s%s%s%s", "http://", ip_address, ":8080", "/getStatus");
        send_post_request_for_getProperty(url, post_data);
    }
}

void *optimizer_query_function(void *arg)
{
    while (1)
    {
        for (int i = 0; i < MAX_CONNECTION; i++)
        {
            pthread_mutex_lock(&mutex);
            if (latest_data[i] != NULL)
            {
                latest_data[i] = (char *)realloc(latest_data[i], 0);
            }
            if (optimizerID_for_sending_into_getProperperties != NULL)
            {
                optimizerID_for_sending_into_getProperperties = (char *)realloc(optimizerID_for_sending_into_getProperperties, 0);
            }
            if (properties_for_saving_in_latest_data != NULL)
            {
                properties_for_saving_in_latest_data = (char *)realloc(properties_for_saving_in_latest_data, 0);
            }
            if (connected_optimizers[i] != NULL)
            {
                query_function(connected_optimizers[i], i);
                if (properties_for_saving_in_latest_data != NULL)
                {
                    int content_length = strlen(properties_for_saving_in_latest_data) + 1;
                    latest_data[i] = (char *)realloc(latest_data[i], content_length);
                    strcpy(latest_data[i], properties_for_saving_in_latest_data);
                }
            }
            pthread_mutex_unlock(&mutex);
            usleep(400);
        }

        sleep(3);
    }
    pthread_exit(NULL);
}

/**************************************************THREAD-1 RELATED CODE END*********************************************************/

/**************************************************THREAD-2 RELATED CODE START*********************************************************/

void send_data_to_server_nodejs_using_system(char *data)
{

    char curlbuffer[7000];
    sprintf(curlbuffer, "curl -H 'Content-Type: application/json' -d '%s' -X POST  http://44.202.86.124:5000/sendNewDetailsFromGateway", data);
    // sprintf(curlbuffer, "curl -H 'Content-Type: application/json' -d '%s' -X POST  http://192.168.5.17:4000/data", data);
    // sprintf(curlbuffer, "curl -H 'Content-Type: application/json' -d '%s' -X POST  http://localhost:4000/data", data);
    system(curlbuffer);
}

void save_data_to_local_file(char *data)
{
    char *date = (char *)malloc(current_date_len);
    sprintf(date, "%s%s", current_date, ".json");
    FILE *file_fd = fopen(date, "a");
    if (file_fd == NULL)
    {
        printf("File Opening Fail");
        free(date);
        return;
    }
    size_t num_elements = strlen(data);
    fwrite(data, sizeof(data[0]), num_elements, file_fd);
    fwrite("\n", sizeof(data[0]), 1, file_fd);
    free(date);
    fclose(file_fd);
}

void *send_data_to_server_function(void *arg)
{
    char json[3000];
    char newJson[5000];
    char data_to_send[5000];
    char meterDetails[3000];
    while (1)
    {
        setup_communication();
        if (uart_check == 1)
        {
            phase1Values();
            usleep(1);
            phase2Values();
            usleep(1);
            phase3Values();
            close(serial_port);
        }
        getDateTime();
        memset(json, 0, sizeof(json));
        memset(newJson, 0, sizeof(newJson));
        memset(data_to_send, 0, sizeof(data_to_send));
        memset(meterDetails, 0, sizeof(meterDetails));
        for (int i = 0; i < MAX_CONNECTION; i++)
        {
            pthread_mutex_lock(&mutex);
            if (latest_data[i] != NULL)
            {
                if (json[0] == '\0')
                {
                    sprintf(json, "%s", latest_data[i]);
                }
                else
                {
                    sprintf(json, "%s,%s", json, latest_data[i]);
                }
                database_data[i] = realloc(database_data[i], strlen(latest_data[i]) + 1);
                strcpy(database_data[i], latest_data[i]);
            }
            else if (connected_optimizers[i] != NULL && database_data[i] != NULL)
            {
                if (json[0] == '\0')
                {
                    sprintf(json, "%s", database_data[i]);
                }
                else
                {
                    sprintf(json, "%s,%s", json, database_data[i]);
                }
            }
            pthread_mutex_unlock(&mutex);
        }

        sprintf(newJson, "\"data\":[\n%s\n]", json);

        sprintf(meterDetails, "\"meterDetails\":{\"GatewayID\":\"%s\",\"Date\":\"%s\",\"Time\":\"%s\",\"Ph1Voltage(V)\":\"%f\",\"Ph1Current(A)\":\"%f\",\"Ph1ActivePower(kW)\":\"%f\",\"Ph1PowerFactor\":\"%f\",\"Ph1ApparentPower(kVA)\":\"%f\",\"Ph2Voltage(V)\":\"%f\",\"Ph2Current(A)\":\"%f\",\"Ph2ActivePower(kW)\":\"%f\",\"Ph2PowerFactor\":\"%f\",\"Ph2ApparentPower(kVA)\":\"%f\",\"Ph3Voltage(V)\":\"%f\",\"Ph3Current(A)\":\"%f\",\"Ph3ActivePower(kW)\":\"%f\",\"Ph3PowerFactor\":\"%f\",\"Ph3ApparentPower(kVA)\":\"%f\"}", GatewayID, current_date, current_time, volt_value, curr_value, powf_value, act_value, app_value, volt_value2, curr_value2, powf_value2, act_value2, app_value2, volt_value3, curr_value3, powf_value3, act_value3, app_value3);

        sprintf(data_to_send, "{%s,%s}", newJson, meterDetails);

        send_data_to_server_nodejs_using_system(data_to_send);
        // save_data_to_local_file(data_to_send);
        sleep(5);
    }
}

/**************************************************THREAD-2 RELATED CODE END*********************************************************/

/**************************************************THREAD-3 RELATED CODE START*********************************************************/

void *update_connected_optimizer_function()
{
    char command[100];
    int try_cnt = 0, cur_cnt = 0, prev_cnt = 0;
    while (1)
    {
        if (!try_cnt)
        {
            for (int i = 0; i < MAX_CONNECTION; i++)
            {
                if (connected_optimizers_IPs[i] != NULL)
                {
                    connected_optimizers_IPs[i] = (char *)realloc(connected_optimizers_IPs[i], 0);
                    prev_cnt++;
                }
            }
        }
        for (int i = 0, j = 0; j < (MAX_CONNECTION) && i < (MAX_CONNECTION + 1); i++)
        {
            memset(command, 0, sizeof(command));
            snprintf(command, sizeof(command), "ping -c 1 -W 1 %s", all_possible_ips[i]);
            int ping_status = system(command);
            if (ping_status == 0)
            {
                connected_optimizers_IPs[j] = realloc(connected_optimizers_IPs[j], strlen(all_possible_ips[i]) + 1);
                strcpy(connected_optimizers_IPs[j], all_possible_ips[i]);
                j++;
                cur_cnt++;
            }
        }

        for (int i = 0; i < MAX_CONNECTION; i++)
        {

            if (connected_optimizers[i] != NULL)
            {
                connected_optimizers[i] = (char *)realloc(connected_optimizers[i], 0);
            }
            if (database_data[i] != NULL)
            {
                database_data[i] = (char *)realloc(database_data[i], 0);
            }
        }

        for (int i = 0; i < MAX_CONNECTION; i++)
        {
            if (connected_optimizers_IPs[i] != NULL)
            {
                connected_optimizers[i] = realloc(connected_optimizers[i], strlen(connected_optimizers_IPs[i]) + 1);
                strcpy(connected_optimizers[i], connected_optimizers_IPs[i]);
            }
        }
        if (prev_cnt && (!cur_cnt) && try_cnt < 4)
        {
            system("ifconfig wlan0 down");
            sleep(1);
            system("ifconfig wlan0 up");
            sleep(1);
            system("/etc/initscripts/ifup-wlan wlan0");
            sleep(1);
            system("app/quectel-CM &");
            try_cnt++;
            printf("try_cnt = %d\n", try_cnt);
        }
        else
        {
            for (int i = 0; i < MAX_CONNECTION; i++)
            {
                if (connected_optimizers_IPs[i] != NULL)
                {
                    connected_optimizers[i] = realloc(connected_optimizers[i], strlen(connected_optimizers_IPs[i]) + 1);
                    strcpy(connected_optimizers[i], connected_optimizers_IPs[i]);
                }
            }
            try_cnt = 0;
            prev_cnt = 0;
        }
        if (prev_cnt != 0 && try_cnt == 4 && cur_cnt == 0)
        {
            system("reboot");
        }
        cur_cnt = 0;
        sleep(30);
    }
}

/**************************************************THREAD-3 RELATED CODE END*********************************************************/

/**************************************************THREAD-4 RELATED CODE START*********************************************************/

size_t write_callback_for_toggle(void *contents, size_t size, size_t nmemb, void *userp)
{
    printf("%s\n", (char *)contents);
    return size * nmemb;
}

void send_post_request_for_toggle(const char *url, const char *data)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl == NULL)
    {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_for_toggle);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 2000);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Failed to perform P-POST request:%s %s\n", url, curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
}

void toggle_function(char *ip_address, char *data)
{
    char url[50] = {0};
    sprintf(url, "%s%s%s%s", "http://", ip_address, ":8080", "/setBypass");
    send_post_request_for_toggle(url, data);
}

size_t write_callback_for_toggleData(void *contents, size_t size, size_t nmemb, void *userp)
{
    char json_to_toggle[5000];
    strcpy(json_to_toggle, (char *)contents);
    char Optimizer[MAX_CONNECTION][20] = {0};
    char Optimizer_flag[MAX_CONNECTION][10] = {0};
    // char arr[] = "[{\"OptimizerID\":\"tes743875874ting3\",\"Flag\":true},{\"OptimizerID\":\"testing2\",\"Flag\":false}]";
    // printf("\n%s\n", json_to_toggle);

    char *position = json_to_toggle;
    int idx = 0;
    while ((position = findSubstring(position, "OptimizerID")) != NULL)
    {
        char *start = strchr(position, ':');

        int len = strlen(start);
        for (int i = 2; i < len; i++)
        {
            if (start[i] == '\"')
            {
                break;
            }
            else
            {
                Optimizer[idx][i - 2] = start[i];
            }
        }

        printf("%s\n", Optimizer[idx]);

        idx++;
        position += 5;
    }
    position = json_to_toggle;
    idx = 0;
    while ((position = findSubstring(position, "Flag")) != NULL)
    {
        char *start = strchr(position, ':');

        int len = strlen(start);
        for (int i = 1; i < len; i++)
        {
            if (start[i] == '}')
            {
                break;
            }
            else
            {
                Optimizer_flag[idx][i - 1] = start[i];
            }
        }

        printf("%s\n", Optimizer_flag[idx]);
        idx++;
        position += 3;
    }

    for (int i = 0; i < MAX_CONNECTION; i++)
    {
        if (Optimizer[i][0] == '\0' || Optimizer_flag[i][0] == '\0')
        {
            continue;
        }
        for (int j = 0; j < MAX_CONNECTION; j++)
        {
            pthread_mutex_lock(&mutex);
            if (IP_OptimizerIDs_map[j][0] == '\0')
            {
                printf("NO Map available");
            }
            else if (strcmp(Optimizer[i], IP_OptimizerIDs_map[j]) == 0)
            {
                char arr[200];
                sprintf(arr, "{\"OptimizerID\":\"%s\",\"Flag\":\"%s\"", Optimizer[i], Optimizer_flag[i]);
                printf("%s\n", arr);
                toggle_function(connected_optimizers[j], arr);
                memset(IP_OptimizerIDs_map[j], 0, 1);
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    return size * nmemb;
}

void *check_for_toogle_request()
{

    char url[] = "http://44.202.86.124:5000/toggleOptimizerIDGateway";
    char data[40];
    sprintf(data, "{\"GatewayID\":\"%s\"}", GatewayID);
    while (1)
    {
        CURL *curl;
        CURLcode res;

        curl = curl_easy_init();
        if (curl == NULL)
        {
            fprintf(stderr, "Failed to initialize libcurl\n");
            return (void *)(-1);
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_for_toggleData);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "Failed to perform P-POST request:%s %s\n", url, curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        sleep(30);
    }
}

/**************************************************THREAD-4 RELATED CODE END*********************************************************/

int main()
{

    for (int i = 0; i < MAX_CONNECTION; i++)
    {
        connected_optimizers[i] = (char *)malloc(current_date_len);
        connected_optimizers[i] = (char *)realloc(connected_optimizers[i], 0);
        connected_optimizers_IPs[i] = (char *)malloc(current_date_len);
        connected_optimizers_IPs[i] = (char *)realloc(connected_optimizers_IPs[i], 0);
        database_data[i] = (char *)malloc(current_date_len);
        database_data[i] = (char *)realloc(database_data[i], 0);
        latest_data[i] = (char *)malloc(current_date_len);
        latest_data[i] = (char *)realloc(latest_data[i], 0);
    }

    optimizerID_for_sending_into_getProperperties = (char *)malloc(current_date_len);
    properties_for_saving_in_latest_data = (char *)malloc(current_date_len);
    current_date = (char *)malloc(current_date_len);
    current_time = (char *)malloc(current_date_len);

    optimizerID_for_sending_into_getProperperties = (char *)realloc(optimizerID_for_sending_into_getProperperties, 0);
    properties_for_saving_in_latest_data = (char *)realloc(properties_for_saving_in_latest_data, 0);
    current_date = (char *)realloc(current_date, 0);
    current_time = (char *)realloc(current_time, 0);

    pthread_mutex_init(&mutex, NULL);

    pthread_t optimizer_query_thread, send_data_to_server_thread, update_connected_optimizer_thread, bypass_optimizers_thread;
    int result;

    result = pthread_create(&update_connected_optimizer_thread, NULL, update_connected_optimizer_function, NULL);
    if (result != 0)
    {
        printf("Failed Create UpdateConnectedOptimizer Thread\n");
        return 1;
    }

    result = pthread_create(&optimizer_query_thread, NULL, optimizer_query_function, NULL);
    if (result != 0)
    {
        printf("Failed to Create OptimizerQuery Thread\n");
        return 1;
    }

    result = pthread_create(&send_data_to_server_thread, NULL, send_data_to_server_function, NULL);
    if (result != 0)
    {
        printf("Failed Create sendDataToServer Thread\n");
        return 1;
    }

    result = pthread_create(&bypass_optimizers_thread, NULL, check_for_toogle_request, NULL);
    if (result != 0)
    {
        printf("Failed Create bypassOptimizers Thread\n");
        return 1;
    }

    // Joining and Wait for the thread to finish
    result = pthread_join(update_connected_optimizer_thread, NULL);
    if (result != 0)
    {
        printf("Failed to join UpdateConnectedOptimizer thread\n");
        return 1;
    }

    result = pthread_join(optimizer_query_thread, NULL);
    if (result != 0)
    {
        printf("Failed to join OptimizerQuery thread\n");
        return 1;
    }

    result = pthread_join(send_data_to_server_thread, NULL);
    if (result != 0)
    {
        printf("Failed to join sendDataToServer thread\n");
        return 1;
    }

    result = pthread_join(bypass_optimizers_thread, NULL);
    if (result != 0)
    {
        printf("Failed to join bypassOptimizers thread\n");
        return 1;
    }
    return 0;
}
// gcc -o sound ./sound.c -pthread -lcurl