// can be used instead pass login\pass via command line
#define TPSLOGIN "" //<< your tps login
#define TPSPASSWORD "" //<< your tps pass

#include <curl/curl.h>
//#include <curl/types.h> // uncoment this for old curl-dev version
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <locale.h>

typedef struct {
  char *data;
  size_t size;
} ResponseStruct;

typedef struct {
	float TraffUsed;
    float TraffLeft;
	float Balance;
    int CabinetID;
    float AbonentPlata;
    float Kurs;
} CabinetInfoStruct;

CURLcode do_web_request(char *url, char *postvar, ResponseStruct *response);
size_t static WriteMemoryCallback(void *buffer, size_t size, size_t nmemb, void *userp);
int GetCabinetVal(const char *str, char *pattern, char **result);
void usage();
void ShowInfo(CabinetInfoStruct *CabinetInfo);
int DaysInMonth(int month, int year);