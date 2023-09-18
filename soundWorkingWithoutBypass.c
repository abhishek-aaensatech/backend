#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
// #include <jansson.h>
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

#define MAX_CONNECTION 10
#define FIELD_SIZE 50

char *all_possible_ips[MAX_CONNECTION + 1] = {"192.168.5.10", "192.168.5.11", "192.168.5.12", "192.168.5.13", "192.168.5.14", "192.168.5.15", "192.168.5.16", "192.168.5.17", "192.168.5.18", "192.168.5.19", "192.168.5.20"};
char *connected_optimizers_IPs[MAX_CONNECTION];
char *connected_optimizers[MAX_CONNECTION]; // = {"192.168.5.10", "192.168.5.11", "192.168.5.12", "192.168.5.13", "192.168.5.14", "192.168.5.15", "192.168.5.16", "192.168.5.17", "192.168.5.18", "192.168.5.19", "192.168.5.20", "192.168.5.21", "192.168.5.22", "192.168.5.23", "192.168.5.24", "192.168.5.25", "192.168.5.26", "192.168.5.27", "192.168.5.28", "192.168.5.29", "192.168.5.30", "192.168.5.31", "192.168.5.32", "192.168.5.33", "192.168.5.34", "192.168.5.35", "192.168.5.36", "192.168.5.37", "192.168.5.38", "192.168.5.39", "192.168.5.40"};
char *latest_data[MAX_CONNECTION];
char *database_data[MAX_CONNECTION];
// json_t* to_decref_letter[FIELD_SIZE];



char *optimizerID_for_sending_into_getProperperties;
char *properties_for_saving_in_latest_data;
char *current_date;
char *current_time;

char GatewayID[] = "NGCS2023011002";

int cnt = 0;
int is_by_pass_mode = 0;
int current_date_len = 12;
int current_time_len = 10;
int gsmstatus=0;

pthread_mutex_t mutex;
pthread_mutex_t mutex2;

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
        // printf("Error reading: %s\n", strerror(errno));
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
char *concatenate_strings(const char *str1, const char *str2)
{
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char *result = malloc(len1 + len2 + 1);
    if (result == NULL)
    {
        return NULL;
    }
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}

void gsmStat(void)
{
  FILE *fp = fopen("/sys/class/net/wlan0/carrier", "r");
  fscanf(fp, "%d", &gsmstatus);
  pclose(fp);
  printf("GSM status = %d\n",gsmstatus);

  if(gsmstatus==1)
  {
    system("gpiotest 17 0x203 1");
    printf("GSM available\n");
  }
  else
  {
    system("gpiotest 17 0x203 0");
    printf("GSM not available\n");
  }
  return 0;
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
    // pthread_mutex_lock(&mutex_for_curl);
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
    // pthread_mutex_unlock(&mutex_for_curl);
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
    // pthread_mutex_lock(&mutex_for_curl);
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
    // pthread_mutex_unlock(&mutex_for_curl);
}

void query_function(char *ip_address)
{
    int url_len = 25;
    char *url = malloc(url_len * sizeof(char));
    snprintf(url, url_len, "%s%s%s", "http://", ip_address, ":8080");

    // json_t *json_obj = json_object();
    // json_t *value_object = json_string("Optimizer");
    // json_object_set_new(json_obj, "Hello", value_object);
    char post_data[] = "{\"Hello\":\"Optimizer\"}";
    // json_decref(value_object);

    char *completeURL = concatenate_strings(url, "/getOptimizerID");
    send_post_request_for_getOptimizerID(completeURL, post_data);
    printf("free point1\n");
    free(completeURL);
    if (optimizerID_for_sending_into_getProperperties != NULL)
    {
        completeURL = concatenate_strings(url, "/getProperties");
        send_post_request_for_getProperty(concatenate_strings(url, "/getProperties"), optimizerID_for_sending_into_getProperperties);
        printf("free point2\n");
        free(completeURL);
    }
    if (is_by_pass_mode)
    {
        completeURL = concatenate_strings(url, "/getStatus");
        send_post_request_for_getProperty(concatenate_strings(url, "/getStatus"), post_data);
        printf("free point3\n");
        free(completeURL);
    }
    printf("free point4\n");
//    free(post_data);
    printf("free point5\n");
    free(url);
    // json_decref(json_obj);
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
                query_function(connected_optimizers[i]);
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

size_t write_callback_for_data_to_server_nodejs(void *contents, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}

// void send_data_to_server_nodejs(char *data)
// {
//     pthread_mutex_lock(&mutex_for_curl);
//     CURL *curl;
//     CURLcode res;
//     char *url = "http://192.168.5.17:4000/data";

//     printf("curl init\n");
//     curl = curl_easy_init();
//     if (curl == NULL)
//     {
//         fprintf(stderr, "Failed to initialize libcurl\n");
//         return;
//     }
//     printf("URL OPT\n");
//     curl_easy_setopt(curl, CURLOPT_URL, url);
//     printf("POST OPT\n");
//     curl_easy_setopt(curl, CURLOPT_POST, 1L);
//     printf("POSTFIELDS OPT\n");
//     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
//     printf("Callback OPT\n");
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_for_data_to_server_nodejs);
//     printf("Timeout OPT\n");
//     curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000);

//     struct curl_slist *headers = NULL;
//     printf("Accept Header\n");
//     headers = curl_slist_append(headers, "Accept: application/json");
//     printf("contentype Header\n");
//     headers = curl_slist_append(headers, "Content-Type: application/json");
//     printf("SetOPt\n");
//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//     printf("EAsyperform\n");
//     res = curl_easy_perform(curl);
//     if (res != CURLE_OK)
//     {
//         fprintf(stderr, "Failed to perform POST request: %s\n", curl_easy_strerror(res));
//     }
//     printf("Cleanup\n");
//     curl_easy_cleanup(curl);
//     pthread_mutex_unlock(&mutex_for_curl);
// }

void send_data_to_server_nodejs_using_system(char *data)
{
    // pthread_mutex_lock(&mutex_for_curl);
    gsmStat();
    printf("node server start\n");
    char curlbuffer[7000];
    sprintf(curlbuffer, "curl -H 'Content-Type: application/json' -d '%s' -X POST  http://44.202.86.124:5000/sendNewDetailsFromGateway", data);
    //sprintf(curlbuffer, "curl -H 'Content-Type: application/json' -d '%s' -X POST  http://192.168.5.17:4000/data", data);
    system(curlbuffer);
    printf("node server end\n");
    // pthread_mutex_unlock(&mutex_for_curl);
}

// void save_data_to_local_file(char *data)
// {
//     char *date = (char *)malloc(current_date_len);
//     sprintf(date, "%s%s", current_date, ".csv");
//     FILE *file_fd = fopen(date, "a");
//     if (file_fd == NULL)
//     {
//         printf("File Opening Fail");
//         free(date);
//         return;
//     }
//     size_t num_elements = strlen(data);
//     fwrite(data, sizeof(data[0]), num_elements, file_fd);
//     free(date);
//     fclose(file_fd);
// }

// int update_all_object_of_idx(json_t *obj, char *data, json_t *array,int i)
// {
//     int j = 0;
//     char stringValue[20];
//     json_error_t error;
//     int errorValue = 0;
//     printf("if start %s\n", data);
//     obj = json_loads(data, 0, &error);
//     if (obj == NULL)
//     {
//         fprintf(stderr, "Error decoding JSON: %s\n", error.text);
//         errorValue = -1;
//         return -1;
//     }
//     printf("all_object[%d] loaded\n", i);

//     json_t *date_object = json_string(current_date);
//     errorValue = json_object_set_new(obj, "Date", date_object);
//     to_decref_letter[j++] = date_object;
//     if (errorValue == -1)
//     {
//         printf("Error Date\n");
//         return -1;
//     }
//     printf("all_object[%d] Date - %d\n", i, errorValue);

//     json_t *time_object = json_string(current_time);
//     errorValue = json_object_set_new(obj, "Time", time_object);
//     to_decref_letter[j++] = time_object;
//     if (errorValue == -1)
//     {
//         printf("Error Time\n");
//         return -1;
//     }
//     printf("all_object[%d] Time - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", volt_value);
//     json_t *vol1_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph1Voltage(V)", vol1_object);
//     to_decref_letter[j++] = vol1_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph1Voltage\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph1Voltage - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", curr_value);
//     json_t *cur1_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph1Current(A)", cur1_object);
//     to_decref_letter[j++] = cur1_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph1Current\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph1Current - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", powf_value);
//     json_t *pow1_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph1ActivePower(kW)", pow1_object);
//     to_decref_letter[j++] = pow1_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph1ActivePower\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph1ActivePower - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", act_value);
//     json_t *act1_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph1PowerFactor", act1_object);
//     to_decref_letter[j++] = act1_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph1PowerFactor\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph1PowerFactor - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", app_value);
//     json_t*app1_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph1ApparentPower(kVA)", app1_object);
//     to_decref_letter[j++] = app1_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph1ApparentPower\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph1ApparentPower - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", volt_value2);
//     json_t *vol2_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph2Current(A)", vol2_object);
//     to_decref_letter[j++] = vol2_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph2Current\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph2Current - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", curr_value2);
//     json_t *cur2_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph2ActivePower(kW)", cur2_object);
//     to_decref_letter[j++] = cur2_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph2ActivePower\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph2ActivePower - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", powf_value2);
//     json_t *pow_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph2PowerFactor", pow_object);
//     to_decref_letter[j++] = pow_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph2PowerFactor\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph2PowerFactor - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", act_value2);
//     json_t *act2_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph2ApparentPower(kVA)", act2_object);
//     to_decref_letter[j++] = act2_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph2ApparentPower\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph2ApparentPower - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", app_value2);
//     json_t *app2_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph2Voltage(V)", app2_object);
//     to_decref_letter[j++] = app2_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph2Voltage\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph2Voltage - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", volt_value3);
//     json_t *vol3_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph3Voltage(V)", vol3_object);
//     to_decref_letter[j++] = vol3_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph3Voltage\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph3Voltage - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", curr_value3);
//     json_t *cur3_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph3Current(A)", cur3_object);
//     to_decref_letter[j++] = cur3_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph3Current\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph3Current - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", powf_value3);
//     json_t *pow3_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph3ActivePower(kW)", pow3_object);
//     to_decref_letter[j++] = pow3_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph3ActivePower\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph3ActivePower - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", act_value3);
//     json_t *act3_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph3PowerFactor", act3_object);
//     to_decref_letter[j++] = act3_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph3PowerFactor\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph3PowerFactor - %d\n", i, errorValue);

//     memset(stringValue, 0, sizeof(stringValue));
//     sprintf(stringValue, "%f", app_value3);
//     json_t *app3_object = json_string(stringValue);
//     errorValue = json_object_set_new(obj, "Ph3ApparentPower(kVA)", app3_object);
//     to_decref_letter[j++] = app3_object;
//     if (errorValue == -1)
//     {
//         printf("Error Ph3ApparentPower\n");
//         return -1;
//     }
//     printf("all_object[%d] Ph3ApparentPower - %d\n", i, errorValue);

//     json_t *gateway_object = json_string(GatewayID);
//     errorValue = json_object_set_new(obj, "GatewayID", gateway_object);
//     to_decref_letter[j++] = gateway_object;
//     if (errorValue == -1)
//     {
//         printf("Error GatewayID\n");
//         return -1;
//     }
//     printf("all_object[%d] GatewayID - %d\n", i, errorValue);

//     errorValue = json_array_append_new(array, obj);
//     if (errorValue == -1)
//     {
//         printf("Error append array\n");
//         return -1;
//     }
//     printf("all_object[%d] append array - %d\n", i, errorValue);

//     database_data[i] = realloc(database_data[i], strlen(data) + 1);
//     strcpy(database_data[i], data);

//     printf("if end\n\n");
//     return errorValue;
// }

// void *send_data_to_server_function1(void *arg)
// {
//     // json_t *all_object[MAX_CONNECTION];
//     // for (int i = 0; i < MAX_CONNECTION; i++)
//     // {
//     //     all_object[i] = NULL;
//     // }
//     // for(int i = 0; i< FIELD_SIZE; i++){
//     //     to_decref_letter[i] = NULL;
//     // }
//     // json_t *array;
//     // char stringValue[20];
//     // int errorValue = 0;
//     while (1)
//     {
//         printf("while Start \n");
//         if (cnt % 3 == 0)
//         {
//             printf("setup start\n");
//             setup_communication();
//             printf("setup end\n");
//             if (uart_check == 1)
//             {
//                 printf("Phase start\n");
//                 phase1Values();
//                 usleep(1);
//                 phase2Values();
//                 usleep(1);
//                 phase3Values();
//                 close(serial_port);
//                 printf("Phase end\n");
//             }
//             cnt = 0;
//         }
//         getDateTime();
//         // array = json_array();
//         for (int i = 0; i < MAX_CONNECTION; i++)
//         {
//             pthread_mutex_lock(&mutex);

//             if (latest_data[i] != NULL)
//             {
//                 printf("Data has been sent from latest_data\n");
//                 // errorValue = update_all_object_of_idx(all_object[i], latest_data[i], array,i);
//             }

//             else if (connected_optimizers[i] != NULL && database_data[i] != NULL)
//             {
//                 printf("Data has been sent from database_data\n");
//                 // errorValue = update_all_object_of_idx(all_object[i], database_data[i], array,i);
//             }

//             pthread_mutex_unlock(&mutex);
//         }
//         printf("Out of for loop before json_dumps\n");
//         char *json = json_dumps(array, 0);
//         printf("Out of for loop after json_dumps \n%s\n",json);
//         if (strcmp(json, "[]") != 0 && errorValue == 0)
//         {
//             printf("INside error\n");
//             save_data_to_local_file(json);
//             printf("save data to local\n");
//             save_data_to_local_file("\n");
//             printf("Putting BackSlash\n");
//             send_data_to_server_nodejs_using_system(json);
//             // send_data_to_server_nodejs(json);
//             printf("Data has been sent\n");
//         }
//         printf("before the call of decref\n");
//         // for (int i = 0; i < MAX_CONNECTION; i++)
//         // {
//         //     printf("for %d \n",i);
//         //     if (all_object[i] != NULL)
//         //     {
//         //         printf("decref happening\n");
//         //         json_decref(all_object[i]);
//         //         all_object[i] = NULL;
//         //     }
//         // }
//         // for(int i = 0; i<FIELD_SIZE; i++){
//         //     printf("decref of field_size\n");
//         //     if(to_decref_letter[i]!=NULL){
//         //         json_decref(to_decref_letter[i]);
//         //         to_decref_letter[i] = NULL;
//         //     }
//         // }
//         // errorValue = 0;
//         printf("before final json free\n");
//         // if(json!=NULL)
//         // {
//         //     free(json);
//         // }
//         // if(array != NULL)
//         // {
//         //     json_decref(array);
//         //     array = NULL;
//         // }
//         cnt++;
//         printf("End\n");
//         sleep(5);
//     }
// }




void *send_data_to_server_function(void *arg)
{
    char json[3000];
    char newJson[5000];
    char data_to_send[5000];
    char meterDetails[3000];
    while (1)
    {
        printf("while Start \n");
        if (cnt % 3 == 0)
        {
            printf("setup start\n");
            setup_communication();
            printf("setup end\n");
            if (uart_check == 1)
            {
                printf("Phase start\n");
                phase1Values();
                usleep(1);
                phase2Values();
                usleep(1);
                phase3Values();
                close(serial_port);
                printf("Phase end\n");
            }
            cnt = 0;
        }
        getDateTime();
        memset(json,0,sizeof(json));
        memset(newJson,0,sizeof(newJson));
        memset(data_to_send,0,sizeof(data_to_send));
        memset(meterDetails,0,sizeof(meterDetails));
        // pthread_mutex_lock(&mutex2);
        for (int i = 0; i < MAX_CONNECTION; i++)
        {
            pthread_mutex_lock(&mutex);
            if (latest_data[i] != NULL)
            {
                printf("Data has been sent from latest_data\n");
                if(json[0]=='\0'){
                    sprintf(json,"%s",latest_data[i]);
                }else{
                    sprintf(json,"%s,%s",json,latest_data[i]);
                }
                database_data[i] = realloc(database_data[i], strlen(latest_data[i]) + 1);
                strcpy(database_data[i], latest_data[i]);
            }
            else if (connected_optimizers[i] != NULL && database_data[i] != NULL)
            {
                printf("Data has been sent from database_data\n");
                if(json[0]=='\0'){
                    sprintf(json,"%s",database_data[i]);
                }else{
                    sprintf(json,"%s,%s",json,database_data[i]);
                }
            }
            pthread_mutex_unlock(&mutex);
        }
        // pthread_mutex_unlock(&mutex2);

        sprintf(newJson,"\"data\":[\n%s\n]",json);

        sprintf(meterDetails,"\"meterDetails\":{\"GatewayID\":\"%s\",\"Date\":\"%s\",\"Time\":\"%s\",\"Ph1Voltage(V)\":\"%f\",\"Ph1Current(A)\":\"%f\",\"Ph1ActivePower(kW)\":\"%f\",\"Ph1PowerFactor\":\"%f\",\"Ph1ApparentPower(kVA)\":\"%f\",\"Ph2Voltage(V)\":\"%f\",\"Ph2Current(A)\":\"%f\",\"Ph2ActivePower(kW)\":\"%f\",\"Ph2PowerFactor\":\"%f\",\"Ph2ApparentPower(kVA)\":\"%f\",\"Ph3Voltage(V)\":\"%f\",\"Ph3Current(A)\":\"%f\",\"Ph3ActivePower(kW)\":\"%f\",\"Ph3PowerFactor\":\"%f\",\"Ph3ApparentPower(kVA)\":\"%f\"}",GatewayID, current_date,current_time,volt_value,curr_value,powf_value,act_value,app_value,volt_value2,curr_value2,powf_value2,act_value2,app_value2,volt_value3,curr_value3,powf_value3,act_value3,app_value3);

        sprintf(data_to_send,"{%s,%s}",newJson,meterDetails);

        send_data_to_server_nodejs_using_system(data_to_send);
        // if (strcmp(json, "[]") != 0 && errorValue == 0)
        // {
        //     printf("INside error\n");
        
        //     save_data_to_local_file(json);
        //     printf("save data to local\n");
        //     save_data_to_local_file("\n");
        //     printf("Putting BackSlash\n");
            // send_data_to_server_nodejs(json);
        //     printf("Data has been sent\n");
        // }
        printf("End\n");
        sleep(5);
    }
}

/**************************************************THREAD-2 RELATED CODE END*********************************************************/

/**************************************************THREAD-3 RELATED CODE START*********************************************************/

void *update_connected_optimizer_function()
{
    char command[100];
    while (1)
    {
        for (int i = 0; i < MAX_CONNECTION; i++)
        {
            if (connected_optimizers_IPs[i] != NULL)
            {
                connected_optimizers_IPs[i] = (char *)realloc(connected_optimizers_IPs[i], 0);
            }
        }

        for (int i = 0, j = 0; j < (MAX_CONNECTION) && i < (MAX_CONNECTION + 1); i++)
        {
            memset(command, 0, sizeof(command));
            snprintf(command, sizeof(command), "ping -c 1 -W 1 %s", all_possible_ips[i]);
            int ping_status = system(command);
            printf("%s -> %d\n", command, ping_status);
            if (ping_status == 0)
            {
                connected_optimizers_IPs[j] = realloc(connected_optimizers_IPs[j], strlen(all_possible_ips[i]) + 1);
                strcpy(connected_optimizers_IPs[j], all_possible_ips[i]);
                j++;
            }
        }

        // pthread_mutex_lock(&mutex2);
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
                printf("IP -> *%s*\n", connected_optimizers[i]);
            }
        }
        // pthread_mutex_unlock(&mutex2);
        if (connected_optimizers[0] == NULL)
        {
            printf("Try to connect Optimizers\n");
            sleep(1);
        }
        else
        {
            sleep(30);
        }
    }
}

/**************************************************THREAD-3 RELATED CODE END*********************************************************/

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
    // pthread_mutex_init(&mutex2, NULL);

    pthread_t optimizer_query_thread, send_data_to_server_thread, update_connected_optimizer_thread;
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
    // Joining and Wait for the thread to finish
    result = pthread_join(update_connected_optimizer_thread, NULL);
    if (result != 0)
    {
        printf("Failed to UpdateConnectedOptimizer thread\n");
        return 1;
    }

    result = pthread_join(optimizer_query_thread, NULL);
    if (result != 0)
    {
        printf("Failed to OptimizerQuery thread\n");
        return 1;
    }

    result = pthread_join(send_data_to_server_thread, NULL);
    if (result != 0)
    {
        printf("Failed to sendDataToServer thread\n");
        return 1;
    }

//    pthread_mutex_destroy(&mutex);
    return 0;
}

// gcc -o sound ./sound.c -pthread -lcurl -ljansson

